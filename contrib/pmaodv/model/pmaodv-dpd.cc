/*
 * Copyright (c) 2009 IITP RAS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 *
 * Authors: Elena Buchatskaia <borovkovaes@iitp.ru>
 *          Pavel Boyko <boyko@iitp.ru>
 *
 * Modified for PMAODV (Probabilistic Multipath AODV):
 *      Copyright (c) 2025-2026 Trong-Hien Le, Khoa Tran Thi-Minh, Huu-Dung Ngo (IUH)
 *      Derived from the ns-3 AODV module (src/aodv, ns-3.48); see README.md.
 */

#include "pmaodv-dpd.h"

namespace ns3
{
namespace pmaodv
{

bool
DuplicatePacketDetection::IsDuplicate(Ptr<const Packet> p, const Ipv4Header& header)
{
    return m_idCache.IsDuplicate(header.GetSource(), p->GetUid());
}

void
DuplicatePacketDetection::SetLifetime(Time lifetime)
{
    m_idCache.SetLifetime(lifetime);
}

Time
DuplicatePacketDetection::GetLifetime() const
{
    return m_idCache.GetLifeTime();
}

} // namespace pmaodv
} // namespace ns3
