#ifndef CUAS_MESH_DETECTION_H
#define CUAS_MESH_DETECTION_H

#include <string>
#include <cstdint>
#include <nlohmann/json.hpp>
#include "enums.h"

namespace cuas {

// Based on OMNI proto: trackevent.proto, geopoint.proto, acousticinfo.proto
struct Detection {
    std::string _id;
    std::string sensor_id;  // Producer sensor

    // FunctionalIdentity (target classification)
    EnvironmentCategory environment = EnvironmentCategory::AIR;
    IdentityAffiliation identity = IdentityAffiliation::PENDING;
    std::string platform_type;  // "small_uav", "group1_uas", "quadcopter"

    // Geopoint (full position)
    double latitude = 0.0;
    double longitude = 0.0;
    double hae = 0.0;           // Height above ellipsoid
    double circular_error = 0.0; // Horizontal accuracy (meters)
    double linear_error = 0.0;  // Vertical accuracy (meters)
    double course = 0.0;        // Heading (degrees true north)
    double speed = 0.0;         // m/s

    // TrackIndicators
    int track_quality = 0;      // 0-100
    int strength = 0;           // Signal strength
    bool simulated = false;
    bool special_interest = false;

    // Acoustic/RF specific
    double bearing = 0.0;       // Bearing from sensor (degrees)
    double range = 0.0;         // Estimated range (meters)
    double bearing_accuracy = 0.0;

    // Timing (TimeOfValidity)
    int64_t created = 0;
    int64_t updated = 0;
    int64_t timeout = 30000;    // 30 seconds default

    bool deleted = false;

    // JSON serialization
    nlohmann::json to_json() const;
    static Detection from_json(const nlohmann::json& j);

    // Generate unique ID
    static std::string generate_id();

    // Get current timestamp
    static int64_t now();
};

// JSON conversion
void to_json(nlohmann::json& j, const Detection& d);
void from_json(const nlohmann::json& j, Detection& d);

} // namespace cuas

#endif // CUAS_MESH_DETECTION_H
