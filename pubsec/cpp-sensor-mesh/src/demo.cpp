#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <atomic>
#include <cstdlib>
#include <filesystem>

#include "env.h"
#include "ditto/mesh_peer.h"
#include "nodes/sensor_node.h"
#include "nodes/c2_node.h"
#include "nodes/effector_node.h"

namespace fs = std::filesystem;

using namespace cuas;

std::atomic<bool> running{true};

void print_banner() {
    std::cout << R"(
  ╔═══════════════════════════════════════════════════════════╗
  ║     Counter-UAS Sensor Mesh - Multi-Node Demo             ║
  ║     All nodes running in single process for demo          ║
  ╚═══════════════════════════════════════════════════════════╝
)" << std::endl;
}

void print_status(SensorNode& sensor, C2Node& c2, EffectorNode& effector) {
    std::cout << "\033[2J\033[H";  // Clear screen
    print_banner();

    std::cout << "═══════════════════════════════════════════════════════════\n";
    std::cout << "  SENSOR NODE: " << sensor.get_sensor()._id << "\n";
    std::cout << "  Detections: " << sensor.get_detections().size() << "\n";
    for (const auto& det : sensor.get_detections()) {
        std::cout << "    - " << det.platform_type << " ["
                  << identityToString(det.identity) << "] "
                  << "Bearing: " << det.bearing << "° Range: " << det.range << "m\n";
    }

    std::cout << "\n═══════════════════════════════════════════════════════════\n";
    std::cout << "  C2 NODE: " << c2.get_id() << "\n";
    std::cout << "  Tracks: " << c2.get_tracks().size()
              << " | Alerts: " << c2.get_alerts().size()
              << " | Assignments: " << c2.get_assignments().size() << "\n";

    for (const auto& track : c2.get_tracks()) {
        std::cout << "    Track " << track.track_number << " ["
                  << identityToString(track.identity) << "] "
                  << track.platform_type << " - " << trackStatusToString(track.status) << "\n";
    }

    for (const auto& alert : c2.get_alerts()) {
        std::cout << "    ALERT [" << alertCategoryToString(alert.category) << "]: "
                  << alert.message << "\n";
    }

    // Show assignments with progress (synced via Ditto from effector)
    auto assignments = c2.get_assignments();
    if (!assignments.empty()) {
        std::cout << "  ─────────────────────────────────────────────────────────\n";
        std::cout << "  ASSIGNMENTS (synced via Ditto):\n";
        for (const auto& a : assignments) {
            std::cout << "    " << a.effector_id << " → Track "
                      << a.track_id.substr(0, 8) << "... ["
                      << assignmentStatusToString(a.status) << "]";
            if (a.status == AssignmentStatus::ENGAGING) {
                std::cout << " Progress: " << a.progress << "%";
            }
            std::cout << "\n";
        }
    }

    std::cout << "\n═══════════════════════════════════════════════════════════\n";
    std::cout << "  EFFECTOR NODE: " << effector.get_id() << "\n";
    std::cout << "  Type: " << effector.get_type_string() << "\n";
    std::cout << "  Status: " << assignmentStatusToString(effector.get_status()) << "\n";
    if (effector.has_active_assignment()) {
        std::cout << "  Local Progress: " << effector.get_progress() << "% ETA: "
                  << effector.get_eta() << "s\n";
    }

    std::cout << "\n═══════════════════════════════════════════════════════════\n";
    std::cout << "  Commands: [s] Sim | [i] Inject | [e] Engage | [c] Clear | [q] Quit\n";
    std::cout << "═══════════════════════════════════════════════════════════\n";
}

int main() {
    // Suppress Ditto SDK verbose logging
    setenv("RUST_LOG", "error", 0);

    print_banner();

    std::cout << "Initializing Ditto mesh...\n";

    try {
        // Create unique persistence directory for demo
        std::string persistence_dir = "/tmp/ditto-cuas-demo";
        fs::create_directories(persistence_dir);

        // Create shared MeshPeer (all nodes share the same peer for demo)
        auto peer = std::make_shared<MeshPeer>(
            DITTO_APP_ID,
            DITTO_PLAYGROUND_TOKEN,
            DITTO_WEBSOCKET_URL,
            DITTO_AUTH_URL,
            true,
            persistence_dir
        );

        peer->start_sync();
        std::cout << "Ditto mesh started!\n\n";

        // Create nodes
        SensorNode sensor(peer, "RADAR-1", "radar", 38.9072, -77.0369);
        C2Node c2(peer, "C2-ALPHA");
        EffectorNode effector(peer, "EFFECTOR-1", EffectorType::INTERCEPTOR, 38.9072, -77.0369);

        // Start all nodes
        sensor.start();
        c2.start();
        effector.start();

        std::cout << "All nodes started!\n";
        std::cout << "Press Enter to continue...\n";
        std::cin.get();

        // Main loop
        while (running) {
            print_status(sensor, c2, effector);

            // Non-blocking input check
            std::cout << "> ";
            std::cout.flush();

            char cmd;
            if (std::cin >> cmd) {
                switch (cmd) {
                    case 's':
                    case 'S':
                        if (sensor.is_simulating()) {
                            sensor.stop_simulation();
                            std::cout << "Simulation stopped.\n";
                        } else {
                            sensor.start_simulation();
                            std::cout << "Simulation started.\n";
                        }
                        break;

                    case 'i':
                    case 'I': {
                        std::cout << "Injecting hostile detection...\n";
                        auto det = sensor.inject_detection(
                            "Unknown sUAS",
                            IdentityAffiliation::HOSTILE,
                            45.0, 2000.0, 85);
                        std::cout << "Created detection: " << det._id << "\n";
                        break;
                    }

                    case 'e':
                    case 'E': {
                        auto tracks = c2.get_tracks();
                        if (!tracks.empty()) {
                            auto& track = tracks[0];
                            std::cout << "Engaging track " << track.track_number << "...\n";
                            c2.assign_interceptor(track._id, "EFFECTOR-1");
                        } else {
                            std::cout << "No tracks to engage.\n";
                        }
                        break;
                    }

                    case 'c':
                    case 'C':
                        std::cout << "Clearing all data...\n";
                        peer->clear_all_data();
                        std::cout << "Data cleared.\n";
                        break;

                    case 'q':
                    case 'Q':
                        running = false;
                        break;

                    default:
                        break;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        // Cleanup
        sensor.stop();
        c2.stop();
        effector.stop();
        peer->stop_sync();

        std::cout << "\nDemo complete.\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
