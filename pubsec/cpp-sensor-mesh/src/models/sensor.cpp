#include "sensor.h"
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>

namespace cuas {

nlohmann::json Sensor::to_json() const {
    return nlohmann::json{
        {"_id", _id},
        {"name", name},
        {"environment", static_cast<int>(environment)},
        {"platform_type", platform_type},
        {"latitude", latitude},
        {"longitude", longitude},
        {"hae", hae},
        {"circular_error", circular_error},
        {"status", static_cast<int>(status)},
        {"detection_range_m", detection_range_m},
        {"bearing_accuracy_deg", bearing_accuracy_deg},
        {"active", active},
        {"last_heartbeat", last_heartbeat},
        {"deleted", deleted}
    };
}

Sensor Sensor::from_json(const nlohmann::json& j) {
    Sensor s;
    s._id = j.value("_id", "");
    s.name = j.value("name", "");
    s.environment = static_cast<EnvironmentCategory>(j.value("environment", 4));
    s.platform_type = j.value("platform_type", "");
    s.latitude = j.value("latitude", 0.0);
    s.longitude = j.value("longitude", 0.0);
    s.hae = j.value("hae", 0.0);
    s.circular_error = j.value("circular_error", 0.0);
    s.status = static_cast<SensorStatus>(j.value("status", 9));
    s.detection_range_m = j.value("detection_range_m", 5000.0);
    s.bearing_accuracy_deg = j.value("bearing_accuracy_deg", 2.0);
    s.active = j.value("active", true);
    s.last_heartbeat = j.value("last_heartbeat", static_cast<int64_t>(0));
    s.deleted = j.value("deleted", false);
    return s;
}

std::string Sensor::generate_id() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);

    std::stringstream ss;
    ss << std::hex;
    for (int i = 0; i < 8; ++i) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 4; ++i) ss << dis(gen);
    ss << "-4";  // Version 4 UUID
    for (int i = 0; i < 3; ++i) ss << dis(gen);
    ss << "-";
    ss << (dis(gen) & 0x3 | 0x8);  // Variant
    for (int i = 0; i < 3; ++i) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 12; ++i) ss << dis(gen);
    return ss.str();
}

void to_json(nlohmann::json& j, const Sensor& s) {
    j = s.to_json();
}

void from_json(const nlohmann::json& j, Sensor& s) {
    s = Sensor::from_json(j);
}

} // namespace cuas
