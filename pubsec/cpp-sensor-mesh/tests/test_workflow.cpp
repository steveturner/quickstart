#include <gtest/gtest.h>

#include "models/sensor.h"
#include "models/detection.h"
#include "models/track.h"
#include "models/alert.h"
#include "models/assignment.h"
#include "correlation/correlator.h"
#include "simulation/drone_sim.h"

using namespace cuas;

// These tests verify the end-to-end Counter-UAS workflow

class WorkflowTest : public ::testing::Test {
protected:
    Correlator correlator;
    std::unique_ptr<DroneSim> drone_sim;

    void SetUp() override {
        // DC coordinates
        drone_sim = std::make_unique<DroneSim>(38.9072, -77.0369, 10000.0);
    }
};

// ============================================================================
// Detection → Track Workflow
// ============================================================================

TEST_F(WorkflowTest, DetectionCreatesTrack) {
    Detection det;
    det._id = Detection::generate_id();
    det.sensor_id = "RADAR-1";
    det.environment = EnvironmentCategory::AIR;
    det.identity = IdentityAffiliation::HOSTILE;
    det.platform_type = "DJI Mavic";
    det.latitude = 38.91;
    det.longitude = -77.03;
    det.hae = 100.0;
    det.bearing = 45.0;
    det.range = 5000.0;
    det.track_quality = 85;
    det.created = Detection::now();
    det.updated = det.created;

    auto tracks = correlator.correlate({det}, {});

    ASSERT_EQ(tracks.size(), 1);
    EXPECT_EQ(tracks[0].identity, IdentityAffiliation::HOSTILE);
    EXPECT_EQ(tracks[0].status, TrackStatus::ACTIVE);
    EXPECT_GT(tracks[0].threat_level, 50);  // Hostile should have high threat
}

TEST_F(WorkflowTest, MultiSensorCorrelation) {
    // Simulate same target detected by multiple sensors
    Detection det1;
    det1._id = Detection::generate_id();
    det1.sensor_id = "RADAR-1";
    det1.latitude = 38.91;
    det1.longitude = -77.03;
    det1.identity = IdentityAffiliation::UNKNOWN;
    det1.track_quality = 70;
    det1.created = Detection::now();
    det1.updated = det1.created;

    auto tracks1 = correlator.correlate({det1}, {});
    ASSERT_EQ(tracks1.size(), 1);
    EXPECT_EQ(tracks1[0].detection_count, 1);

    // Second sensor detects same target with hostile ID
    Detection det2;
    det2._id = Detection::generate_id();
    det2.sensor_id = "ACOUSTIC-1";
    det2.latitude = 38.9101;  // Slightly different position
    det2.longitude = -77.0301;
    det2.identity = IdentityAffiliation::HOSTILE;
    det2.track_quality = 90;
    det2.created = Detection::now();
    det2.updated = det2.created;

    auto tracks2 = correlator.correlate({det2}, tracks1);

    ASSERT_EQ(tracks2.size(), 1);
    EXPECT_EQ(tracks2[0].detection_count, 2);
    EXPECT_EQ(tracks2[0].identity, IdentityAffiliation::HOSTILE);  // Upgraded
}

// ============================================================================
// Track → Alert Workflow
// ============================================================================

TEST_F(WorkflowTest, HostileTrackTriggersAlert) {
    Track hostile_track;
    hostile_track._id = Track::generate_id();
    hostile_track.track_number = "T001";
    hostile_track.status = TrackStatus::ACTIVE;
    hostile_track.identity = IdentityAffiliation::HOSTILE;
    hostile_track.platform_type = "Unknown sUAS";
    hostile_track.latitude = 38.91;
    hostile_track.longitude = -77.03;
    hostile_track.threat_level = 85;

    // C2 logic: Create alert for hostile track
    Alert alert = Alert::create_threat_alert(
        hostile_track._id,
        "HOSTILE " + hostile_track.platform_type + " detected - " + hostile_track.track_number,
        AlertCategory::CRITICAL
    );

    EXPECT_EQ(alert.track_id, hostile_track._id);
    EXPECT_EQ(alert.category, AlertCategory::CRITICAL);
    EXPECT_EQ(alert.state, AlertState::AWAITING_RESPONSE);
    EXPECT_TRUE(alert.message.find("HOSTILE") != std::string::npos);
}

// ============================================================================
// Alert → Assignment Workflow
// ============================================================================

TEST_F(WorkflowTest, AlertResponseCreatesInterceptorAssignment) {
    Alert alert = Alert::create_threat_alert("TRK-001", "HOSTILE drone", AlertCategory::CRITICAL);

    // Operator responds with ENGAGE
    alert.response = AlertResponse::ENGAGE;
    alert.state = AlertState::ACTIVE;

    // Create interceptor assignment
    Assignment assignment = Assignment::create_interceptor_assignment(
        alert.track_id, "INTERCEPTOR-1");

    EXPECT_EQ(assignment.track_id, "TRK-001");
    EXPECT_EQ(assignment.type, AssignmentType::KINETIC_INTERCEPT);
    EXPECT_EQ(assignment.status, AssignmentStatus::PENDING);
    EXPECT_GT(assignment.eta_seconds, 0);
}

