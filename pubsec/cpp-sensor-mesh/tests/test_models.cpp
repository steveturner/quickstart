#include <gtest/gtest.h>

#include "models/enums.h"
#include "models/sensor.h"
#include "models/detection.h"
#include "models/track.h"
#include "models/alert.h"
#include "models/assignment.h"

using namespace cuas;

// ============================================================================
// Enum Tests
// ============================================================================

TEST(EnumsTest, IdentityToString) {
    EXPECT_EQ(identityToString(IdentityAffiliation::HOSTILE), "HOSTILE");
    EXPECT_EQ(identityToString(IdentityAffiliation::UNKNOWN), "UNKNOWN");
    EXPECT_EQ(identityToString(IdentityAffiliation::FRIENDLY), "FRIENDLY");
}

TEST(EnumsTest, AlertCategoryToString) {
    EXPECT_EQ(alertCategoryToString(AlertCategory::CRITICAL), "CRITICAL");
    EXPECT_EQ(alertCategoryToString(AlertCategory::MAJOR), "MAJOR");
    EXPECT_EQ(alertCategoryToString(AlertCategory::ROUTINE), "ROUTINE");
}

TEST(EnumsTest, EffectorTypeToString) {
    EXPECT_EQ(effectorTypeToString(EffectorType::INTERCEPTOR), "INTERCEPTOR");
    EXPECT_EQ(effectorTypeToString(EffectorType::EW_JAMMER), "EW_JAMMER");
}

// ============================================================================
// Sensor Tests
// ============================================================================

TEST(SensorTest, DefaultConstruction) {
    Sensor s;
    EXPECT_TRUE(s._id.empty());
    EXPECT_EQ(s.status, SensorStatus::OFFLINE);
    EXPECT_FALSE(s.active);
}

TEST(SensorTest, JsonSerialization) {
    Sensor s;
    s._id = "SENSOR-001";
    s.name = "Radar Alpha";
    s.platform_type = "radar";
    s.latitude = 38.9072;
    s.longitude = -77.0369;
    s.hae = 100.0;
    s.status = SensorStatus::COVERING;
    s.active = true;
    s.detection_range_m = 10000.0;
    s.bearing_accuracy_deg = 1.0;

    nlohmann::json j = s.to_json();

    EXPECT_EQ(j["_id"], "SENSOR-001");
    EXPECT_EQ(j["name"], "Radar Alpha");
    EXPECT_EQ(j["platform_type"], "radar");
    EXPECT_DOUBLE_EQ(j["latitude"], 38.9072);
    EXPECT_TRUE(j["active"]);
}

TEST(SensorTest, JsonDeserialization) {
    nlohmann::json j = {
        {"_id", "SENSOR-002"},
        {"name", "Acoustic Beta"},
        {"platform_type", "acoustic"},
        {"latitude", 39.0},
        {"longitude", -78.0},
        {"hae", 50.0},
        {"status", static_cast<int>(SensorStatus::COVERING)},
        {"active", true},
        {"detection_range_m", 3000.0},
        {"bearing_accuracy_deg", 5.0},
        {"last_heartbeat", 1234567890}
    };

    Sensor s = Sensor::from_json(j);

    EXPECT_EQ(s._id, "SENSOR-002");
    EXPECT_EQ(s.platform_type, "acoustic");
    EXPECT_DOUBLE_EQ(s.latitude, 39.0);
    EXPECT_EQ(s.status, SensorStatus::COVERING);
}

// ============================================================================
// Detection Tests
// ============================================================================

TEST(DetectionTest, GenerateId) {
    std::string id1 = Detection::generate_id();
    std::string id2 = Detection::generate_id();

    EXPECT_FALSE(id1.empty());
    EXPECT_FALSE(id2.empty());
    EXPECT_NE(id1, id2);
}

TEST(DetectionTest, NowReturnsTimestamp) {
    int64_t t1 = Detection::now();
    int64_t t2 = Detection::now();

    EXPECT_GT(t1, 0);
    EXPECT_GE(t2, t1);
}

