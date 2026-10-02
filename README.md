# PMAODV on ns-3 — reproducibility package

Code, scripts and data for the paper

> Trong-Hien Le, **"Re-evaluating the reproducibility of PMAODV performance on NS-3 for FANETs:
> a comparison with AODV, AOMDV and DSDV"** (*Đánh giá khả năng tái lập hiệu năng PMAODV trên
> NS-3 trong mạng FANET: so sánh với AODV, AOMDV và DSDV*), The University of Danang —
> Journal of Science and Technology (submitted).

which re-evaluates the protocol originally proposed in

> T.-H. Le, K. Tran Thi-Minh, H.-D. Ngo, "Probabilistic Multipath Ad-hoc Routing Protocol for the
> Internet of Things based Applications", *LNNS* vol. 1782, Springer, 2026, pp. 287–299.
> https://doi.org/10.1007/978-3-032-14935-0_22

| | |
|---|---|
| Simulator | **ns-3.48** (pinned; installed into a dedicated directory) |
| Protocols | AODV, DSDV (ns-3 built-in), **PMAODV** and **AOMDV** (this repository, `contrib/`) |
| Licence | GPL-2.0-only (the modules are derived from the ns-3 AODV module) |

## Repository layout

```
contrib/pmaodv/          PMAODV ns-3 module (Probabilistic Multipath AODV)
contrib/aomdv/           AOMDV baseline module (simplified, see docs/IMPLEMENTATION_NOTES.md)
simulation/fanet-sim.cc  FANET scenario: one run = one CSV row
scripts/install.sh       dedicated ns-3.48 tree + build + .venv + smoke test
scripts/run-experiments.sh   all scenarios of the paper (resumable, parallel)
scripts/analyze.py       Tables 2-5, Welch/Holm statistics, Figures 1-4
data/                    simulation results used in the paper
docs/IMPLEMENTATION_NOTES.md   implementation details and deviations
```

## 1. Install (≈10–25 min)

Tested on Ubuntu 24.04, GCC 13, CMake ≥ 3.20, Python 3.10+.

```bash
sudo apt-get install -y git g++ cmake ninja-build python3-venv   # usually already present

git clone https://github.com/Letronghien/pmaodv-ns3.git ~/pmaodv-repocd1
cd ~/pmaodv-repocd1
bash scripts/install.sh
```

Directory layout created on the machine (one ns-3 tree per project):

```
~/pmaodv-repocd1/                     this repository (source of truth)
~/pmaodv-repocd1/.venv/               Python environment (pandas, scipy, matplotlib)
~/pmaodv-repocd1/data/ns-3.48/runs/   simulation batches
~/pmaodv-fanet/ns-3-pmaodv/           dedicated ns-3.48 tree
    src/pmaodv, src/aomdv             copied from contrib/ of the repository
    scratch/pmaodv-sim.cc             copied from simulation/fanet-sim.cc
    build/scratch/ns3.48-pmaodv-sim-optimized
```

`install.sh` clones a **fresh** ns-3.48 tree used only by this project, copies the two modules
into its `src/` and the driver into `scratch/` (re-running the script re-syncs them after
changes in the repository), builds an optimized binary with all CPU cores (`JOBS=` to limit),
creates `.venv` from `requirements.txt` and runs a smoke test. It refuses to modify an ns-3
tree it did not create and installs nothing system-wide, so other ns-3 projects on the same
machine are not affected.

## 2. Run the experiments

```bash
tmux new -s pmaodv
bash scripts/run-experiments.sh                         # everything, data/ns-3.48/runs/run-<date>/
SCENARIOS=default NODES="5 10 15 20" SEEDS=3 bash scripts/run-experiments.sh   # quick check
```

| Scenario | Speed (m/s) | Packet interval (s) | Flows | N | Seeds |
|---|---|---|---|---|---|
| default | 15–25 | 0.25 | `DEFAULT_FLOWS` | 5–20 | 20 |
| multi-flow | 15–25 | 0.25 | 3 | 6–20 | 20 |
| stress-mob | 30–50 | 0.25 | `DEFAULT_FLOWS` | 5–20 | 20 |
| stress-load | 15–25 | 0.05 | `DEFAULT_FLOWS` | 5–20 | 20 |
| stress-all | 30–50 | 0.10 | 3 | 6–20 | 20 |
| density | 15–25 | 0.25 | `DEFAULT_FLOWS` | 25, 30, 40 | 10 |

