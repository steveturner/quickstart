#include "track.h"
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>
#include <atomic>

namespace cuas {

static std::atomic<int> track_counter{1};

nlohmann::json Track::to_json() const {
    return nlohmann::json{
        {"_id", _id},
        {"track_number", track_number},
        {"latitude", latitude},
        {"longitude", longitude},
        {"hae", hae},
        {"course", course},
        {"speed", speed},
        {"identity", static_cast<int>(identity)},
        {"platform_type", platform_type},
        {"track_quality", track_quality},
        {"detection_ids", detection_ids},
        {"relation_type", static_cast<int>(relation_type)},
        {"status", trackStatusToString(status)},
        {"last_update", last_update},
        {"deleted", deleted}
    };
}

Track Track::from_json(const nlohmann::json& j) {
    Track t;
    t._id = j.value("_id", "");
    t.track_number = j.value("track_number", "");
    t.latitude = j.value("latitude", 0.0);
    t.longitude = j.value("longitude", 0.0);
    t.hae = j.value("hae", 0.0);
    t.course = j.value("course", 0.0);
    t.speed = j.value("speed", 0.0);
    t.identity = static_cast<IdentityAffiliation>(j.value("identity", 0));
    t.platform_type = j.value("platform_type", "");
    t.track_quality = j.value("track_quality", 0);

    if (j.contains("detection_ids") && j["detection_ids"].is_array()) {
        t.detection_ids = j["detection_ids"].get<std::vector<std::string>>();
    }

    t.relation_type = static_cast<RelationType>(j.value("relation_type", 8));

    std::string status_str = j.value("status", "active");
    if (status_str == "lost") t.status = TrackStatus::LOST;
    else if (status_str == "killed") t.status = TrackStatus::KILLED;
    else t.status = TrackStatus::ACTIVE;

    t.last_update = j.value("last_update", static_cast<int64_t>(0));
    t.deleted = j.value("deleted", false);
    return t;
}

std::string Track::generate_id() {
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

std::string Track::generate_track_number() {
    std::stringstream ss;
    ss << "TN-" << std::setfill('0') << std::setw(4) << track_counter++;
    return ss.str();
}

void to_json(nlohmann::json& j, const Track& t) {
    j = t.to_json();
}

void from_json(const nlohmann::json& j, Track& t) {
    t = Track::from_json(j);
}

} // namespace cuas
