#!/usr/bin/env bash
# run-experiments.sh — all simulations of the UD-JST paper.
#
#   Scenario     speed (m/s)  pkt interval (s)  flows
#   default      15-25        0.25              DEFAULT_FLOWS
#   multi-flow   15-25        0.25              3        (N >= 6)
#   stress-mob   30-50        0.25              DEFAULT_FLOWS
#   stress-load  15-25        0.05              DEFAULT_FLOWS
#   stress-all   30-50        0.10              3        (N >= 6)
#   density      15-25        0.25              DEFAULT_FLOWS, N = 25 30 40
#
# DEFAULT_FLOWS=0 means every UAV 1..N-1 sends to UAV 0 (see README, "Traffic").
#
#   tmux new -s pmaodv
#   bash scripts/run-experiments.sh
#   SCENARIOS=default NODES="5 10 15 20" SEEDS=3 bash scripts/run-experiments.sh   # quick test
#   JOBS=4 NICE=10 bash scripts/run-experiments.sh   # if other projects run at the same time
#
# Defaults use all CPU cores (JOBS = nproc, NICE = 0). Each run is wrapped in
# `timeout --kill-after=30 RUN_TIMEOUT`.
# Re-running with the same OUT directory resumes: finished runs are skipped.
set -euo pipefail

REPO_DIR=$(cd "$(dirname "$0")/.." && pwd)
NS3_DIR="${NS3_DIR:-$HOME/pmaodv-fanet/ns-3-pmaodv}"
EXEC="${EXEC:-$NS3_DIR/build/scratch/ns3.48-pmaodv-sim-optimized}"
[ -x "$EXEC" ] || { echo "ERROR: $EXEC not found - run scripts/install.sh first"; exit 1; }

JOBS="${JOBS:-$(nproc)}"
NICE="${NICE:-0}"
RUN_TIMEOUT="${RUN_TIMEOUT:-3600}"
SEEDS="${SEEDS:-20}"
DENSITY_SEEDS="${DENSITY_SEEDS:-10}"
SIM_TIME="${SIM_TIME:-200}"
NODES="${NODES:-$(seq -s' ' 5 20)}"
DENSITY_NODES="${DENSITY_NODES:-25 30 40}"
DEFAULT_FLOWS="${DEFAULT_FLOWS:-0}"
MULTICAST_RREP="${MULTICAST_RREP:-0}"
SCENARIOS="${SCENARIOS:-default multi-flow stress-mob stress-load stress-all density}"
VARIANTS="${VARIANTS:-AODV:1 DSDV:1 AOMDV:3 PMAODV:2 PMAODV:3 PMAODV:4}"   # PROTOCOL:maxPaths
OUT="${OUT:-$REPO_DIR/data/ns-3.48/runs/run-$(date +%Y%m%d-%H%M%S)}"

mkdir -p "$OUT/runs" "$OUT/logs"
JOBLIST="$OUT/joblist.txt"
: > "$JOBLIST"

scenario_params() {    # -> "velMin velMax interval flows"
    case $1 in
        default|density) echo "15 25 0.25 $DEFAULT_FLOWS" ;;
        multi-flow)      echo "15 25 0.25 3" ;;
        stress-mob)      echo "30 50 0.25 $DEFAULT_FLOWS" ;;
        stress-load)     echo "15 25 0.05 $DEFAULT_FLOWS" ;;
        stress-all)      echo "30 50 0.10 3" ;;
        *) echo "unknown scenario: $1" >&2; exit 1 ;;
    esac
}

