#ifndef CUAS_MESH_ENUMS_H
#define CUAS_MESH_ENUMS_H

#include <string>

namespace cuas {

// Based on OMNI proto: common.proto - EnvironmentCategory
enum class EnvironmentCategory {
    AIR = 1,
    SURFACE = 2,
    SUB_SURFACE = 3,
    LAND = 4,
    SPACE = 5,
    SENSOR_POI = 9
};

// Based on OMNI proto: common.proto - IdentityAffiliation
enum class IdentityAffiliation {
    PENDING = 0,
    UNKNOWN = 1,
    ASSUMED_FRIEND = 2,
    FRIEND = 3,
    NEUTRAL = 4,
    SUSPECT = 5,
    HOSTILE = 6,
    JOKER = 7,
    FAKER = 8
};

// Based on OMNI proto: sensorevent.proto - SensorStatus
enum class SensorStatus {
    NEW_SENSOR_REPORT = 1,
    ENGAGING = 2,
    MARK_POINT = 8,
    COVERING = 9,
    DISENGAGING = 10,
    CANCEL_SENSOR_REPORT = 11
};

// Based on OMNI proto: alertevent.proto - CategoryEnum
enum class AlertCategory {
    CRITICAL = 1,   // CAT_1: Operator action required
    MAJOR = 2,      // CAT_2: Operator acknowledgment needed
    ROUTINE = 3     // CAT_3: Acknowledgment or condition resolution
};

// Based on OMNI proto: alertevent.proto - AlertStateEnum
enum class AlertState {
    CLOSED = 0,
    AWAITING_RESPONSE = 1,
    ACTIVE = 2,
    TIMED_OUT = 3,
    INVALID_RESPONSE = 4
};

// Based on OMNI proto: alertevent.proto - AlertTypeEnum
enum class AlertType {
    THREAT = 9,              // THREAT_ATE
    TRACK_UPDATE = 20,
    IDENTITY_CHANGE = 26
};

// Based on OMNI proto: alertevent.proto - AlertResponseEnum
enum class AlertResponse {
    NONE = 0,
    ACKNOWLEDGE = 5,
    ACCEPT = 6,
    REJECT = 7,
    ACTIVATE = 9,
    DEACTIVATE = 10
};

// Based on OMNI proto: common.proto - MissionAssignmentType
enum class MissionType {
    ENGAGE = 5,
    BREAK_ENGAGEMENT = 7,
    ATTACK = 41,
    CEASE_ATTACK = 42
};

// Based on OMNI proto: common.proto - RelationType
enum class RelationType {
    PARENT = 1,
    PRODUCER = 2,
    OWNER = 3,
    CORRELATED = 7,
    FUSED = 8,
    COMPOSITE = 9
};

// Effector types
enum class EffectorType {
    INTERCEPTOR,
    EW_JAMMER
};

// Assignment status
enum class AssignmentStatus {
    ASSIGNED,
    IN_TRANSIT,
    ENGAGING,
    COMPLETE,
    ABORTED
};

// Track status
enum class TrackStatus {
    ACTIVE,
    LOST,
    KILLED
};

// String conversion utilities
inline std::string identityToString(IdentityAffiliation id) {
    switch (id) {
        case IdentityAffiliation::PENDING: return "PENDING";
        case IdentityAffiliation::UNKNOWN: return "UNKNOWN";
        case IdentityAffiliation::ASSUMED_FRIEND: return "ASSUMED_FRIEND";
        case IdentityAffiliation::FRIEND: return "FRIEND";
        case IdentityAffiliation::NEUTRAL: return "NEUTRAL";
        case IdentityAffiliation::SUSPECT: return "SUSPECT";
        case IdentityAffiliation::HOSTILE: return "HOSTILE";
        case IdentityAffiliation::JOKER: return "JOKER";
        case IdentityAffiliation::FAKER: return "FAKER";
        default: return "UNKNOWN";
    }
}

inline std::string alertCategoryToString(AlertCategory cat) {
    switch (cat) {
        case AlertCategory::CRITICAL: return "CAT_1";
        case AlertCategory::MAJOR: return "CAT_2";
        case AlertCategory::ROUTINE: return "CAT_3";
        default: return "UNKNOWN";
    }
}

inline std::string alertStateToString(AlertState state) {
    switch (state) {
        case AlertState::CLOSED: return "CLOSED";
        case AlertState::AWAITING_RESPONSE: return "AWAITING";
        case AlertState::ACTIVE: return "ACTIVE";
        case AlertState::TIMED_OUT: return "TIMED_OUT";
        case AlertState::INVALID_RESPONSE: return "INVALID";
        default: return "UNKNOWN";
    }
}

inline std::string sensorStatusToString(SensorStatus status) {
    switch (status) {
        case SensorStatus::NEW_SENSOR_REPORT: return "NEW";
        case SensorStatus::ENGAGING: return "ENGAGING";
        case SensorStatus::COVERING: return "COVERING";
        case SensorStatus::DISENGAGING: return "DISENGAGING";
        default: return "UNKNOWN";
    }
}

inline std::string effectorTypeToString(EffectorType type) {
    switch (type) {
        case EffectorType::INTERCEPTOR: return "interceptor";
        case EffectorType::EW_JAMMER: return "ew_jammer";
        default: return "unknown";
    }
}

inline std::string assignmentStatusToString(AssignmentStatus status) {
    switch (status) {
        case AssignmentStatus::ASSIGNED: return "assigned";
        case AssignmentStatus::IN_TRANSIT: return "in_transit";
        case AssignmentStatus::ENGAGING: return "engaging";
        case AssignmentStatus::COMPLETE: return "complete";
        case AssignmentStatus::ABORTED: return "aborted";
        default: return "unknown";
    }
}

inline std::string trackStatusToString(TrackStatus status) {
    switch (status) {
        case TrackStatus::ACTIVE: return "active";
        case TrackStatus::LOST: return "lost";
        case TrackStatus::KILLED: return "killed";
        default: return "unknown";
    }
}

} // namespace cuas

#endif // CUAS_MESH_ENUMS_H
