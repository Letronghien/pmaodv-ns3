/*
 * Copyright (c) 2009 IITP RAS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Authors: Pavel Boyko <boyko@iitp.ru>, written after OlsrHelper by Mathieu Lacage
 * <mathieu.lacage@sophia.inria.fr>
 *
 * Modified for PMAODV (Probabilistic Multipath AODV):
 *      Copyright (c) 2025-2026 Trong-Hien Le, Khoa Tran Thi-Minh, Huu-Dung Ngo (IUH)
 *      Derived from the ns-3 AODV module (src/aodv, ns-3.48); see README.md.
 */
#include "pmaodv-helper.h"

#include "ns3/pmaodv-routing-protocol.h"
#include "ns3/ipv4-list-routing.h"
#include "ns3/names.h"
#include "ns3/node-list.h"
#include "ns3/ptr.h"

namespace ns3
{

PmaodvHelper::PmaodvHelper()
    : Ipv4RoutingHelper()
{
    m_agentFactory.SetTypeId("ns3::pmaodv::RoutingProtocol");
}

PmaodvHelper*
PmaodvHelper::Copy() const
{
    return new PmaodvHelper(*this);
}

Ptr<Ipv4RoutingProtocol>
PmaodvHelper::Create(Ptr<Node> node) const
{
    Ptr<pmaodv::RoutingProtocol> agent = m_agentFactory.Create<pmaodv::RoutingProtocol>();
    node->AggregateObject(agent);
    return agent;
}

void
PmaodvHelper::Set(std::string name, const AttributeValue& value)
{
    m_agentFactory.Set(name, value);
}

int64_t
PmaodvHelper::AssignStreams(NodeContainer c, int64_t stream)
{
    int64_t currentStream = stream;
    Ptr<Node> node;
    for (auto i = c.Begin(); i != c.End(); ++i)
    {
        node = (*i);
        Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
        NS_ASSERT_MSG(ipv4, "Ipv4 not installed on node");
        Ptr<Ipv4RoutingProtocol> proto = ipv4->GetRoutingProtocol();
        NS_ASSERT_MSG(proto, "Ipv4 routing not installed on node");
        Ptr<pmaodv::RoutingProtocol> pmaodv = DynamicCast<pmaodv::RoutingProtocol>(proto);
        if (pmaodv)
        {
            currentStream += pmaodv->AssignStreams(currentStream);
            continue;
        }
        // Pmaodv may also be in a list
        Ptr<Ipv4ListRouting> list = DynamicCast<Ipv4ListRouting>(proto);
        if (list)
        {
            int16_t priority;
            Ptr<Ipv4RoutingProtocol> listProto;
            Ptr<pmaodv::RoutingProtocol> listPmaodv;
            for (uint32_t i = 0; i < list->GetNRoutingProtocols(); i++)
            {
                listProto = list->GetRoutingProtocol(i, priority);
                listPmaodv = DynamicCast<pmaodv::RoutingProtocol>(listProto);
                if (listPmaodv)
                {
                    currentStream += listPmaodv->AssignStreams(currentStream);
                    break;
                }
            }
        }
    }
    return (currentStream - stream);
}

} // namespace ns3
