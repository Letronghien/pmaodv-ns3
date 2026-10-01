#!/usr/bin/env bash
# install.sh — build PMAODV/AOMDV on a dedicated ns-3.48 tree.
#
# Creates a NEW ns-3 tree (default ~/pmaodv-ns3-sim/ns-3.48) used only by this
# project. Existing ns-3 installations are never touched: the script refuses to
# use a directory it did not create, and installs nothing system-wide.
#
#   bash scripts/install.sh                 # clone + build + smoke test
#   NS3_DIR=/path/to/dir bash scripts/install.sh
#   JOBS=4 bash scripts/install.sh
set -euo pipefail

REPO_DIR=$(cd "$(dirname "$0")/.." && pwd)
NS3_VERSION="3.48"
NS3_DIR="${NS3_DIR:-$HOME/pmaodv-ns3-sim/ns-$NS3_VERSION}"
JOBS="${JOBS:-$(nproc)}"
MARKER=".pmaodv-ns3-sandbox"
MODULES="core;network;internet;wifi;mobility;applications;internet-apps;flow-monitor;energy;aodv;dsdv;aomdv;pmaodv"

say() { printf '\n=== %s ===\n' "$*"; }

# ---------------------------------------------------------------- 1. ns-3 tree
if [ ! -d "$NS3_DIR" ]; then
    say "Cloning ns-$NS3_VERSION into $NS3_DIR"
    mkdir -p "$(dirname "$NS3_DIR")"
    git clone --depth 1 --branch "ns-$NS3_VERSION" https://gitlab.com/nsnam/ns-3-dev.git "$NS3_DIR" \
        || git clone --depth 1 --branch "ns-$NS3_VERSION" https://github.com/nsnam/ns-3-dev-git.git "$NS3_DIR"
    touch "$NS3_DIR/$MARKER"
elif [ ! -f "$NS3_DIR/$MARKER" ]; then
    echo "ERROR: $NS3_DIR exists but was not created by this script; refusing to modify it."
    echo "       Use another location, e.g. NS3_DIR=\$HOME/pmaodv-ns3-sim/ns-$NS3_VERSION"
    exit 1
else
    say "Reusing $NS3_DIR"
fi

# ---------------------------------------------------------------- 2. link sources
# Symlinks: the repository stays the single source of truth.
say "Linking contrib modules and simulation driver"
for m in aomdv pmaodv; do
    rm -rf "$NS3_DIR/contrib/$m"
    ln -s "$REPO_DIR/contrib/$m" "$NS3_DIR/contrib/$m"
done
ln -sf "$REPO_DIR/simulation/fanet-sim.cc" "$NS3_DIR/scratch/fanet-sim.cc"

# ---------------------------------------------------------------- 3. build
cd "$NS3_DIR"
say "Configuring (optimized, modules: $MODULES)"
./ns3 configure --build-profile=optimized --disable-examples --disable-tests \
    --disable-python --enable-modules="$MODULES"
say "Building ($JOBS jobs)"
./ns3 build -j "$JOBS" scratch/fanet-sim

EXEC=$(ls "$NS3_DIR"/build/scratch/ns3.*-fanet-sim-optimized 2>/dev/null | head -1)
[ -x "$EXEC" ] || { echo "ERROR: build failed"; exit 1; }
ln -sf "$EXEC" "$NS3_DIR/fanet-sim"

# ---------------------------------------------------------------- 4. smoke test
say "Smoke test (N=8, 30 s)"
rm -f /tmp/pmaodv-smoke.csv
for v in AODV:1 DSDV:1 AOMDV:3 PMAODV:2 PMAODV:3 PMAODV:4; do
    "$NS3_DIR/fanet-sim" --protocol="${v%%:*}" --maxPaths="${v##*:}" --numNodes=8 \
        --simTime=30 --csvFile=/tmp/pmaodv-smoke.csv | tail -1
done

cat <<EOF

Build OK.
  ns-3 tree  : $NS3_DIR
  executable : $NS3_DIR/fanet-sim
Next:
  NS3_DIR=$NS3_DIR bash $REPO_DIR/scripts/run-experiments.sh
EOF
