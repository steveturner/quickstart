#include "assignment.h"
#include <chrono>
#include <random>
#include <sstream>

namespace cuas {

nlohmann::json Assignment::to_json() const {
    return nlohmann::json{
        {"_id", _id},
        {"track_id", track_id},
        {"effector_id", effector_id},
        {"mission_type", static_cast<int>(mission_type)},
        {"effector_type", effectorTypeToString(effector_type)},
        {"status", assignmentStatusToString(status)},
        {"latitude", latitude},
        {"longitude", longitude},
        {"hae", hae},
        {"created", created},
        {"eta_seconds", eta_seconds},
        {"progress", progress},
        {"result", result},
        {"deleted", deleted}
    };
}

Assignment Assignment::from_json(const nlohmann::json& j) {
    Assignment a;
    a._id = j.value("_id", "");
    a.track_id = j.value("track_id", "");
    a.effector_id = j.value("effector_id", "");
    a.mission_type = static_cast<MissionType>(j.value("mission_type", 5));

    std::string eff_type = j.value("effector_type", "interceptor");
    if (eff_type == "ew_jammer") a.effector_type = EffectorType::EW_JAMMER;
    else a.effector_type = EffectorType::INTERCEPTOR;

    std::string status_str = j.value("status", "assigned");
    if (status_str == "in_transit") a.status = AssignmentStatus::IN_TRANSIT;
    else if (status_str == "engaging") a.status = AssignmentStatus::ENGAGING;
    else if (status_str == "complete") a.status = AssignmentStatus::COMPLETE;
    else if (status_str == "aborted") a.status = AssignmentStatus::ABORTED;
    else a.status = AssignmentStatus::ASSIGNED;

    a.latitude = j.value("latitude", 0.0);
    a.longitude = j.value("longitude", 0.0);
    a.hae = j.value("hae", 0.0);
    a.created = j.value("created", static_cast<int64_t>(0));
    a.eta_seconds = j.value("eta_seconds", static_cast<int64_t>(0));
    a.progress = j.value("progress", 0);
    a.result = j.value("result", "");
    a.deleted = j.value("deleted", false);
    return a;
}

std::string Assignment::generate_id() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);

    std::stringstream ss;
    ss << std::hex;
    for (int i = 0; i < 8; ++i) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 4; ++i) ss << dis(gen);
    ss << "-4";
    for (int i = 0; i < 3; ++i) ss << dis(gen);
    ss << "-";
    ss << (dis(gen) & 0x3 | 0x8);
    for (int i = 0; i < 3; ++i) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 12; ++i) ss << dis(gen);
    return ss.str();
}

Assignment Assignment::create_interceptor_assignment(const std::string& track_id,
                                                     const std::string& effector_id) {
    Assignment a;
    a._id = generate_id();
    a.track_id = track_id;
    a.effector_id = effector_id;
    a.mission_type = MissionType::ENGAGE;
    a.effector_type = EffectorType::INTERCEPTOR;
    a.status = AssignmentStatus::ASSIGNED;
    a.created = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    a.eta_seconds = 60;  // Default 60 seconds ETA
    return a;
}

Assignment Assignment::create_ew_assignment(const std::string& track_id,
                                            const std::string& effector_id) {
    Assignment a;
    a._id = generate_id();
    a.track_id = track_id;
    a.effector_id = effector_id;
    a.mission_type = MissionType::ENGAGE;
    a.effector_type = EffectorType::EW_JAMMER;
    a.status = AssignmentStatus::ASSIGNED;
    a.created = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    a.eta_seconds = 5;  // EW activates quickly
    return a;
}

void to_json(nlohmann::json& j, const Assignment& a) {
    j = a.to_json();
}

void from_json(const nlohmann::json& j, Assignment& a) {
    a = Assignment::from_json(j);
}

} // namespace cuas
