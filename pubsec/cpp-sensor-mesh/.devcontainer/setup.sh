#!/bin/bash
set -e

echo "=== Setting up C++ Counter-UAS Sensor Mesh ==="

# Install dependencies
sudo apt-get update
sudo apt-get install -y curl libssl-dev

# Clean any existing build from host (different paths)
echo ""
echo "=== Cleaning previous build ==="
rm -rf build

# Download the real Ditto SDK (Linux only)
echo ""
echo "=== Downloading Ditto C++ SDK ==="
make download-sdk

# Build the project with real SDK
echo ""
echo "=== Building with Ditto SDK ==="
make build

echo ""
echo "=== Setup Complete ==="
echo ""
echo "The C++ Counter-UAS Sensor Mesh is built with the real Ditto SDK."
echo "This enables actual P2P synchronization over the network."
echo ""
echo "To run the demo:"
echo "  ./build/cuas-demo"
echo ""
echo "To run the full TUI application:"
echo "  ./build/cuas-mesh"
echo ""
echo "To run tests:"
echo "  ./build/test-data-flow"
