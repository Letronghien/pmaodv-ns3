/*
 * Copyright (c) 2025-2026 Trong-Hien Le, Khoa Tran Thi-Minh, Huu-Dung Ngo (IUH)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * MultipathTable for the AOMDV baseline (simplified AOMDV).
 *
 * Stores up to MaxPaths-1 alternate routes per destination (next-hop disjoint).
 * Unlike PMAODV, alternates are used deterministically: only when the primary
 * route is no longer valid, the alternate with the lowest hop count is promoted.
 * NOTE: this is a simplified AOMDV (no advertised-hop-count / link-disjointness
 * rules of Marina & Das, ICNP 2001).
 */

#ifndef AOMDV_MULTIPATH_TABLE_H
#define AOMDV_MULTIPATH_TABLE_H

#include "aomdv-rtable.h"

#include "ns3/ipv4-address.h"

#include <map>
#include <vector>

namespace ns3
{
namespace aomdv
{

/**
 * \brief Stores alternate routes per destination for AOMDV multipath fallback.
 */
class MultipathTable
{
  public:
    MultipathTable(uint32_t maxPaths = 3);

    void SetMaxPaths(uint32_t mp);
    uint32_t GetMaxPaths() const;

    /**
     * Try to add an alternate route for rt.GetDestination().
     * Reject if (dst, nextHop) duplicate (link-disjoint check).
     * If at capacity, replace worst (highest HC) if rt is better.
     */
    bool AddRoute(const RoutingTableEntry& rt);

    /**
     * Get all VALID alternate routes for `dst`.
     */
    // If mainTable is given, alternates whose next hop is no longer a usable
    // neighbour (no VALID entry in the main routing table) are skipped.
    uint32_t GetRoutes(Ipv4Address dst, std::vector<RoutingTableEntry>& routes,
                      const RoutingTable* mainTable = nullptr) const;

    /**
     * Select best (lowest HC) valid alternate route.
     * Used as fallback when the primary route is invalid.
     */
    bool SelectBestAlternate(Ipv4Address dst, RoutingTableEntry& out,
                             const RoutingTable* mainTable = nullptr) const;

    /**
     * Remove all alternate routes to `dst`.
     */
    void DeleteRoutes(Ipv4Address dst);

    /**
     * Remove a specific (dst, nextHop) alternate.
     */
    void DeleteRoute(Ipv4Address dst, Ipv4Address nextHop);

    uint32_t Size() const;
    uint32_t CountFor(Ipv4Address dst) const;
    bool IsFull(Ipv4Address dst) const;
    void Clear();
    void Print(std::ostream& os) const;

  private:
    std::vector<RoutingTableEntry>::iterator FindWorst(
        std::vector<RoutingTableEntry>& vec);

    std::map<Ipv4Address, std::vector<RoutingTableEntry>> m_routes;
    uint32_t m_maxPaths;
};

} // namespace aomdv
} // namespace ns3

#endif /* AOMDV_MULTIPATH_TABLE_H */
