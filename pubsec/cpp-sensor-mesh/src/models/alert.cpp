#include "alert.h"
#include <chrono>
#include <random>
#include <sstream>

namespace cuas {

nlohmann::json Alert::to_json() const {
    std::vector<int> resp_ints;
    for (auto r : allowed_responses) {
        resp_ints.push_back(static_cast<int>(r));
    }

    return nlohmann::json{
        {"_id", _id},
        {"track_id", track_id},
        {"message", message},
        {"category", static_cast<int>(category)},
        {"alert_type", static_cast<int>(alert_type)},
        {"state", static_cast<int>(state)},
        {"allowed_responses", resp_ints},
        {"response", static_cast<int>(response)},
        {"created", created},
        {"timeout", timeout},
        {"deleted", deleted}
    };
}

Alert Alert::from_json(const nlohmann::json& j) {
    Alert a;
    a._id = j.value("_id", "");
    a.track_id = j.value("track_id", "");
    a.message = j.value("message", "");
    a.category = static_cast<AlertCategory>(j.value("category", 3));
    a.alert_type = static_cast<AlertType>(j.value("alert_type", 9));
    a.state = static_cast<AlertState>(j.value("state", 1));

    if (j.contains("allowed_responses") && j["allowed_responses"].is_array()) {
        for (auto& r : j["allowed_responses"]) {
            a.allowed_responses.push_back(static_cast<AlertResponse>(r.get<int>()));
        }
    }

    a.response = static_cast<AlertResponse>(j.value("response", 0));
    a.created = j.value("created", static_cast<int64_t>(0));
    a.timeout = j.value("timeout", static_cast<int64_t>(60000));
    a.deleted = j.value("deleted", false);
    return a;
}

std::string Alert::generate_id() {
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

Alert Alert::create_threat_alert(const std::string& track_id,
                                 const std::string& message,
                                 AlertCategory category) {
    Alert a;
    a._id = generate_id();
    a.track_id = track_id;
    a.message = message;
    a.category = category;
    a.alert_type = AlertType::THREAT;
    a.state = AlertState::AWAITING_RESPONSE;
    a.allowed_responses = {
        AlertResponse::ACKNOWLEDGE,
        AlertResponse::ACTIVATE,
        AlertResponse::REJECT
    };
    a.created = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    return a;
}

void to_json(nlohmann::json& j, const Alert& a) {
    j = a.to_json();
}

void from_json(const nlohmann::json& j, Alert& a) {
    a = Alert::from_json(j);
}

} // namespace cuas