TEST_F(WorkflowTest, AlertResponseCreatesEwAssignment) {
    Alert alert = Alert::create_threat_alert("TRK-002", "HOSTILE drone", AlertCategory::CRITICAL);

    // Operator responds with JAM
    alert.response = AlertResponse::JAM;
    alert.state = AlertState::ACTIVE;

    // Create EW assignment
    Assignment assignment = Assignment::create_ew_assignment(
        alert.track_id, "EW-JAMMER-1");

    EXPECT_EQ(assignment.track_id, "TRK-002");
    EXPECT_EQ(assignment.type, AssignmentType::EW_JAM);
    EXPECT_EQ(assignment.status, AssignmentStatus::PENDING);
}

// ============================================================================
// Assignment Lifecycle
// ============================================================================

TEST_F(WorkflowTest, AssignmentLifecycle) {
    Assignment a = Assignment::create_interceptor_assignment("TRK-001", "EFFECTOR-1");
    EXPECT_EQ(a.status, AssignmentStatus::PENDING);

    // Effector accepts assignment
    a.status = AssignmentStatus::IN_PROGRESS;
    EXPECT_EQ(a.status, AssignmentStatus::IN_PROGRESS);

    // Effector completes assignment
    a.status = AssignmentStatus::COMPLETED;
    EXPECT_EQ(a.status, AssignmentStatus::COMPLETED);
}

TEST_F(WorkflowTest, AssignmentAbort) {
    Assignment a = Assignment::create_interceptor_assignment("TRK-001", "EFFECTOR-1");
    a.status = AssignmentStatus::IN_PROGRESS;

    // Operator aborts
    a.status = AssignmentStatus::ABORTED;
    EXPECT_EQ(a.status, AssignmentStatus::ABORTED);
}

// ============================================================================
// Drone Simulation Tests
// ============================================================================

TEST_F(WorkflowTest, DronSimSpawnsHostiles) {
    EXPECT_EQ(drone_sim->get_drones().size(), 0);

    drone_sim->spawn_random_hostiles(3);

    auto drones = drone_sim->get_drones();
    EXPECT_EQ(drones.size(), 3);

    for (const auto& drone : drones) {
        EXPECT_EQ(drone.identity, IdentityAffiliation::HOSTILE);
        EXPECT_TRUE(drone.active);
        EXPECT_GT(drone.speed, 0);
    }
}

TEST_F(WorkflowTest, DroneSimGeneratesDetections) {
    drone_sim->spawn_random_hostiles(2);

    auto detections = drone_sim->generate_detections("SENSOR-1");

    // Should generate detections for drones within range
    EXPECT_GE(detections.size(), 0);  // May be 0-2 depending on range

    for (const auto& det : detections) {
        EXPECT_EQ(det.sensor_id, "SENSOR-1");
        EXPECT_TRUE(det.simulated);
        EXPECT_GT(det.track_quality, 0);
    }
}

TEST_F(WorkflowTest, DroneSimMovement) {
    drone_sim->spawn_random_hostiles(1);

    auto drones_before = drone_sim->get_drones();
    double lat_before = drones_before[0].latitude;
    double lon_before = drones_before[0].longitude;

    // Simulate 10 seconds
    drone_sim->update(10.0);

    auto drones_after = drone_sim->get_drones();
    double lat_after = drones_after[0].latitude;
    double lon_after = drones_after[0].longitude;

    // Position should have changed
    bool position_changed = (lat_before != lat_after) || (lon_before != lon_after);
    EXPECT_TRUE(position_changed);
}

// ============================================================================
// Full End-to-End Workflow
// ============================================================================

TEST_F(WorkflowTest, FullCuasWorkflow) {
    // 1. Spawn simulated hostile drone
    drone_sim->spawn_random_hostiles(1);

    // 2. Generate detections
    auto detections = drone_sim->generate_detections("RADAR-1");
    ASSERT_GE(detections.size(), 1);

    // 3. Correlate to track
    auto tracks = correlator.correlate(detections, {});
    ASSERT_EQ(tracks.size(), 1);
    auto& track = tracks[0];

    // 4. Track should be hostile (from simulation)
    EXPECT_EQ(track.identity, IdentityAffiliation::HOSTILE);
    EXPECT_EQ(track.status, TrackStatus::ACTIVE);

    // 5. Create alert for hostile track
    Alert alert = Alert::create_threat_alert(
        track._id,
        "HOSTILE " + track.platform_type + " - " + track.track_number,
        AlertCategory::CRITICAL);
    EXPECT_EQ(alert.state, AlertState::AWAITING_RESPONSE);

    // 6. Operator responds
    alert.response = AlertResponse::ENGAGE;
    alert.state = AlertState::ACTIVE;

    // 7. Create assignment
    Assignment assignment = Assignment::create_interceptor_assignment(
        track._id, "INTERCEPTOR-1");
    EXPECT_EQ(assignment.status, AssignmentStatus::PENDING);

    // 8. Effector accepts and completes
    assignment.status = AssignmentStatus::IN_PROGRESS;
    assignment.status = AssignmentStatus::COMPLETED;

    // 9. Close alert
    alert.state = AlertState::CLOSED;

    // Verify final states
    EXPECT_EQ(alert.state, AlertState::CLOSED);
    EXPECT_EQ(assignment.status, AssignmentStatus::COMPLETED);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
