#ifndef CUAS_MESH_TRACK_H
#define CUAS_MESH_TRACK_H

#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>
#include "enums.h"

namespace cuas {

// Fused track from multiple detections
struct Track {
    std::string _id;
    std::string track_number;  // Human-readable e.g., "TN-0042"

    // Fused position
    double latitude = 0.0;
    double longitude = 0.0;
    double hae = 0.0;
    double course = 0.0;
    double speed = 0.0;

    // Classification (best estimate)
    IdentityAffiliation identity = IdentityAffiliation::PENDING;
    std::string platform_type;
    int track_quality = 0;

    // Correlation
    std::vector<std::string> detection_ids;
    RelationType relation_type = RelationType::FUSED;

    // Status
    TrackStatus status = TrackStatus::ACTIVE;
    int64_t last_update = 0;

    bool deleted = false;

    // JSON serialization
    nlohmann::json to_json() const;
    static Track from_json(const nlohmann::json& j);

    // Generate unique ID
    static std::string generate_id();

    // Generate track number
    static std::string generate_track_number();
};

// JSON conversion
void to_json(nlohmann::json& j, const Track& t);
void from_json(const nlohmann::json& j, Track& t);

} // namespace cuas

#endif // CUAS_MESH_TRACK_H
