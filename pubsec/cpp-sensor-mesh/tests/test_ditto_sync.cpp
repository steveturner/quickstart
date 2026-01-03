#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "models/sensor.h"
#include "models/detection.h"
#include "models/track.h"
#include "models/alert.h"
#include "models/assignment.h"
#include "ditto/collections.h"

using namespace cuas;

// These tests verify the data model JSON serialization is compatible
// with Ditto's document structure requirements.

class DittoSyncTest : public ::testing::Test {
protected:
    // Verify JSON has required _id field for Ditto documents
    void verify_has_id(const nlohmann::json& j) {
        ASSERT_TRUE(j.contains("_id"));
        ASSERT_TRUE(j["_id"].is_string());
        EXPECT_FALSE(j["_id"].get<std::string>().empty());
    }
};

// ============================================================================
// Collection Name Tests
// ============================================================================

TEST_F(DittoSyncTest, CollectionNamesAreDefined) {
    EXPECT_FALSE(std::string(COLLECTION_SENSORS).empty());
    EXPECT_FALSE(std::string(COLLECTION_DETECTIONS).empty());
    EXPECT_FALSE(std::string(COLLECTION_TRACKS).empty());
    EXPECT_FALSE(std::string(COLLECTION_ALERTS).empty());
    EXPECT_FALSE(std::string(COLLECTION_ASSIGNMENTS).empty());
}

// ============================================================================
// Sensor Sync Tests
// ============================================================================

TEST_F(DittoSyncTest, SensorDocumentStructure) {
    Sensor s;
    s._id = "SENSOR-001";
    s.name = "Radar Alpha";
    s.platform_type = "radar";
    s.latitude = 38.9072;
    s.longitude = -77.0369;
    s.hae = 0.0;
    s.status = SensorStatus::COVERING;
    s.active = true;
    s.detection_range_m = 10000.0;
    s.bearing_accuracy_deg = 1.0;
    s.last_heartbeat = Detection::now();

    nlohmann::json j = s.to_json();

    verify_has_id(j);
    EXPECT_TRUE(j.contains("latitude"));
    EXPECT_TRUE(j.contains("longitude"));
    EXPECT_TRUE(j.contains("active"));
    EXPECT_TRUE(j.contains("last_heartbeat"));

    // Verify numeric types
    EXPECT_TRUE(j["latitude"].is_number());
    EXPECT_TRUE(j["longitude"].is_number());
    EXPECT_TRUE(j["active"].is_boolean());
    EXPECT_TRUE(j["last_heartbeat"].is_number());
}

TEST_F(DittoSyncTest, SensorRoundtripPreservesData) {
    Sensor original;
    original._id = "SENSOR-002";
    original.name = "Test Sensor";
    original.platform_type = "acoustic";
    original.latitude = 39.5;
    original.longitude = -76.5;
    original.hae = 100.0;
    original.status = SensorStatus::COVERING;
    original.active = true;
    original.detection_range_m = 5000.0;
    original.bearing_accuracy_deg = 3.0;
    original.last_heartbeat = 1234567890;

    nlohmann::json j = original.to_json();
    Sensor restored = Sensor::from_json(j);

    EXPECT_EQ(restored._id, original._id);
    EXPECT_EQ(restored.name, original.name);
    EXPECT_EQ(restored.platform_type, original.platform_type);
    EXPECT_DOUBLE_EQ(restored.latitude, original.latitude);
    EXPECT_DOUBLE_EQ(restored.longitude, original.longitude);
    EXPECT_DOUBLE_EQ(restored.hae, original.hae);
    EXPECT_EQ(restored.status, original.status);
    EXPECT_EQ(restored.active, original.active);
    EXPECT_DOUBLE_EQ(restored.detection_range_m, original.detection_range_m);
    EXPECT_DOUBLE_EQ(restored.bearing_accuracy_deg, original.bearing_accuracy_deg);
    EXPECT_EQ(restored.last_heartbeat, original.last_heartbeat);
}

// ============================================================================
// Detection Sync Tests
// ============================================================================

TEST_F(DittoSyncTest, DetectionDocumentStructure) {
    Detection d;
    d._id = Detection::generate_id();
    d.sensor_id = "SENSOR-001";
    d.environment = EnvironmentCategory::AIR;
    d.identity = IdentityAffiliation::HOSTILE;
    d.platform_type = "DJI Mavic";
    d.latitude = 38.9;
    d.longitude = -77.0;
    d.hae = 150.0;
    d.bearing = 45.0;
    d.range = 2000.0;
    d.track_quality = 85;
    d.simulated = false;
    d.created = Detection::now();
    d.updated = d.created;

    nlohmann::json j = d.to_json();

    verify_has_id(j);
    EXPECT_TRUE(j.contains("sensor_id"));
    EXPECT_TRUE(j.contains("identity"));
    EXPECT_TRUE(j.contains("latitude"));
    EXPECT_TRUE(j.contains("longitude"));
    EXPECT_TRUE(j.contains("bearing"));
    EXPECT_TRUE(j.contains("range"));
    EXPECT_TRUE(j.contains("created"));
    EXPECT_TRUE(j.contains("updated"));
}