TEST(DetectionTest, JsonRoundtrip) {
    Detection d;
    d._id = "DET-001";
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
    d.simulated = true;

    nlohmann::json j = d.to_json();
    Detection d2 = Detection::from_json(j);

    EXPECT_EQ(d2._id, d._id);
    EXPECT_EQ(d2.sensor_id, d.sensor_id);
    EXPECT_EQ(d2.identity, d.identity);
    EXPECT_EQ(d2.platform_type, d.platform_type);
    EXPECT_DOUBLE_EQ(d2.bearing, d.bearing);
    EXPECT_EQ(d2.track_quality, d.track_quality);
    EXPECT_EQ(d2.simulated, d.simulated);
}

// ============================================================================
// Track Tests
// ============================================================================

TEST(TrackTest, GenerateId) {
    std::string id = Track::generate_id();
    EXPECT_FALSE(id.empty());
    EXPECT_EQ(id.substr(0, 4), "TRK-");
}

TEST(TrackTest, JsonRoundtrip) {
    Track t;
    t._id = "TRK-001";
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

    nlohmann::json j = t.to_json();
    Track t2 = Track::from_json(j);

    EXPECT_EQ(t2._id, t._id);
    EXPECT_EQ(t2.track_number, t.track_number);
    EXPECT_EQ(t2.status, t.status);
    EXPECT_EQ(t2.identity, t.identity);
    EXPECT_EQ(t2.threat_level, t.threat_level);
    EXPECT_EQ(t2.detection_count, t.detection_count);
}

// ============================================================================
// Alert Tests
// ============================================================================

TEST(AlertTest, CreateThreatAlert) {
    Alert a = Alert::create_threat_alert("TRK-001", "HOSTILE drone detected", AlertCategory::CRITICAL);

    EXPECT_FALSE(a._id.empty());
    EXPECT_EQ(a.track_id, "TRK-001");
    EXPECT_EQ(a.message, "HOSTILE drone detected");
    EXPECT_EQ(a.category, AlertCategory::CRITICAL);
    EXPECT_EQ(a.state, AlertState::AWAITING_RESPONSE);
    EXPECT_GT(a.created, 0);
}

TEST(AlertTest, JsonRoundtrip) {
    Alert a = Alert::create_threat_alert("TRK-002", "Test alert", AlertCategory::MAJOR);

    nlohmann::json j = a.to_json();
    Alert a2 = Alert::from_json(j);

    EXPECT_EQ(a2._id, a._id);
    EXPECT_EQ(a2.track_id, a.track_id);
    EXPECT_EQ(a2.message, a.message);
    EXPECT_EQ(a2.category, a.category);
    EXPECT_EQ(a2.state, a.state);
}

// ============================================================================
// Assignment Tests
// ============================================================================

TEST(AssignmentTest, CreateInterceptorAssignment) {
    Assignment a = Assignment::create_interceptor_assignment("TRK-001", "EFFECTOR-1");

    EXPECT_FALSE(a._id.empty());
    EXPECT_EQ(a.track_id, "TRK-001");
    EXPECT_EQ(a.effector_id, "EFFECTOR-1");
    EXPECT_EQ(a.type, AssignmentType::KINETIC_INTERCEPT);
    EXPECT_EQ(a.status, AssignmentStatus::PENDING);
    EXPECT_GT(a.eta_seconds, 0);
}

TEST(AssignmentTest, CreateEwAssignment) {
    Assignment a = Assignment::create_ew_assignment("TRK-002", "EW-1");

    EXPECT_EQ(a.track_id, "TRK-002");
    EXPECT_EQ(a.effector_id, "EW-1");
    EXPECT_EQ(a.type, AssignmentType::EW_JAM);
    EXPECT_EQ(a.status, AssignmentStatus::PENDING);
}

TEST(AssignmentTest, JsonRoundtrip) {
    Assignment a = Assignment::create_interceptor_assignment("TRK-003", "EFFECTOR-2");
    a.status = AssignmentStatus::IN_PROGRESS;

    nlohmann::json j = a.to_json();
    Assignment a2 = Assignment::from_json(j);

    EXPECT_EQ(a2._id, a._id);
    EXPECT_EQ(a2.track_id, a.track_id);
    EXPECT_EQ(a2.effector_id, a.effector_id);
    EXPECT_EQ(a2.type, a.type);
    EXPECT_EQ(a2.status, a.status);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
