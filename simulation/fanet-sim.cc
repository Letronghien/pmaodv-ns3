/*
 * Copyright (c) 2025-2026 Trong-Hien Le, Khoa Tran Thi-Minh, Huu-Dung Ngo (IUH)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * fanet-sim: FANET scenario used to compare AODV, DSDV, AOMDV and PMAODV
 * (UD-JST re-evaluation paper). One run = one (protocol, N, seed) configuration;
 * the result is appended as one row to a CSV file.
 *
 *   Area      1000 x 1000 x 300 m, 3-D Gauss-Markov mobility (alpha = 0.85)
 *   PHY/MAC   IEEE 802.11b ad hoc, 11 Mb/s data, Friis loss, Tx power 16 dBm
 *   Traffic   UDP CBR, 512 B every 0.25 s per source (all defaults overridable)
 *   Energy    BasicEnergySource (50 J) + WifiRadioEnergyModel
 *
 * Example:
 *   ./ns3 run "fanet-sim --protocol=PMAODV --maxPaths=3 --numNodes=15 --seed=1"
 */

#include "ns3/aodv-module.h"
#include "ns3/aomdv-module.h"
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/energy-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/pmaodv-module.h"
#include "ns3/wifi-module.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <sys/stat.h>
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("FanetSim");

namespace
{

const uint16_t kDataPort = 9;

/**
 * Counts every IPv4 transmission (Ipv4L3Protocol "Tx" trace, i.e. every hop)
 * and classifies it as data (UDP to kDataPort) or control (everything else:
 * RREQ/RREP/RERR/HELLO, DSDV updates, ...).
 *
 * routingOverhead (legacy metric, as reported in the paper)
 *     = all IP transmissions - data packets sent by the sources
 *     = controlPackets + dataForwards
 */
struct TxCounter
{
    uint64_t ipTx = 0;
    uint64_t dataTx = 0;

    void OnIpTx(Ptr<const Packet> p, Ptr<Ipv4>, uint32_t)
    {
        ++ipTx;
        Ptr<Packet> c = p->Copy();
        Ipv4Header ip;
        c->RemoveHeader(ip);
        if (ip.GetProtocol() == UdpL4Protocol::PROT_NUMBER)
        {
            UdpHeader udp;
            c->PeekHeader(udp);
            if (udp.GetDestinationPort() == kDataPort)
            {
                ++dataTx;
            }
        }
    }
};

/// Route-selection statistics from the PMAODV "RouteSelection" trace source.
struct SelectionCounter
{
    uint64_t events = 0;    ///< all selections
    uint64_t multi = 0;     ///< selections with >= 2 candidate routes
    uint64_t nonMinHop = 0; ///< selections of a route longer than the shortest candidate

    void OnSelect(Ipv4Address, uint32_t candidates, uint32_t chosenHops, uint32_t minHops)
    {
        ++events;
        multi += (candidates > 1);
        nonMinHop += (chosenHops > minHops);
    }
};

bool
IsMultipath(const std::string& protocol)
{
    return protocol == "PMAODV" || protocol == "AOMDV";
}

} // namespace

