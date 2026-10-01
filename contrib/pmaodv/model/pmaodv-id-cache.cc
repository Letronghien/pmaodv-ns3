/*
 * Copyright (c) 2009 IITP RAS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Based on
 *      NS-2 AODV model developed by the CMU/MONARCH group and optimized and
 *      tuned by Samir Das and Mahesh Marina, University of Cincinnati;
 *
 *      AODV-UU implementation by Erik Nordström of Uppsala University
 *      https://web.archive.org/web/20100527072022/http://core.it.uu.se/core/index.php/AODV-UU
 *
 * Authors: Elena Buchatskaia <borovkovaes@iitp.ru>
 *          Pavel Boyko <boyko@iitp.ru>
 *
 * Modified for PMAODV (Probabilistic Multipath AODV):
 *      Copyright (c) 2025-2026 Trong-Hien Le, Khoa Tran Thi-Minh, Huu-Dung Ngo (IUH)
 *      Derived from the ns-3 AODV module (src/aodv, ns-3.48); see README.md.
 */
#include "pmaodv-id-cache.h"

#include <algorithm>

namespace ns3
{
namespace pmaodv
{
bool
IdCache::IsDuplicate(Ipv4Address addr, uint32_t id)
{
    Purge();
    for (auto i = m_idCache.begin(); i != m_idCache.end(); ++i)
    {
        if (i->m_context == addr && i->m_id == id)
        {
            return true;
        }
    }
    UniqueId uniqueId = {addr, id, m_lifetime + Simulator::Now()};
    m_idCache.push_back(uniqueId);
    return false;
}

void
IdCache::Purge()
{
    m_idCache.erase(remove_if(m_idCache.begin(), m_idCache.end(), IsExpired()), m_idCache.end());
}

uint32_t
IdCache::GetSize()
{
    Purge();
    return m_idCache.size();
}

} // namespace pmaodv
} // namespace ns3
