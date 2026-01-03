#ifndef CUAS_MESH_ALERT_H
#define CUAS_MESH_ALERT_H

#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>
#include "enums.h"

namespace cuas {

// Based on OMNI proto: alertevent.proto
struct Alert {
    std::string _id;
    std::string track_id;       // Related track
    std::string message;

    // CategoryEnum (severity)
    AlertCategory category = AlertCategory::ROUTINE;

    // AlertTypeEnum
    AlertType alert_type = AlertType::THREAT;

    // AlertStateEnum
    AlertState state = AlertState::AWAITING_RESPONSE;

    // Response handling
    std::vector<AlertResponse> allowed_responses;
    AlertResponse response = AlertResponse::NONE;

    int64_t created = 0;
    int64_t timeout = 60000;  // 60 seconds default

    bool deleted = false;

    // JSON serialization
    nlohmann::json to_json() const;
    static Alert from_json(const nlohmann::json& j);

    // Generate unique ID
    static std::string generate_id();

    // Factory for threat alerts
    static Alert create_threat_alert(const std::string& track_id,
                                     const std::string& message,
                                     AlertCategory category = AlertCategory::CRITICAL);
};

// JSON conversion
void to_json(nlohmann::json& j, const Alert& a);
void from_json(const nlohmann::json& j, Alert& a);

} // namespace cuas

#endif // CUAS_MESH_ALERT_H
