#include <iostream>
#include <memory>
#include <string>
#include <cstdlib>
#include <filesystem>

#include <cxxopts.hpp>

#include "env.h"
#include "ditto/mesh_peer.h"
#include "tui/app.h"

namespace fs = std::filesystem;

using namespace cuas;

void print_banner() {
    std::cout << R"(
  ╔═══════════════════════════════════════════════════════════╗
  ║     Counter-UAS Sensor Mesh - Ditto Quickstart            ║
  ║     Distributed detection & response demonstration        ║
  ╚═══════════════════════════════════════════════════════════╝
)" << std::endl;
}

int main(int argc, char* argv[]) {
    cxxopts::Options options("cuas_mesh", "Counter-UAS Sensor Mesh Demo");

    options.add_options()
        ("m,mode", "Node mode: sensor, c2, or effector",
            cxxopts::value<std::string>()->default_value("sensor"))
        ("i,id", "Node identifier",
            cxxopts::value<std::string>()->default_value("NODE-1"))
        ("t,type", "Sensor type (radar, acoustic, rf, eo) or effector type (interceptor, ew)",
            cxxopts::value<std::string>()->default_value("radar"))
        ("lat", "Latitude", cxxopts::value<double>()->default_value("38.9072"))
        ("lon", "Longitude", cxxopts::value<double>()->default_value("-77.0369"))
        ("h,help", "Print usage");

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    print_banner();

    // Parse mode
    std::string mode_str = result["mode"].as<std::string>();
    NodeMode mode;
    if (mode_str == "sensor") {
        mode = NodeMode::SENSOR;
    } else if (mode_str == "c2") {
        mode = NodeMode::C2;
    } else if (mode_str == "effector") {
        mode = NodeMode::EFFECTOR;
    } else {
        std::cerr << "Error: Invalid mode '" << mode_str << "'\n";
        std::cerr << "Valid modes: sensor, c2, effector\n";
        return 1;
    }

    std::string node_id = result["id"].as<std::string>();
    std::string node_type = result["type"].as<std::string>();
    double lat = result["lat"].as<double>();
    double lon = result["lon"].as<double>();

    std::cout << "Starting " << mode_str << " node: " << node_id << "\n";
    std::cout << "Location: " << lat << ", " << lon << "\n\n";

    // Check environment
    if (std::string(DITTO_APP_ID).empty()) {
        std::cerr << "Error: DITTO_APP_ID not set in .env file\n";
        return 1;
    }

    std::cout << "Connecting to Ditto...\n";

    // Suppress Ditto SDK verbose logging (set before init)
    setenv("RUST_LOG", "error", 0);  // Only show errors, don't override if set

    try {
        // Create unique persistence directory per node
        std::string persistence_dir = "/tmp/ditto-cuas-" + node_id;
        fs::create_directories(persistence_dir);

        // Initialize Ditto
        auto peer = std::make_shared<MeshPeer>(
            DITTO_APP_ID,
            DITTO_PLAYGROUND_TOKEN,
            DITTO_WEBSOCKET_URL,
            DITTO_AUTH_URL,
            true,  // enable_cloud_sync
            persistence_dir
        );

        peer->start_sync();
        std::cout << "Connected!\n\n";

        // Create and run TUI
        App app(mode, node_id);
        app.init(peer);
        app.run();

        peer->stop_sync();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
