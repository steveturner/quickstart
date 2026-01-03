#!/bin/bash

# Counter-UAS Sensor Mesh Demo
# Launches 4 terminal windows with different node types

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/../build"
BINARY="${BUILD_DIR}/cuas-mesh"

# Check if binary exists
if [ ! -f "$BINARY" ]; then
    echo "Binary not found. Building..."
    cd "${SCRIPT_DIR}/.."
    cmake -B build
    cmake --build build
fi

# Default coordinates (Washington DC area)
BASE_LAT=38.8977
BASE_LON=-77.0365

# Detect terminal emulator
if command -v gnome-terminal &> /dev/null; then
    TERM_CMD="gnome-terminal --"
elif command -v xterm &> /dev/null; then
    TERM_CMD="xterm -e"
elif command -v konsole &> /dev/null; then
    TERM_CMD="konsole -e"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS - use osascript
    TERM_CMD="osascript_mac"
else
    echo "No supported terminal emulator found."
    echo "Please run manually in separate terminals:"
    echo ""
    echo "Terminal 1 (Radar Sensor):"
    echo "  $BINARY --node sensor --id S1 --type radar --lat $BASE_LAT --lon $BASE_LON --simulate"
    echo ""
    echo "Terminal 2 (Acoustic Sensor):"
    echo "  $BINARY --node sensor --id S2 --type acoustic --lat $(echo "$BASE_LAT + 0.001" | bc) --lon $(echo "$BASE_LON + 0.001" | bc)"
    echo ""
    echo "Terminal 3 (C2 Station):"
    echo "  $BINARY --node c2 --id C2"
    echo ""
    echo "Terminal 4 (Interceptor):"
    echo "  $BINARY --node effector --id INT1 --type interceptor --lat $(echo "$BASE_LAT - 0.001" | bc) --lon $(echo "$BASE_LON - 0.001" | bc)"
    exit 1
fi

launch_terminal() {
    local title="$1"
    local cmd="$2"

    if [[ "$TERM_CMD" == "osascript_mac" ]]; then
        osascript -e "tell application \"Terminal\" to do script \"$cmd\""
    else
        $TERM_CMD $cmd &
    fi
    sleep 0.5
}

echo "Launching Counter-UAS Sensor Mesh Demo..."
echo ""

# Launch Radar Sensor
echo "Starting Radar Sensor (S1)..."
launch_terminal "Radar Sensor" "$BINARY --node sensor --id S1 --type radar --lat $BASE_LAT --lon $BASE_LON --simulate"

# Launch Acoustic Sensor
echo "Starting Acoustic Sensor (S2)..."
launch_terminal "Acoustic Sensor" "$BINARY --node sensor --id S2 --type acoustic --lat $(echo "$BASE_LAT + 0.001" | bc) --lon $(echo "$BASE_LON + 0.001" | bc)"

# Launch C2 Station
echo "Starting C2 Station..."
launch_terminal "C2 Station" "$BINARY --node c2 --id C2"

# Launch Interceptor
echo "Starting Interceptor (INT1)..."
launch_terminal "Interceptor" "$BINARY --node effector --id INT1 --type interceptor --lat $(echo "$BASE_LAT - 0.001" | bc) --lon $(echo "$BASE_LON - 0.001" | bc)"

echo ""
echo "Demo launched! All nodes should now be syncing via Ditto P2P."
echo ""
echo "Controls:"
echo "  Sensor:    [D]etect [I]nject [S]imulate [Q]uit"
echo "  C2:        [A]lert [E]ngage [J]am [V]iew [Q]uit"
echo "  Effector:  [K]ill Confirm [A]bort [Q]uit"