int
main(int argc, char* argv[])
{
    // ---------------------------------------------------------------- parameters
    std::string protocol = "AODV";
    uint32_t maxPaths = 3;
    bool multicastRrep = false;
    std::string mobility = "GAUSS";
    uint32_t numNodes = 10;
    double simTime = 200.0;
    uint32_t seed = 1;
    double initialEnergyJ = 50.0;
    double txPowerDbm = 16.0;
    double areaX = 1000.0;
    double areaY = 1000.0;
    double areaZ = 300.0;
    double alpha = 0.85;
    double meanVelMin = 15.0;
    double meanVelMax = 25.0;
    uint32_t pktSize = 512;
    double pktInterval = 0.25;
    uint32_t numFlows = 0;
    std::string scenario = "default";
    bool enableEnergy = true;
    std::string csvFile = "results.csv";

    CommandLine cmd(__FILE__);
    cmd.AddValue("protocol", "Routing protocol: AODV | DSDV | AOMDV | PMAODV", protocol);
    cmd.AddValue("maxPaths", "AOMDV/PMAODV: max routes per destination", maxPaths);
    cmd.AddValue("multicastRrep", "PMAODV: destination answers duplicate RREQs", multicastRrep);
    cmd.AddValue("mobility", "Mobility model: GAUSS (3-D) | RWP (2-D)", mobility);
    cmd.AddValue("numNodes", "Number of UAVs", numNodes);
    cmd.AddValue("simTime", "Simulation time (s)", simTime);
    cmd.AddValue("seed", "RNG run number", seed);
    cmd.AddValue("initialEnergy", "Initial energy per UAV (J)", initialEnergyJ);
    cmd.AddValue("txPowerDbm", "Tx power (dBm)", txPowerDbm);
    cmd.AddValue("areaX", "Area X (m)", areaX);
    cmd.AddValue("areaY", "Area Y (m)", areaY);
    cmd.AddValue("areaZ", "Area Z (m, GAUSS only)", areaZ);
    cmd.AddValue("alpha", "Gauss-Markov alpha (0..1)", alpha);
    cmd.AddValue("meanVelMin", "Min UAV speed (m/s)", meanVelMin);
    cmd.AddValue("meanVelMax", "Max UAV speed (m/s)", meanVelMax);
    cmd.AddValue("pktSize", "UDP payload (bytes)", pktSize);
    cmd.AddValue("pktInterval", "Packet interval per source (s)", pktInterval);
    cmd.AddValue("numFlows",
                 "0 = every UAV 1..N-1 sends to UAV 0; k>0 = k pairs (i -> N-1-i)",
                 numFlows);
    cmd.AddValue("scenario", "Scenario label written to the CSV", scenario);
    cmd.AddValue("enableEnergy", "Install the energy model", enableEnergy);
    cmd.AddValue("csvFile", "Output CSV (one row appended per run)", csvFile);
    cmd.Parse(argc, argv);

    RngSeedManager::SetSeed(1);
    RngSeedManager::SetRun(seed);

    std::cout << "=== FANET sim ===  protocol=" << protocol;
    if (IsMultipath(protocol))
    {
        std::cout << "(maxPaths=" << maxPaths << ")";
    }
    if (protocol == "PMAODV" && multicastRrep)
    {
        std::cout << "(multicastRrep)";
    }
    std::cout << "  mob=" << mobility << "  N=" << numNodes << "  T=" << simTime
              << "s  seed=" << seed << "  E0=" << initialEnergyJ << "J" << std::endl;

    // ---------------------------------------------------------------- nodes + Wi-Fi
    NodeContainer nodes;
    nodes.Create(numNodes);

    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211b);
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                 "DataMode",
                                 StringValue("DsssRate11Mbps"),
                                 "ControlMode",
                                 StringValue("DsssRate1Mbps"));

    YansWifiPhyHelper wifiPhy;
    wifiPhy.Set("TxPowerStart", DoubleValue(txPowerDbm));
    wifiPhy.Set("TxPowerEnd", DoubleValue(txPowerDbm));

    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");
    wifiPhy.SetChannel(wifiChannel.Create());

    WifiMacHelper wifiMac;
    wifiMac.SetType("ns3::AdhocWifiMac");
    NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, nodes);

    // ---------------------------------------------------------------- mobility
    MobilityHelper mob;
    std::ostringstream velStr;
    velStr << "ns3::UniformRandomVariable[Min=" << meanVelMin << "|Max=" << meanVelMax << "]";
    std::ostringstream xStr;
    std::ostringstream yStr;
    std::ostringstream zStr;
    xStr << "ns3::UniformRandomVariable[Min=0|Max=" << areaX << "]";
    yStr << "ns3::UniformRandomVariable[Min=0|Max=" << areaY << "]";
    zStr << "ns3::UniformRandomVariable[Min=0|Max=" << areaZ << "]";

    if (mobility == "GAUSS")
    {
        mob.SetPositionAllocator("ns3::RandomBoxPositionAllocator",
                                 "X",
                                 StringValue(xStr.str()),
                                 "Y",
                                 StringValue(yStr.str()),
                                 "Z",
                                 StringValue(zStr.str()));
        mob.SetMobilityModel(
            "ns3::GaussMarkovMobilityModel",
            "Bounds",
            BoxValue(Box(0, areaX, 0, areaY, 0, areaZ)),
            "TimeStep",
            TimeValue(Seconds(0.5)),
            "Alpha",
            DoubleValue(alpha),
            "MeanVelocity",
            StringValue(velStr.str()),
            "MeanDirection",
            StringValue("ns3::UniformRandomVariable[Min=0|Max=6.283185307]"),
            "MeanPitch",
            StringValue("ns3::UniformRandomVariable[Min=-0.05|Max=0.05]"),
            "NormalVelocity",
            StringValue("ns3::NormalRandomVariable[Mean=0.0|Variance=1.0|Bound=2.0]"),
            "NormalDirection",
            StringValue("ns3::NormalRandomVariable[Mean=0.0|Variance=0.2|Bound=0.4]"),
            "NormalPitch",
            StringValue("ns3::NormalRandomVariable[Mean=0.0|Variance=0.02|Bound=0.04]"));
    }
    else if (mobility == "RWP")
    {
        mob.SetPositionAllocator("ns3::RandomRectanglePositionAllocator",
                                 "X",
                                 StringValue(xStr.str()),
                                 "Y",
                                 StringValue(yStr.str()));
        mob.SetMobilityModel("ns3::RandomWaypointMobilityModel",
                             "Speed",
                             StringValue(velStr.str()),
                             "Pause",
                             StringValue("ns3::ConstantRandomVariable[Constant=0.0]"),
                             "PositionAllocator",
                             PointerValue(CreateObjectWithAttributes<RandomRectanglePositionAllocator>(
                                 "X",
                                 StringValue(xStr.str()),
                                 "Y",
                                 StringValue(yStr.str()))));
    }
    else
    {
        NS_FATAL_ERROR("Unknown mobility: " << mobility << " (GAUSS|RWP)");
    }
    mob.Install(nodes);

    // ---------------------------------------------------------------- routing + IP
    InternetStackHelper internet;
    if (protocol == "AODV")
    {
        AodvHelper aodv;
        internet.SetRoutingHelper(aodv);
    }
    else if (protocol == "DSDV")
    {
        DsdvHelper dsdv;
        internet.SetRoutingHelper(dsdv);
    }
    else if (protocol == "PMAODV")
    {
        PmaodvHelper pmaodv;
        pmaodv.Set("MaxPaths", UintegerValue(maxPaths));
        pmaodv.Set("MulticastRrep", BooleanValue(multicastRrep));
        internet.SetRoutingHelper(pmaodv);
    }
    else if (protocol == "AOMDV")
    {
        AomdvHelper aomdv;
        aomdv.Set("MaxPaths", UintegerValue(maxPaths));
        internet.SetRoutingHelper(aomdv);
    }
    else
    {
        NS_FATAL_ERROR("Unknown protocol: " << protocol << " (AODV|DSDV|AOMDV|PMAODV)");
    }
    internet.Install(nodes);

    Ipv4AddressHelper addresses;
    addresses.SetBase("10.0.0.0", "255.0.0.0");
    Ipv4InterfaceContainer interfaces = addresses.Assign(devices);

    // ---------------------------------------------------------------- energy
    // EnergySourceContainer / DeviceEnergyModelContainer derive from ns3::Object:
    // fill them with Add(), never with operator= (that double-frees on exit).
    energy::EnergySourceContainer sources;
    energy::DeviceEnergyModelContainer deviceModels;
    if (enableEnergy)
    {
        BasicEnergySourceHelper esHelper;
        esHelper.Set("BasicEnergySourceInitialEnergyJ", DoubleValue(initialEnergyJ));
        sources.Add(esHelper.Install(nodes));
        WifiRadioEnergyModelHelper wifiEnergyHelper;
        deviceModels.Add(wifiEnergyHelper.Install(devices, sources));
    }

    // ---------------------------------------------------------------- traffic
    struct FlowSpec
    {
        uint32_t src;
        uint32_t dst;
    };

    std::vector<FlowSpec> flows;
    if (numFlows == 0)
    {
        for (uint32_t i = 1; i < numNodes; ++i)
        {
            flows.push_back({i, 0});
        }
    }
    else
    {
        if (numFlows * 2 > numNodes)
        {
            NS_FATAL_ERROR("numFlows*2 > numNodes (" << numFlows << "*2 vs " << numNodes << ")");
        }
        for (uint32_t i = 0; i < numFlows; ++i)
        {
            flows.push_back({i, numNodes - 1 - i});
        }
    }

    std::set<uint32_t> sinkIdxs;
    for (const auto& f : flows)
    {
        sinkIdxs.insert(f.dst);
    }
    for (uint32_t sinkIdx : sinkIdxs)
    {
        PacketSinkHelper sinkHelper("ns3::UdpSocketFactory",
                                    InetSocketAddress(Ipv4Address::GetAny(), kDataPort));
        ApplicationContainer serverApp = sinkHelper.Install(nodes.Get(sinkIdx));
        serverApp.Start(Seconds(1.0));
        serverApp.Stop(Seconds(simTime));
    }

    uint64_t dataRateBps = static_cast<uint64_t>(pktSize * 8 / pktInterval);
    std::ostringstream rateStr;
    rateStr << dataRateBps << "bps";

    double startBase = 5.0;
    for (size_t i = 0; i < flows.size(); ++i)
    {
        OnOffHelper onoff("ns3::UdpSocketFactory",
                          InetSocketAddress(interfaces.GetAddress(flows[i].dst), kDataPort));
        onoff.SetAttribute("DataRate", StringValue(rateStr.str()));
        onoff.SetAttribute("PacketSize", UintegerValue(pktSize));
        onoff.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));
        onoff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));
        ApplicationContainer app = onoff.Install(nodes.Get(flows[i].src));
        app.Start(Seconds(startBase + 0.5 * static_cast<double>(i)));
        app.Stop(Seconds(simTime - 1.0));
    }

    // ---------------------------------------------------------------- measurement
    FlowMonitorHelper flowMonHelper;
    Ptr<FlowMonitor> monitor = flowMonHelper.InstallAll();

    TxCounter txCounter;
    Config::ConnectWithoutContext("/NodeList/*/$ns3::Ipv4L3Protocol/Tx",
                                  MakeCallback(&TxCounter::OnIpTx, &txCounter));
    SelectionCounter selCounter;
    if (protocol == "PMAODV")
    {
        Config::ConnectWithoutContext("/NodeList/*/$ns3::pmaodv::RoutingProtocol/RouteSelection",
                                      MakeCallback(&SelectionCounter::OnSelect, &selCounter));
    }

    // ---------------------------------------------------------------- run
    Simulator::Stop(Seconds(simTime + 1.0));
    Simulator::Run();

    // ---------------------------------------------------------------- metrics
    monitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier>(flowMonHelper.GetClassifier());

    uint64_t txPackets = 0;
    uint64_t rxPackets = 0;
    uint64_t rxBytes = 0;
    double sumDelay = 0.0;
    for (const auto& kv : monitor->GetFlowStats())
    {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(kv.first);
        if (t.destinationPort == kDataPort)
        {
            txPackets += kv.second.txPackets;
            rxPackets += kv.second.rxPackets;
            rxBytes += kv.second.rxBytes;
            sumDelay += kv.second.delaySum.GetSeconds();
        }
    }

    double deliveryRatio = (txPackets > 0) ? (100.0 * rxPackets / txPackets) : 0.0;
    double avgDelayMs = (rxPackets > 0) ? (1000.0 * sumDelay / rxPackets) : 0.0;
    double throughputMbps = (rxBytes * 8.0) / (simTime * 1.0e6);
    uint64_t routingOverhead = (txCounter.ipTx > txPackets) ? (txCounter.ipTx - txPackets) : 0;
    uint64_t controlPackets = txCounter.ipTx - txCounter.dataTx;
    uint64_t dataForwards = (txCounter.dataTx > txPackets) ? (txCounter.dataTx - txPackets) : 0;

    double totalEnergyJ = 0.0;
    uint32_t nodesDead = 0;
    for (uint32_t i = 0; i < sources.GetN(); ++i)
    {
        Ptr<energy::BasicEnergySource> src =
            DynamicCast<energy::BasicEnergySource>(sources.Get(i));
        double rem = src->GetRemainingEnergy();
        totalEnergyJ += (initialEnergyJ - rem);
        nodesDead += (rem <= 0.0);
    }

    Simulator::Destroy();

    // ---------------------------------------------------------------- output
    std::cout << std::fixed << std::setprecision(4) << "  delivery=" << deliveryRatio << "%"
              << "  delay=" << avgDelayMs << "ms"
              << "  thr=" << throughputMbps << "Mbps"
              << "  rOver=" << routingOverhead << " (ctrl=" << controlPackets
              << " fwd=" << dataForwards << ")"
              << "  E=" << totalEnergyJ << "J"
              << "  dead=" << nodesDead << "/" << numNodes << std::endl;

    struct stat st;
    bool needHeader = (stat(csvFile.c_str(), &st) != 0) || (st.st_size == 0);
    std::ofstream csv(csvFile.c_str(), std::ios::app);
    if (needHeader)
    {
        csv << "scenario,protocol,mobility,maxPaths,numNodes,numFlows,"
            << "meanVelMin,meanVelMax,pktInterval,simTime,seed,"
            << "deliveryRatio,avgDelayMs,throughputMbps,"
            << "routingOverhead,totalEnergyJ,nodesDead,"
            << "multicastRrep,dataTxSrc,dataRx,controlPackets,dataForwards,"
            << "selEvents,selMultiCandidate,selNonMinHop\n";
    }
    csv << scenario << "," << protocol << "," << mobility << ","
        << (IsMultipath(protocol) ? maxPaths : 1) << "," << numNodes << "," << numFlows << ","
        << meanVelMin << "," << meanVelMax << "," << pktInterval << "," << simTime << ","
        << seed << "," << std::fixed << std::setprecision(4) << deliveryRatio << ","
        << avgDelayMs << "," << throughputMbps << "," << routingOverhead << ","
        << totalEnergyJ << "," << nodesDead << ","
        << ((protocol == "PMAODV" && multicastRrep) ? 1 : 0) << "," << txPackets << ","
        << rxPackets << "," << controlPackets << "," << dataForwards << ","
        << selCounter.events << "," << selCounter.multi << "," << selCounter.nonMinHop << "\n";
    return 0;
}
