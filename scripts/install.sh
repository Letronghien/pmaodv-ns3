#!/usr/bin/env bash
# install.sh — dedicated ns-3.48 tree for PMAODV, same layout as the other
# FANET projects on the VM:
#
#   ~/pmaodv-repocd1/                  this repository (source of truth, GitHub)
#   ~/pmaodv-repocd1/.venv/            Python environment for the analysis
#   ~/pmaodv-fanet/ns-3-pmaodv/        ns-3.48 tree used ONLY by this project
#       src/pmaodv, src/aomdv          <- copied from contrib/ of this repo
#       scratch/pmaodv-sim.cc          <- copied from simulation/fanet-sim.cc
#       build/scratch/ns3.48-pmaodv-sim-optimized
#
# Re-running the script re-syncs the sources from the repository and rebuilds
# (incremental). Nothing outside ~/pmaodv-fanet and the repo's .venv is written;
# a tree not created by this script is never modified.
#
#   bash scripts/install.sh
#   JOBS=4 bash scripts/install.sh          # limit build parallelism (default: all cores)
#   SKIP_VENV=1 bash scripts/install.sh     # do not create/update .venv
set -euo pipefail

REPO_DIR=$(cd "$(dirname "$0")/.." && pwd)
NS3_VERSION="3.48"
NS3_ROOT="${NS3_ROOT:-$HOME/pmaodv-fanet}"
NS3_DIR="${NS3_DIR:-$NS3_ROOT/ns-3-pmaodv}"
JOBS="${JOBS:-$(nproc)}"
SKIP_VENV="${SKIP_VENV:-0}"
MARKER=".pmaodv-project-tree"
SIM_NAME="pmaodv-sim"
MODULES="core;network;internet;wifi;mobility;applications;internet-apps;flow-monitor;energy;aodv;dsdv;aomdv;pmaodv"

say() { printf '\n=== %s ===\n' "$*"; }

# ---------------------------------------------------------------- 1. ns-3 tree
if [ ! -d "$NS3_DIR" ]; then
    say "[1/5] Cloning ns-$NS3_VERSION into $NS3_DIR"
    mkdir -p "$NS3_ROOT"
    git clone --branch "ns-$NS3_VERSION" https://gitlab.com/nsnam/ns-3-dev.git "$NS3_DIR" \
        || git clone --branch "ns-$NS3_VERSION" https://github.com/nsnam/ns-3-dev-git.git "$NS3_DIR"
    touch "$NS3_DIR/$MARKER"
elif [ ! -f "$NS3_DIR/$MARKER" ]; then
    echo "ERROR: $NS3_DIR exists but was not created by this script; refusing to modify it."
    exit 1
else
    say "[1/5] Reusing $NS3_DIR ($(git -C "$NS3_DIR" describe --tags 2>/dev/null || echo '?'))"
fi

# ---------------------------------------------------------------- 2. sync sources
say "[2/5] Syncing modules and simulation from $REPO_DIR"
for m in aomdv pmaodv; do
    rm -rf "${NS3_DIR:?}/src/$m"
    cp -r "$REPO_DIR/contrib/$m" "$NS3_DIR/src/$m"
done
cp "$REPO_DIR/simulation/fanet-sim.cc" "$NS3_DIR/scratch/$SIM_NAME.cc"
# keep the synced copies out of the ns-3 tree's own git status
EXCL="$NS3_DIR/.git/info/exclude"
if [ -d "$NS3_DIR/.git" ] && ! grep -q "pmaodv-project" "$EXCL" 2>/dev/null; then
    printf '# pmaodv-project (synced from pmaodv-repocd1)\n/src/pmaodv/\n/src/aomdv/\n/scratch/%s.cc\n/%s\n' \
        "$SIM_NAME" "$MARKER" >> "$EXCL"
fi

# ---------------------------------------------------------------- 3. build
cd "$NS3_DIR"
if [ ! -f cmake-cache/CMakeCache.txt ] || ! grep -q "NS3_ENABLED_MODULES:.*pmaodv" cmake-cache/CMakeCache.txt; then
    say "[3/5] Configuring (optimized)"
    ./ns3 configure --build-profile=optimized --disable-examples --disable-tests \
        --disable-python --enable-modules="$MODULES"
fi
say "[3/5] Building with $JOBS jobs"
./ns3 build -j "$JOBS" "scratch/$SIM_NAME"
EXEC="$NS3_DIR/build/scratch/ns$NS3_VERSION-$SIM_NAME-optimized"
[ -x "$EXEC" ] || { echo "ERROR: build failed ($EXEC missing)"; exit 1; }

# ---------------------------------------------------------------- 4. python venv
if [ "$SKIP_VENV" != "1" ]; then
    say "[4/5] Python environment $REPO_DIR/.venv"
    [ -d "$REPO_DIR/.venv" ] || python3 -m venv "$REPO_DIR/.venv"
    "$REPO_DIR/.venv/bin/pip" install -q --upgrade pip
    "$REPO_DIR/.venv/bin/pip" install -q -r "$REPO_DIR/requirements.txt"
else
    say "[4/5] Python environment skipped"
fi

# ---------------------------------------------------------------- 5. smoke test
say "[5/5] Smoke test (N=8, 30 s)"
SMOKE="$NS3_ROOT/smoke-test.csv"
rm -f "$SMOKE"
for v in AODV:1 DSDV:1 AOMDV:3 PMAODV:2 PMAODV:3 PMAODV:4; do
    "$EXEC" --protocol="${v%%:*}" --maxPaths="${v##*:}" --numNodes=8 --simTime=30 \
        --csvFile="$SMOKE" | tail -1
done

cat <<EOF

Build OK
  ns-3 tree  : $NS3_DIR
  executable : $EXEC
  smoke CSV  : $SMOKE
Next (inside tmux):
  tmux new -s pmaodv
  bash $REPO_DIR/scripts/run-experiments.sh
EOF