TEST_F(DittoSyncTest, DetectionIdUniqueness) {
    std::set<std::string> ids;
    for (int i = 0; i < 100; ++i) {
        ids.insert(Detection::generate_id());
    }
    EXPECT_EQ(ids.size(), 100);  // All IDs should be unique
}

// ============================================================================
// Track Sync Tests
// ============================================================================

TEST_F(DittoSyncTest, TrackDocumentStructure) {
    Track t;
    t._id = Track::generate_id();
    t.track_number = "T42";
    t.status = TrackStatus::ACTIVE;
    t.identity = IdentityAffiliation::HOSTILE;
    t.environment = EnvironmentCategory::AIR;
    t.platform_type = "Unknown sUAS";
    t.latitude = 38.91;
    t.longitude = -77.01;
    t.hae = 100.0;
    t.threat_level = 80;
    t.detection_count = 5;
    t.last_detection_id = "DET-001";
    t.created = Track::now();
    t.updated = t.created;

    nlohmann::json j = t.to_json();

    verify_has_id(j);
    EXPECT_TRUE(j.contains("track_number"));
    EXPECT_TRUE(j.contains("status"));
    EXPECT_TRUE(j.contains("identity"));
    EXPECT_TRUE(j.contains("threat_level"));
    EXPECT_TRUE(j.contains("detection_count"));
}

// ============================================================================
// Alert Sync Tests
// ============================================================================

TEST_F(DittoSyncTest, AlertDocumentStructure) {
    Alert a = Alert::create_threat_alert("TRK-001", "HOSTILE detected", AlertCategory::CRITICAL);

    nlohmann::json j = a.to_json();

    verify_has_id(j);
    EXPECT_TRUE(j.contains("track_id"));
    EXPECT_TRUE(j.contains("message"));
    EXPECT_TRUE(j.contains("category"));
    EXPECT_TRUE(j.contains("state"));
    EXPECT_TRUE(j.contains("response"));
    EXPECT_TRUE(j.contains("created"));
}

TEST_F(DittoSyncTest, AlertStateTransitions) {
    Alert a = Alert::create_threat_alert("TRK-001", "Test", AlertCategory::CRITICAL);
    EXPECT_EQ(a.state, AlertState::AWAITING_RESPONSE);

    // Simulate state transition
    a.state = AlertState::ACTIVE;
    a.response = AlertResponse::ENGAGE;

    nlohmann::json j = a.to_json();
    Alert restored = Alert::from_json(j);

    EXPECT_EQ(restored.state, AlertState::ACTIVE);
    EXPECT_EQ(restored.response, AlertResponse::ENGAGE);
}

// ============================================================================
// Assignment Sync Tests
// ============================================================================

TEST_F(DittoSyncTest, AssignmentDocumentStructure) {
    Assignment a = Assignment::create_interceptor_assignment("TRK-001", "EFFECTOR-1");

    nlohmann::json j = a.to_json();

    verify_has_id(j);
    EXPECT_TRUE(j.contains("track_id"));
    EXPECT_TRUE(j.contains("effector_id"));
    EXPECT_TRUE(j.contains("type"));
    EXPECT_TRUE(j.contains("status"));
    EXPECT_TRUE(j.contains("eta_seconds"));
    EXPECT_TRUE(j.contains("created"));
    EXPECT_TRUE(j.contains("updated"));
}

TEST_F(DittoSyncTest, AssignmentStatusUpdate) {
    Assignment a = Assignment::create_interceptor_assignment("TRK-001", "EFFECTOR-1");
    EXPECT_EQ(a.status, AssignmentStatus::PENDING);

    // Simulate status update from another peer
    nlohmann::json j = a.to_json();
    j["status"] = static_cast<int>(AssignmentStatus::IN_PROGRESS);

    Assignment updated = Assignment::from_json(j);
    EXPECT_EQ(updated.status, AssignmentStatus::IN_PROGRESS);
}

// ============================================================================
// CRDT-Friendly Structure Tests
// ============================================================================

TEST_F(DittoSyncTest, DocumentsHaveTimestamps) {
    // All documents should have created/updated timestamps for conflict resolution

    Detection d;
    d._id = Detection::generate_id();
    d.created = Detection::now();
    d.updated = d.created;
    nlohmann::json dj = d.to_json();
    EXPECT_GT(dj["created"].get<int64_t>(), 0);
    EXPECT_GT(dj["updated"].get<int64_t>(), 0);

    Track t;
    t._id = Track::generate_id();
    t.created = Track::now();
    t.updated = t.created;
    nlohmann::json tj = t.to_json();
    EXPECT_GT(tj["created"].get<int64_t>(), 0);
    EXPECT_GT(tj["updated"].get<int64_t>(), 0);

    Alert a = Alert::create_threat_alert("TRK-001", "Test", AlertCategory::CRITICAL);
    nlohmann::json aj = a.to_json();
    EXPECT_GT(aj["created"].get<int64_t>(), 0);
    EXPECT_GT(aj["updated"].get<int64_t>(), 0);

    Assignment assign = Assignment::create_interceptor_assignment("TRK-001", "EFF-1");
    nlohmann::json assignj = assign.to_json();
    EXPECT_GT(assignj["created"].get<int64_t>(), 0);
    EXPECT_GT(assignj["updated"].get<int64_t>(), 0);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