for sc in $SCENARIOS; do
    read -r vmin vmax ival flows <<< "$(scenario_params "$sc")"
    if [ "$sc" = density ]; then nlist=$DENSITY_NODES; nseeds=$DENSITY_SEEDS
    else nlist=$NODES; nseeds=$SEEDS; fi
    for n in $nlist; do
        if [ "$flows" -gt 0 ] && [ $((flows * 2)) -gt "$n" ]; then continue; fi
        for v in $VARIANTS; do
            proto=${v%%:*}; mp=${v##*:}
            for s in $(seq 1 "$nseeds"); do
                id="${sc}_${proto}${mp}_N${n}_s${s}"
                [ -s "$OUT/runs/$id.csv" ] && continue
                echo "$id --scenario=$sc --protocol=$proto --maxPaths=$mp \
--multicastRrep=$MULTICAST_RREP --numNodes=$n --numFlows=$flows \
--meanVelMin=$vmin --meanVelMax=$vmax --pktInterval=$ival --pktSize=512 \
--mobility=GAUSS --alpha=0.85 --enableEnergy=1 --simTime=$SIM_TIME --seed=$s \
--csvFile=$OUT/runs/$id.csv" >> "$JOBLIST"
            done
        done
    done
done

# Provenance (kept next to the data).
{
    echo "date:            $(date -Iseconds)"
    echo "repository:      $(git -C "$REPO_DIR" remote get-url origin 2>/dev/null || echo n/a)"
    echo "repo commit:     $(git -C "$REPO_DIR" rev-parse --verify -q HEAD 2>/dev/null || echo n/a)"
    echo "repo dirty:      $(git -C "$REPO_DIR" status --porcelain 2>/dev/null | grep -qv '^??' && echo yes || echo no)"
    echo "ns-3 tag:        $(git -C "$NS3_DIR" describe --tags 2>/dev/null || echo n/a)"
    echo "ns-3 commit:     $(git -C "$NS3_DIR" rev-parse HEAD 2>/dev/null || echo n/a)"
    echo "compiler:        $(c++ --version | head -1)"
    echo "os:              $(. /etc/os-release && echo "$PRETTY_NAME")"
    echo "cpu:             $(nproc) x $(grep -m1 'model name' /proc/cpuinfo | cut -d: -f2 | xargs)"
    echo "SCENARIOS=$SCENARIOS"
    echo "NODES=$NODES  DENSITY_NODES=$DENSITY_NODES"
    echo "SEEDS=$SEEDS  DENSITY_SEEDS=$DENSITY_SEEDS  SIM_TIME=$SIM_TIME"
    echo "DEFAULT_FLOWS=$DEFAULT_FLOWS  MULTICAST_RREP=$MULTICAST_RREP"
    echo "VARIANTS=$VARIANTS"
} > "$OUT/manifest.txt"

TOTAL=$(wc -l < "$JOBLIST")
echo "executable : $EXEC"
echo "output     : $OUT"
echo "runs to do : $TOTAL (parallel jobs: $JOBS, nice $NICE, timeout ${RUN_TIMEOUT}s)"

export EXEC OUT NICE RUN_TIMEOUT
run_job() {
    local id=$1; shift
    local rc=0
    nice -n "$NICE" timeout --kill-after=30 "$RUN_TIMEOUT" "$EXEC" "$@" \
        > "$OUT/logs/$id.log" 2>&1 || rc=$?
    if [ "$rc" -ne 0 ]; then
        echo "$id exit=$rc" >> "$OUT/failed.txt"     # 124 = timeout
        rm -f "$OUT/runs/$id.csv"
    fi
}
export -f run_job
if [ "$TOTAL" -gt 0 ]; then
    # one CSV per run -> no write races; merged below
    xargs -P "$JOBS" -L 1 bash -c 'run_job "$@"' _ < "$JOBLIST"
fi

MERGED="$OUT/all-runs.csv"
first=$(find "$OUT/runs" -name '*.csv' | sort | head -1)
if [ -n "$first" ]; then
    head -1 "$first" > "$MERGED"
    find "$OUT/runs" -name '*.csv' | sort | while read -r f; do tail -n +2 "$f"; done >> "$MERGED"
    echo "merged     : $MERGED ($(($(wc -l < "$MERGED") - 1)) rows)"
fi
[ -f "$OUT/failed.txt" ] && echo "FAILED     : $(wc -l < "$OUT/failed.txt") runs, see $OUT/failed.txt"
PY="$REPO_DIR/.venv/bin/python"; [ -x "$PY" ] || PY=python3
echo "analysis   : $PY $REPO_DIR/scripts/analyze.py $MERGED --outdir $OUT/analysis"
