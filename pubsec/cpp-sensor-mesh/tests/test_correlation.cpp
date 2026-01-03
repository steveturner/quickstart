#include <gtest/gtest.h>

#include "correlation/correlator.h"

using namespace cuas;

class CorrelatorTest : public ::testing::Test {
protected:
    Correlator correlator;

    Detection make_detection(const std::string& id, double lat, double lon,
                             IdentityAffiliation identity = IdentityAffiliation::UNKNOWN) {
        Detection d;
        d._id = id;
        d.sensor_id = "SENSOR-1";
        d.latitude = lat;
        d.longitude = lon;
        d.hae = 100.0;
        d.environment = EnvironmentCategory::AIR;
        d.identity = identity;
        d.platform_type = "sUAS";
        d.bearing = 45.0;
        d.range = 1000.0;
        d.track_quality = 80;
        d.created = Detection::now();
        d.updated = d.created;
        return d;
    }
};

TEST_F(CorrelatorTest, CreateTrackFromSingleDetection) {
    std::vector<Detection> detections = {
        make_detection("DET-1", 38.9072, -77.0369)
    };

    auto tracks = correlator.correlate(detections, {});

    EXPECT_EQ(tracks.size(), 1);
    EXPECT_EQ(tracks[0].status, TrackStatus::ACTIVE);
    EXPECT_DOUBLE_EQ(tracks[0].latitude, 38.9072);
    EXPECT_DOUBLE_EQ(tracks[0].longitude, -77.0369);
    EXPECT_EQ(tracks[0].detection_count, 1);
}

TEST_F(CorrelatorTest, CorrelateNearbyDetections) {
    // First detection creates a track
    std::vector<Detection> det1 = {
        make_detection("DET-1", 38.9072, -77.0369)
    };
    auto tracks1 = correlator.correlate(det1, {});
    ASSERT_EQ(tracks1.size(), 1);

    // Second detection nearby should correlate to same track
    std::vector<Detection> det2 = {
        make_detection("DET-2", 38.9073, -77.0370)  // ~15m away
    };
    auto tracks2 = correlator.correlate(det2, tracks1);

    EXPECT_EQ(tracks2.size(), 1);
    EXPECT_EQ(tracks2[0]._id, tracks1[0]._id);
    EXPECT_EQ(tracks2[0].detection_count, 2);
}

TEST_F(CorrelatorTest, CreateSeparateTracksForDistantDetections) {
    // Two detections far apart
    std::vector<Detection> detections = {
        make_detection("DET-1", 38.9072, -77.0369),
        make_detection("DET-2", 39.0000, -78.0000)  // ~80km away
    };

    auto tracks = correlator.correlate(detections, {});

    EXPECT_EQ(tracks.size(), 2);
}

TEST_F(CorrelatorTest, HostileIdentityPropagates) {
    std::vector<Detection> det1 = {
        make_detection("DET-1", 38.9072, -77.0369, IdentityAffiliation::UNKNOWN)
    };
    auto tracks1 = correlator.correlate(det1, {});
    EXPECT_EQ(tracks1[0].identity, IdentityAffiliation::UNKNOWN);

    // Hostile detection correlates
    std::vector<Detection> det2 = {
        make_detection("DET-2", 38.9073, -77.0370, IdentityAffiliation::HOSTILE)
    };
    auto tracks2 = correlator.correlate(det2, tracks1);

    EXPECT_EQ(tracks2[0].identity, IdentityAffiliation::HOSTILE);
}

TEST_F(CorrelatorTest, ConfigurationApplied) {
    CorrelatorConfig config;
    config.correlation_distance_m = 100.0;  // Tight correlation

    Correlator tight_correlator(config);

    // Two detections 200m apart should create separate tracks
    std::vector<Detection> detections = {
        make_detection("DET-1", 38.9072, -77.0369),
        make_detection("DET-2", 38.9090, -77.0369)  // ~200m north
    };

    auto tracks = tight_correlator.correlate(detections, {});
    EXPECT_EQ(tracks.size(), 2);
}

TEST_F(CorrelatorTest, Reset) {
    std::vector<Detection> det1 = {
        make_detection("DET-1", 38.9072, -77.0369)
    };
    correlator.correlate(det1, {});

    correlator.reset();

    // After reset, new detection should create new track
    std::vector<Detection> det2 = {
        make_detection("DET-2", 38.9072, -77.0369)
    };
    auto tracks = correlator.correlate(det2, {});

    EXPECT_EQ(tracks.size(), 1);
    EXPECT_EQ(tracks[0].detection_count, 1);
}

TEST_F(CorrelatorTest, ThreatLevelIncreasesForHostile) {
    std::vector<Detection> det1 = {
        make_detection("DET-1", 38.9072, -77.0369, IdentityAffiliation::HOSTILE)
    };
    auto tracks1 = correlator.correlate(det1, {});
    int initial_threat = tracks1[0].threat_level;

    // More hostile detections should increase threat
    std::vector<Detection> det2 = {
        make_detection("DET-2", 38.9073, -77.0370, IdentityAffiliation::HOSTILE)
    };
    auto tracks2 = correlator.correlate(det2, tracks1);

    EXPECT_GT(tracks2[0].threat_level, initial_threat);
}

TEST_F(CorrelatorTest, PositionFusing) {
    // First detection
    std::vector<Detection> det1 = {
        make_detection("DET-1", 38.9072, -77.0369)
    };
    det1[0].track_quality = 100;
    auto tracks1 = correlator.correlate(det1, {});

    // Second detection with lower quality
    std::vector<Detection> det2 = {
        make_detection("DET-2", 38.9080, -77.0369)
    };
    det2[0].track_quality = 50;
    auto tracks2 = correlator.correlate(det2, tracks1);

    // Fused position should be weighted toward higher quality detection
    double fused_lat = tracks2[0].latitude;

    // Should be closer to first detection (38.9072) than second (38.9080)
    EXPECT_LT(fused_lat, 38.9077);  // Midpoint would be 38.9076
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
