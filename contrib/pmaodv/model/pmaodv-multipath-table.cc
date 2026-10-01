/*
 * Copyright (c) 2025-2026 Trong-Hien Le, Khoa Tran Thi-Minh, Huu-Dung Ngo (IUH)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * PMAODV multipath table, see pmaodv-multipath-table.h.
 */

#include "pmaodv-multipath-table.h"

#include "ns3/log.h"
#include "ns3/simulator.h"

#include <algorithm>
#include <numeric>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("PmaodvMultipathTable");

namespace pmaodv
{

MultipathTable::MultipathTable(uint32_t maxPaths)
    : m_maxPaths(maxPaths)
{
    m_uniform = CreateObject<UniformRandomVariable>();
}

void
MultipathTable::SetMaxPaths(uint32_t mp)
{
    NS_ASSERT_MSG(mp >= 1, "MaxPaths must be >= 1");
    m_maxPaths = mp;
}

uint32_t
MultipathTable::GetMaxPaths() const
{
    return m_maxPaths;
}

std::vector<RoutingTableEntry>::iterator
MultipathTable::FindWorst(std::vector<RoutingTableEntry>& vec)
{
    if (vec.empty())
    {
        return vec.end();
    }
    auto worst = vec.begin();
    for (auto it = vec.begin() + 1; it != vec.end(); ++it)
    {
        if (it->GetHop() > worst->GetHop())
        {
            worst = it;
        }
    }
    return worst;
}

bool
MultipathTable::AddRoute(const RoutingTableEntry& rt)
{
    Ipv4Address dst = rt.GetDestination();
    Ipv4Address nh = rt.GetNextHop();

    // MaxPaths == 1 means "primary only, no alternates".
    if (m_maxPaths <= 1)
    {
        return false;
    }

    auto& vec = m_routes[dst];

    // Reject duplicate (dst, nextHop).
    for (auto& existing : vec)
    {
        if (existing.GetNextHop() == nh)
        {
            NS_LOG_DEBUG("Reject duplicate alternate: " << dst << " via " << nh);
            return false;
        }
    }

    // Capacity: alternates count is MaxPaths - 1 (one slot reserved for primary).
    uint32_t altCapacity = m_maxPaths - 1;

    if (vec.size() < altCapacity)
    {
        vec.push_back(rt);
        NS_LOG_DEBUG("Added alternate " << dst << " via " << nh
                     << " HC=" << (uint32_t)rt.GetHop()
                     << " (" << vec.size() << "/" << altCapacity << ")");
        return true;
    }

    // At capacity: evict worst if rt is better (smaller hop count).
    auto worst = FindWorst(vec);
    if (worst != vec.end() && rt.GetHop() < worst->GetHop())
    {
        NS_LOG_DEBUG("Evict alternate " << dst << " via " << worst->GetNextHop()
                     << " HC=" << (uint32_t)worst->GetHop()
                     << " for new HC=" << (uint32_t)rt.GetHop());
        *worst = rt;
        return true;
    }

    return false;
}

uint32_t
MultipathTable::GetRoutes(Ipv4Address dst, std::vector<RoutingTableEntry>& routes,
                          const RoutingTable* mainTable) const
{
    auto it = m_routes.find(dst);
    if (it == m_routes.end())
    {
        return 0;
    }

    uint32_t added = 0;
    for (const auto& e : it->second)
    {
        // Skip expired alternates (alternate lifetime = ActiveRouteTimeout).
        if (e.GetFlag() != VALID || e.GetLifeTime() <= Time(0))
        {
            continue;
        }
        // At use time, the alternate's next hop must still be a usable
        // neighbour, i.e. have a VALID entry in the main routing table.
        if (mainTable != nullptr)
        {
            RoutingTableEntry nbr;
            if (!const_cast<RoutingTable*>(mainTable)->LookupRoute(e.GetNextHop(), nbr) ||
                nbr.GetFlag() != VALID)
            {
                continue;
            }
        }
        routes.push_back(e);
        ++added;
    }
    return added;
}

bool
MultipathTable::SelectProbabilisticWithPrimary(const RoutingTableEntry& primary,
                                               RoutingTableEntry& out,
                                               const RoutingTable* mainTable,
                                               uint32_t* nCandidates,
                                               uint32_t* minHop)
{
    Ipv4Address dst = primary.GetDestination();

    // Build candidate list: primary + valid alternates
    // Skip alternates that share the primary's next hop (possible when the
    // primary was updated after the alternate had been stored).
    std::vector<RoutingTableEntry> cands;
    cands.push_back(primary);
    std::vector<RoutingTableEntry> alts;
    GetRoutes(dst, alts, mainTable);
    for (const auto& a : alts)
    {
        if (a.GetNextHop() != primary.GetNextHop())
        {
            cands.push_back(a);
        }
    }

    if (nCandidates)
    {
        *nCandidates = static_cast<uint32_t>(cands.size());
    }
    if (minHop)
    {
        uint32_t m = cands.front().GetHop();
        for (const auto& c : cands)
        {
            m = std::min<uint32_t>(m, c.GetHop());
        }
        *minHop = m;
    }

    if (cands.size() == 1)
    {
        // Only primary — no probabilistic step needed.
        out = primary;
        return true;
    }

    // Compute probabilities and sample.
    std::vector<double> weights;
    weights.reserve(cands.size());
    double sum = 0.0;
    for (const auto& r : cands)
    {
        uint32_t hc = std::max<uint32_t>(1, r.GetHop());
        double w = 1.0 / static_cast<double>(hc);
        weights.push_back(w);
        sum += w;
    }
    if (sum <= 0.0)
    {
        out = primary;
        return true;
    }

    double u = m_uniform->GetValue(0.0, sum);
    double acc = 0.0;
    for (size_t i = 0; i < cands.size(); ++i)
    {
        acc += weights[i];
        if (u < acc)
        {
            out = cands[i];
            NS_LOG_DEBUG("PMAODV select " << dst << " via " << out.GetNextHop()
                         << " HC=" << (uint32_t)out.GetHop()
                         << " p=" << (weights[i] / sum)
                         << " (out of " << cands.size() << " paths)");
            return true;
        }
    }
    out = cands.back();
    return true;
}

void
MultipathTable::DeleteRoutes(Ipv4Address dst)
{
    m_routes.erase(dst);
}

void
MultipathTable::DeleteRoute(Ipv4Address dst, Ipv4Address nextHop)
{
    auto it = m_routes.find(dst);
    if (it == m_routes.end())
    {
        return;
    }
    auto& vec = it->second;
    vec.erase(std::remove_if(vec.begin(), vec.end(),
                             [&](const RoutingTableEntry& e) {
                                 return e.GetNextHop() == nextHop;
                             }),
              vec.end());
    if (vec.empty())
    {
        m_routes.erase(it);
    }
}

uint32_t
MultipathTable::Size() const
{
    return std::accumulate(m_routes.begin(), m_routes.end(), uint32_t{0},
                           [](uint32_t a, const auto& kv) {
                               return a + kv.second.size();
                           });
}

uint32_t
MultipathTable::CountFor(Ipv4Address dst) const
{
    auto it = m_routes.find(dst);
    return (it == m_routes.end()) ? 0 : static_cast<uint32_t>(it->second.size());
}

bool
MultipathTable::IsFull(Ipv4Address dst) const
{
    if (m_maxPaths <= 1)
    {
        return true;
    }
    return CountFor(dst) >= (m_maxPaths - 1);
}

void
MultipathTable::Clear()
{
    m_routes.clear();
}

void
MultipathTable::Print(std::ostream& os) const
{
    os << "MultipathTable (" << Size() << " entries, MaxPaths=" << m_maxPaths << "):\n";
    for (const auto& kv : m_routes)
    {
        os << "  dst=" << kv.first << " alternates=" << kv.second.size() << "\n";
        for (const auto& e : kv.second)
        {
            os << "    via " << e.GetNextHop()
               << " HC=" << (uint32_t)e.GetHop()
               << " state=" << static_cast<int>(e.GetFlag()) << "\n";
        }
    }
}

} // namespace pmaodv
} // namespace ns3
