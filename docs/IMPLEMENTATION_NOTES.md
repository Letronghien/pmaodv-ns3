# Implementation notes

This document states precisely what the code does, so that readers can relate it to the
protocol descriptions in the papers. File and function names refer to `contrib/pmaodv/model/`.

## PMAODV

PMAODV is the ns-3 AODV module plus a per-node `MultipathTable` that stores up to
`MaxPaths − 1` **alternate** routes per destination; the primary route stays in the normal AODV
routing table.

**Storing alternates (route discovery).**

* *Reverse routes* — `RecvRequest()`: a duplicate RREQ (same originator and RREQ id) arriving from
  another neighbour is still dropped as in AODV, but first the reverse route it reveals
  (next hop = that neighbour, hop count = RREQ hop count + 1) is stored as an alternate.
* *Forward routes* — `RecvReply()`: an RREP received via a next hop different from the current
  primary route is stored as an alternate.
* Alternates are next-hop disjoint (one entry per (destination, next hop)). When the table is
  full, a new alternate replaces the longest stored one only if it is shorter.
* Alternates live for `ActiveRouteTimeout` (default 3 s) and are not refreshed. At use time an
  alternate is ignored if it has expired or if its next hop no longer has a VALID entry in the
  main routing table. Alternates are not removed on RERR; they age out.
* `MulticastRrep` (default **false**): when true, the destination also answers each duplicate
  RREQ with an extra RREP along the newly stored reverse path, so the source learns several
  forward routes (the "multicast RREP" of the original PMAODV design, paper §3.1/Fig. 2).
  Intermediate nodes never re-broadcast duplicate RREQs in either mode.
  **The published ns-3 results use `MulticastRrep = false`.**

**Probabilistic selection (data transfer).**
`RouteOutput()` → `MultipathTable::SelectProbabilisticWithPrimary()`: the candidate set is the
primary route plus all usable alternates (excluding any alternate with the primary's next hop);
route *i* is chosen with p_i = (1/HC_i) / Σ_k (1/HC_k) (Eq. 1, Algorithm 1) by roulette-wheel
sampling from a per-node `UniformRandomVariable`.

* Selection is applied in **`RouteOutput()`, i.e. at the node where a packet originates**
  (the data sources, and nodes originating unicast control packets). Packets **forwarded** by
  intermediate nodes follow the primary route (`Forwarding()` is unchanged from AODV).
  The original paper (§3.2) describes selection at both source and intermediate nodes.
* The trace source `RouteSelection(dst, candidates, chosenHops, minHops)` reports every
  selection; `fanet-sim` aggregates it into `selEvents`, `selMultiCandidate`, `selNonMinHop`.
  The share of selections that use a longer-than-shortest route depends on the denominator:
  `selNonMinHop / selMultiCandidate` (only decisions with a real choice) versus
  `selNonMinHop / selEvents` (all decisions, most of which have a single candidate).

## AOMDV (baseline)

Same storage of alternates as PMAODV (same `MaxPaths` semantics, lifetime and validation), but
**deterministic use**: the primary route is always used while valid; when it is no longer valid,
`RouteOutput()` promotes the alternate with the lowest hop count to primary before starting a new
route discovery. This is a simplified AOMDV: it does not implement the advertised-hop-count
loop-freedom rule or link-disjointness of Marina & Das (ICNP 2001); alternates are only
next-hop disjoint.

## Metrics (`simulation/fanet-sim.cc`)

* PDR, delay and throughput are computed by FlowMonitor over the data flows (UDP port 9).
  Delay is averaged over **received** packets only.
* `routingOverhead` (the metric used in the paper) counts every IPv4 transmission
  (`Ipv4L3Protocol/Tx`, i.e. every hop, including forwarding) minus the data packets sent by
  the sources. It therefore equals **control packets + data forwarding transmissions**. A
  protocol that uses longer routes increases this metric even with identical control traffic.
  The pure control traffic is reported separately as `controlPackets`, and the forwarding part as
  `dataForwards`.
* Energy: sum over UAVs of (initial − remaining) energy of the BasicEnergySource.

## Porting history

The modules were first generated on ns-3.40 by cloning `src/aodv` and applying Python patch
scripts, then ported to ns-3.48. Checks performed:

* AODV in ns-3.40 and ns-3.48 is behaviourally identical (differences are comments, formatting,
  an equivalent rewrite of the RREP update condition and a new `TtlStart ≤ NetDiameter` check).
* The patch scripts inject byte-identical code on both versions.
* The cleaned modules here (comments translated, internal labels removed, unused function
  `SelectProbabilistic()` removed, `MulticastRrep` turned from a build-time patch into an
  attribute, `RouteSelection` trace added) give identical results to the patch-generated code
  (36/36 regression runs).
* ns-3.48 fix in the driver: `EnergySourceContainer`/`DeviceEnergyModelContainer` derive from
  `ns3::Object`; assigning them with `operator=` double-frees at exit (exit code 139). They are
  now filled with `Add()`.
