#include "detection.h"
#include <chrono>
#include <random>
#include <sstream>

namespace cuas {

nlohmann::json Detection::to_json() const {
    return nlohmann::json{
        {"_id", _id},
        {"sensor_id", sensor_id},
        {"environment", static_cast<int>(environment)},
        {"identity", static_cast<int>(identity)},
        {"platform_type", platform_type},
        {"latitude", latitude},
        {"longitude", longitude},
        {"hae", hae},
        {"circular_error", circular_error},
        {"linear_error", linear_error},
        {"course", course},
        {"speed", speed},
        {"track_quality", track_quality},
        {"strength", strength},
        {"simulated", simulated},
        {"special_interest", special_interest},
        {"bearing", bearing},
        {"range", range},
        {"bearing_accuracy", bearing_accuracy},
        {"created", created},
        {"updated", updated},
        {"timeout", timeout},
        {"deleted", deleted}
    };
}

Detection Detection::from_json(const nlohmann::json& j) {
    Detection d;
    d._id = j.value("_id", "");
    d.sensor_id = j.value("sensor_id", "");
    d.environment = static_cast<EnvironmentCategory>(j.value("environment", 1));
    d.identity = static_cast<IdentityAffiliation>(j.value("identity", 0));
    d.platform_type = j.value("platform_type", "");
    d.latitude = j.value("latitude", 0.0);
    d.longitude = j.value("longitude", 0.0);
    d.hae = j.value("hae", 0.0);
    d.circular_error = j.value("circular_error", 0.0);
    d.linear_error = j.value("linear_error", 0.0);
    d.course = j.value("course", 0.0);
    d.speed = j.value("speed", 0.0);
    d.track_quality = j.value("track_quality", 0);
    d.strength = j.value("strength", 0);
    d.simulated = j.value("simulated", false);
    d.special_interest = j.value("special_interest", false);
    d.bearing = j.value("bearing", 0.0);
    d.range = j.value("range", 0.0);
    d.bearing_accuracy = j.value("bearing_accuracy", 0.0);
    d.created = j.value("created", static_cast<int64_t>(0));
    d.updated = j.value("updated", static_cast<int64_t>(0));
    d.timeout = j.value("timeout", static_cast<int64_t>(30000));
    d.deleted = j.value("deleted", false);
    return d;
}

std::string Detection::generate_id() {
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

int64_t Detection::now() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

void to_json(nlohmann::json& j, const Detection& d) {
    j = d.to_json();
}

void from_json(const nlohmann::json& j, Detection& d) {
    d = Detection::from_json(j);
}

} // namespace cuas
