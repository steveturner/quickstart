#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

#include "ditto/mesh_peer.h"
#include "nodes/sensor_node.h"
#include "nodes/c2_node.h"
#include "nodes/effector_node.h"

using namespace cuas;

// Helper to flush output immediately
#define LOG(msg) do { std::cout << msg << std::endl; std::cout.flush(); } while(0)

int main() {
    LOG("Testing data flow between nodes...");
    LOG("");

    try {
        LOG("Creating MeshPeer...");
        auto peer = std::make_shared<MeshPeer>(
            "test-app", "test-token", "ws://localhost", "http://localhost",
            true, ""
        );
        LOG("MeshPeer created");

        LOG("Starting sync...");
        peer->start_sync();
        LOG("[OK] Mesh started");

        LOG("Creating SensorNode...");
        SensorNode sensor(peer, "RADAR-1", "radar", 38.9072, -77.0369);
        LOG("SensorNode created");

        LOG("Creating C2Node...");
        C2Node c2(peer, "C2-ALPHA");
        LOG("C2Node created");

        LOG("Creating EffectorNode...");
        EffectorNode effector(peer, "EFFECTOR-1", EffectorType::INTERCEPTOR, 38.9072, -77.0369);
        LOG("EffectorNode created");

        LOG("Starting sensor node...");
        sensor.start();
        LOG("Sensor node started");

        LOG("Starting C2 node...");
        c2.start();
        LOG("C2 node started");

        LOG("Starting effector node...");
        effector.start();
        LOG("[OK] All nodes started");

        // Give observers time to register
        LOG("Waiting for observers...");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        LOG("Wait complete");

        // Initial state
        LOG("");
        LOG("=== Initial State ===");
        LOG("Checking detections...");
        auto initial_dets = sensor.get_detections();
        LOG("Detections: " + std::to_string(initial_dets.size()));

        LOG("Checking tracks...");
        auto initial_tracks = c2.get_tracks();
        LOG("Tracks: " + std::to_string(initial_tracks.size()));

        LOG("Checking alerts...");
        auto initial_alerts = c2.get_alerts();
        LOG("Alerts: " + std::to_string(initial_alerts.size()));

        // Inject hostile detection
        LOG("");
        LOG("=== Injecting HOSTILE detection ===");
        auto det = sensor.inject_detection(
            "Unknown sUAS",
            IdentityAffiliation::HOSTILE,
            45.0, 2000.0, 85);
        LOG("[OK] Created detection: " + det._id);

        // Give time for callbacks to propagate
        LOG("Waiting for propagation...");
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        LOG("Wait complete");

        // Check results
        LOG("");
        LOG("=== After Injection ===");
        auto detections = sensor.get_detections();
        LOG("Detections: " + std::to_string(detections.size()));
        for (const auto& d : detections) {
            LOG("  - " + d._id + " [" + identityToString(d.identity) + "]");
        }

        auto tracks = c2.get_tracks();
        LOG("Tracks: " + std::to_string(tracks.size()));
        for (const auto& t : tracks) {
            LOG("  - " + t.track_number + " [" + identityToString(t.identity) + "] " + trackStatusToString(t.status));
        }

        auto alerts = c2.get_alerts();
        LOG("Alerts: " + std::to_string(alerts.size()));
        for (const auto& a : alerts) {
            LOG("  - [" + alertCategoryToString(a.category) + "] " + a.message);
        }

        // Test assignment
        if (!tracks.empty()) {
            LOG("");
            LOG("=== Testing Assignment ===");
            auto& track = tracks[0];
            auto assignment = c2.assign_interceptor(track._id, "EFFECTOR-1");
            LOG("[OK] Created assignment: " + assignment._id);

            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            auto assignments = c2.get_assignments();
            LOG("Assignments: " + std::to_string(assignments.size()));
            for (const auto& a : assignments) {
                LOG("  - " + a._id + " -> " + a.effector_id + " [" + assignmentStatusToString(a.status) + "]");
            }
        }

        // Cleanup
        LOG("");
        LOG("Cleaning up...");
        sensor.stop();
        c2.stop();
        effector.stop();
        peer->stop_sync();
        LOG("Cleanup complete");

        LOG("");
        LOG("=== VALIDATION ===");
        bool passed = true;
        if (detections.empty()) {
            LOG("[FAIL] No detections found");
            passed = false;
        } else {
            LOG("[PASS] Detection created");
        }
        if (tracks.empty()) {
            LOG("[FAIL] No tracks created from detection");
            passed = false;
        } else {
            LOG("[PASS] Track created from detection");
        }
        if (alerts.empty()) {
            LOG("[FAIL] No alerts created for hostile track");
            passed = false;
        } else {
            LOG("[PASS] Alert created for hostile track");
        }

        LOG("");
        if (passed) {
            LOG("All tests PASSED!");
        } else {
            LOG("Some tests FAILED!");
        }
        return passed ? 0 : 1;

    } catch (const std::exception& e) {
        LOG("Error: " + std::string(e.what()));
        return 1;
    }
}
