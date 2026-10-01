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

#ifndef PMAODV_HELPER_H
#define PMAODV_HELPER_H

#include "ns3/ipv4-routing-helper.h"
#include "ns3/node-container.h"
#include "ns3/node.h"
#include "ns3/object-factory.h"

namespace ns3
{
/**
 * @ingroup pmaodv
 * @brief Helper class that adds PMAODV routing to nodes.
 */
class PmaodvHelper : public Ipv4RoutingHelper
{
  public:
    PmaodvHelper();

    /**
     * @returns pointer to clone of this PmaodvHelper
     *
     * @internal
     * This method is mainly for internal use by the other helpers;
     * clients are expected to free the dynamic memory allocated by this method
     */
    PmaodvHelper* Copy() const override;

    /**
     * @param node the node on which the routing protocol will run
     * @returns a newly-created routing protocol
     *
     * This method will be called by ns3::InternetStackHelper::Install
     *
     * @todo support installing PMAODV on the subset of all available IP interfaces
     */
    Ptr<Ipv4RoutingProtocol> Create(Ptr<Node> node) const override;
    /**
     * @param name the name of the attribute to set
     * @param value the value of the attribute to set.
     *
     * This method controls the attributes of ns3::pmaodv::RoutingProtocol
     */
    void Set(std::string name, const AttributeValue& value);
    /**
     * Assign a fixed random variable stream number to the random variables
     * used by this model.  Return the number of streams (possibly zero) that
     * have been assigned.  The Install() method of the InternetStackHelper
     * should have previously been called by the user.
     *
     * @param stream first stream index to use
     * @param c NodeContainer of the set of nodes for which PMAODV
     *          should be modified to use a fixed stream
     * @return the number of stream indices assigned by this helper
     */
    int64_t AssignStreams(NodeContainer c, int64_t stream);

  private:
    /** the factory to create PMAODV routing object */
    ObjectFactory m_agentFactory;
};

} // namespace ns3

#endif /* PMAODV_HELPER_H */
