#ifndef CUAS_MESH_ASSIGNMENT_H
#define CUAS_MESH_ASSIGNMENT_H

#include <string>
#include <cstdint>
#include <nlohmann/json.hpp>
#include "enums.h"

namespace cuas {

// Effector tasking (interceptor/EW)
struct Assignment {
    std::string _id;
    std::string track_id;       // Target track
    std::string effector_id;    // Interceptor or EW system

    // MissionAssignmentType
    MissionType mission_type = MissionType::ENGAGE;

    // Effector type
    EffectorType effector_type = EffectorType::INTERCEPTOR;

    // Status
    AssignmentStatus status = AssignmentStatus::ASSIGNED;

    // Position (for interceptor tracking)
    double latitude = 0.0;
    double longitude = 0.0;
    double hae = 0.0;

    int64_t created = 0;
    int64_t eta_seconds = 0;
    int progress = 0;    // 0-100 percent complete (synced via Ditto)
    std::string result;  // "target_destroyed", "target_escaped", "jamming_active"

    bool deleted = false;

    // JSON serialization
    nlohmann::json to_json() const;
    static Assignment from_json(const nlohmann::json& j);

    // Generate unique ID
    static std::string generate_id();

    // Factory methods
    static Assignment create_interceptor_assignment(const std::string& track_id,
                                                    const std::string& effector_id);
    static Assignment create_ew_assignment(const std::string& track_id,
                                           const std::string& effector_id);
};

// JSON conversion
void to_json(nlohmann::json& j, const Assignment& a);
void from_json(const nlohmann::json& j, Assignment& a);

} // namespace cuas

#endif // CUAS_MESH_ASSIGNMENT_H