Variants: AODV, DSDV, AOMDV (MaxPaths = 3), PMAODV (MaxPaths = 2, 3, 4).
Common settings: 1000 × 1000 × 300 m, 3-D Gauss-Markov (α = 0.85, time step 0.5 s),
IEEE 802.11b ad hoc (11 Mb/s data, 1 Mb/s control), Friis propagation, 16 dBm,
UDP CBR 512 B, 200 s, BasicEnergySource 50 J + WifiRadioEnergyModel.

**Traffic.** `numFlows=0` (default, `DEFAULT_FLOWS=0`): every UAV 1..N−1 sends to UAV 0 (the
ground-station role). `numFlows=k`: k independent pairs (i → N−1−i).

Environment variables: `SEEDS`, `DENSITY_SEEDS`, `NODES`, `DENSITY_NODES`, `SCENARIOS`,
`VARIANTS`, `SIM_TIME`, `DEFAULT_FLOWS`, `MULTICAST_RREP`, `JOBS` (default: all cores),
`NICE` (default 0), `RUN_TIMEOUT` (default 3600 s per run), `OUT`.
Each run writes its own CSV; interrupted batches are resumed by re-running with the same `OUT`.
`manifest.txt` records the repository commit, ns-3 commit, compiler, OS and all parameters.

Run time: ≈ 9 800 runs. A 200 s run takes ≈ 1 s (N = 5) to ≈ 20 s (N = 20) of CPU time;
N = 40 is considerably slower. A few hours on an 8-vCPU VM.

## 3. Analyse

```bash
RUN=data/ns-3.48/runs/run-<date>
.venv/bin/python scripts/analyze.py $RUN/all-runs.csv --outdir $RUN/analysis
```

Produces Table 2 (means), Tables 3/4 (two-sided Welch t-test vs AODV per N, Holm–Bonferroni
within each family — `default`, `stress` = the four load scenarios pooled, `density` — with
pooled Cohen's d), Table 5 (normalized routing load and energy per delivered packet, using exact
packet counts), PMAODV route-selection statistics, Figures 1–4 (PDF + PNG) and `report.md`.

## 4. Output columns (`all-runs.csv`)

| Column | Meaning |
|---|---|
| `scenario, protocol, maxPaths, numNodes, numFlows, meanVelMin, meanVelMax, pktInterval, simTime, seed` | configuration (`seed` = ns-3 run number) |
| `deliveryRatio` | % of data packets sent by the sources that were received (FlowMonitor) |
| `avgDelayMs` | mean end-to-end delay of **received** packets |
| `throughputMbps` | received bits / simulation time |
| `routingOverhead` | **metric reported in the paper**: all IPv4 transmissions (every hop) − data packets sent by the sources = `controlPackets + dataForwards` |
| `controlPackets` | IPv4 transmissions that are not data (RREQ, RREP, RERR, HELLO, DSDV updates) |
| `dataForwards` | data-packet transmissions by intermediate nodes |
| `totalEnergyJ, nodesDead` | energy consumed by all UAVs; UAVs with depleted battery |
| `multicastRrep` | PMAODV `MulticastRrep` attribute (0 in the paper) |
| `dataTxSrc, dataRx` | data packets sent by sources / received by sinks (exact counts) |
| `selEvents` | PMAODV route selections for locally originated packets |
| `selMultiCandidate` | … of which had ≥ 2 candidate routes |
| `selNonMinHop` | … of which chose a route longer than the shortest candidate |

## 5. Reproducibility notes

* Results are deterministic: the same binary, parameters and seed give identical rows.
* ns-3.48 results are **not** bit-identical to the ns-3.40 runs of earlier drafts: the protocol
  code is identical (verified), but the Wi-Fi/PHY models and random-stream assignment changed
  between ns-3 releases. All numbers in the paper should come from one ns-3 version; the data
  in `data/` state which one.
* The cleaned modules in this repository were regression-tested against the original
  patch-generated implementation: 36/36 test runs (AODV, DSDV, AOMDV, PMAODV mp = 2/3/4,
  `MulticastRrep` = 0 and 1, N = 8/15, two seeds) produced identical output.
* See `docs/IMPLEMENTATION_NOTES.md` for how the implementation relates to the protocol
  description in the papers.

## Citation

See `CITATION.cff`. If you use the PMAODV module, please cite both papers above.

## Acknowledgements and licence

`contrib/pmaodv` and `contrib/aomdv` are derived from the ns-3 AODV module
(Copyright © 2009 IITP RAS; authors Elena Buchatskaia and Pavel Boyko), which is based on the
NS-2 AODV model (CMU/MONARCH, S. Das and M. Marina) and AODV-UU (E. Nordström, Uppsala
University). Modifications © 2025–2026 the authors of this repository. Distributed under
GPL-2.0-only, see `LICENSE`.
