#ifndef CUAS_MESH_SENSOR_H
#define CUAS_MESH_SENSOR_H

#include <string>
#include <cstdint>
#include <nlohmann/json.hpp>
#include "enums.h"

namespace cuas {

// Based on OMNI proto: sensorevent.proto, geopoint.proto
struct Sensor {
    std::string _id;
    std::string name;

    // FunctionalIdentity
    EnvironmentCategory environment = EnvironmentCategory::LAND;
    std::string platform_type;  // "radar", "acoustic", "rf", "eo"

    // Geopoint
    double latitude = 0.0;
    double longitude = 0.0;
    double hae = 0.0;           // Height above ellipsoid
    double circular_error = 0.0; // Position accuracy (meters)

    // SensorStatus
    SensorStatus status = SensorStatus::COVERING;

    // Operational parameters
    double detection_range_m = 5000.0;
    double bearing_accuracy_deg = 2.0;
    bool active = true;
    int64_t last_heartbeat = 0;

    bool deleted = false;

    // JSON serialization
    nlohmann::json to_json() const;
    static Sensor from_json(const nlohmann::json& j);

    // Generate unique ID
    static std::string generate_id();
};

// JSON conversion
void to_json(nlohmann::json& j, const Sensor& s);
void from_json(const nlohmann::json& j, Sensor& s);

} // namespace cuas

#endif // CUAS_MESH_SENSOR_H
