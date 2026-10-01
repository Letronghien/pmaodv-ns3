/*
 * Copyright (c) 2025-2026 Trong-Hien Le, Khoa Tran Thi-Minh, Huu-Dung Ngo (IUH)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * MultipathTable for PMAODV (Probabilistic Multipath AODV).
 *
 * Stores up to MaxPaths-1 ALTERNATE routes per destination, next to the primary
 * route kept in the regular AODV routing table. A route is chosen among the
 * primary and the usable alternates with probability
 *
 *     p_i = (1 / HC_i) / sum_k (1 / HC_k)        (Eq. 1 in the paper)
 *
 * so shorter routes are preferred while longer ones keep a non-zero chance.
 */

#ifndef PMAODV_MULTIPATH_TABLE_H
#define PMAODV_MULTIPATH_TABLE_H

#include "pmaodv-rtable.h"

#include "ns3/ipv4-address.h"
#include "ns3/random-variable-stream.h"

#include <map>
#include <vector>

namespace ns3
{
namespace pmaodv
{

/**
 * \brief Stores alternate routes per destination for PMAODV multipath.
 */
class MultipathTable
{
  public:
    MultipathTable(uint32_t maxPaths = 3);

    void SetMaxPaths(uint32_t mp);
    uint32_t GetMaxPaths() const;

    /**
     * Try to add an alternate route for rt.GetDestination().
     * Rules:
     *   - Reject if (dst, nextHop) duplicate already in table.
     *   - If size < MaxPaths: append.
     *   - Else if rt.HopCount < worst route's HopCount: evict worst, insert rt.
     *   - Else: reject.
     * \return true if added; false if rejected.
     */
    bool AddRoute(const RoutingTableEntry& rt);

    /**
     * Fetch all VALID alternate routes for `dst`.
     * Note: routes with state INVALID/IN_SEARCH are skipped (only VALID returned).
     * \return number of valid routes added to `routes`.
     */
    // If mainTable is given, alternates whose next hop no longer has a VALID
    // entry in the main routing table (i.e. is no longer a usable neighbour)
    // are skipped.
    uint32_t GetRoutes(Ipv4Address dst, std::vector<RoutingTableEntry>& routes,
                      const RoutingTable* mainTable = nullptr) const;

    /**
     * Select one route among {primary} U {usable alternates} with
     * p_i = (1/HC_i) / sum_k (1/HC_k). Used by RoutingProtocol::RouteOutput.
     *
     * \param primary    primary route from the main routing table
     * \param out        selected route
     * \param mainTable  main routing table, used to validate alternates' next hops
     * \param nCandidates [out, optional] size of the candidate set
     * \param minHop      [out, optional] smallest hop count in the candidate set
     * \return always true (falls back to the primary route)
     */
    bool SelectProbabilisticWithPrimary(const RoutingTableEntry& primary,
                                        RoutingTableEntry& out,
                                        const RoutingTable* mainTable = nullptr,
                                        uint32_t* nCandidates = nullptr,
                                        uint32_t* minHop = nullptr);

    /**
     * Remove all alternate routes to `dst`. Called when route invalidated
     * or during link failure handling.
     */
    void DeleteRoutes(Ipv4Address dst);

    /**
     * Remove a specific (dst, nextHop) alternate.
     */
    void DeleteRoute(Ipv4Address dst, Ipv4Address nextHop);

    /**
     * Number of (dst, nextHop) pairs stored across all destinations.
     */
    uint32_t Size() const;

    /**
     * Total alternates currently stored for a destination.
     */
    uint32_t CountFor(Ipv4Address dst) const;

    /**
     * Whether table has reached MaxPaths capacity for a destination.
     * Useful to decide whether to keep processing duplicate RREQs.
     */
    bool IsFull(Ipv4Address dst) const;

    /**
     * Clear all entries.
     */
    void Clear();

    /**
     * Debug print.
     */
    void Print(std::ostream& os) const;

  private:
    /**
     * \return iterator to entry with the highest hop count for `dst`,
     *         or end() if no entries.
     */
    std::vector<RoutingTableEntry>::iterator FindWorst(
        std::vector<RoutingTableEntry>& vec);

    /// Map: destination -> list of alternate routes (excluding primary).
    std::map<Ipv4Address, std::vector<RoutingTableEntry>> m_routes;

    uint32_t m_maxPaths;

    /// RNG for probabilistic selection.
    Ptr<UniformRandomVariable> m_uniform;
};

} // namespace pmaodv
} // namespace ns3

#endif /* PMAODV_MULTIPATH_TABLE_H */
