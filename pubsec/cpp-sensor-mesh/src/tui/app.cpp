#include "app.h"

#include "ditto/mesh_peer.h"
#include "nodes/sensor_node.h"
#include "nodes/c2_node.h"
#include "nodes/effector_node.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <mutex>
#include <atomic>
#include <thread>

namespace cuas {

using namespace ftxui;

class App::Impl {
public:
    ScreenInteractive screen = ScreenInteractive::Fullscreen();
    std::mutex data_mutex;
    std::atomic<bool> needs_refresh{false};

    // Sensor view data
    std::vector<Detection> detections;
    Sensor sensor_info;
    bool simulating = false;

    // C2 view data
    std::vector<Sensor> sensors;
    std::vector<Track> tracks;
    std::vector<Alert> alerts;
    std::vector<Assignment> assignments;
    int selected_track = 0;
    int selected_alert = 0;

    // Effector view data
    const Assignment* current_assignment = nullptr;
    int progress = 0;
    int64_t eta = 0;
    EffectorType effector_type = EffectorType::INTERCEPTOR;

    void refresh() {
        needs_refresh = true;
        screen.PostEvent(Event::Custom);
    }
};

App::App(NodeMode mode, const std::string& node_id)
    : mode_(mode), node_id_(node_id), impl_(std::make_unique<Impl>()) {}

App::~App() {
    stop();
}

void App::init(std::shared_ptr<MeshPeer> peer) {
    peer_ = peer;

    switch (mode_) {
        case NodeMode::SENSOR:
            sensor_node_ = std::make_unique<SensorNode>(
                peer, node_id_, "radar", 38.9072, -77.0369);  // DC coords
            break;
        case NodeMode::C2:
            c2_node_ = std::make_unique<C2Node>(peer, node_id_);
            break;
        case NodeMode::EFFECTOR:
            effector_node_ = std::make_unique<EffectorNode>(
                peer, node_id_, EffectorType::INTERCEPTOR, 38.9072, -77.0369);
            break;
    }
}

void App::run() {
    running_ = true;

    switch (mode_) {
        case NodeMode::SENSOR:
            setup_sensor_view();
            break;
        case NodeMode::C2:
            setup_c2_view();
            break;
        case NodeMode::EFFECTOR:
            setup_effector_view();
            break;
    }
}

void App::stop() {
    running_ = false;
    impl_->screen.ExitLoopClosure()();

    if (sensor_node_) sensor_node_->stop();
    if (c2_node_) c2_node_->stop();
    if (effector_node_) effector_node_->stop();
}

void App::setup_sensor_view() {
    sensor_node_->start();
    impl_->sensor_info = sensor_node_->get_sensor();

    sensor_node_->on_detection_update([this](const std::vector<Detection>& dets) {
        std::lock_guard<std::mutex> lock(impl_->data_mutex);
        impl_->detections = dets;
        impl_->refresh();
    });

    auto sim_button = Button("Toggle Simulation", [this] {
        if (impl_->simulating) {
            sensor_node_->stop_simulation();
        } else {
            sensor_node_->start_simulation();
        }
        impl_->simulating = !impl_->simulating;
    });

    auto inject_button = Button("Inject Hostile", [this] {
        sensor_node_->inject_detection("Unknown sUAS", IdentityAffiliation::HOSTILE,
                                        45.0, 2000.0, 85);
    });

    auto quit_button = Button("Quit", [this] { stop(); });

    auto buttons = Container::Horizontal({sim_button, inject_button, quit_button});

    auto renderer = Renderer(buttons, [this, buttons] {
        std::lock_guard<std::mutex> lock(impl_->data_mutex);

        Elements det_rows;
        det_rows.push_back(hbox({
            text("ID") | size(WIDTH, EQUAL, 12),
            text("Type") | size(WIDTH, EQUAL, 15),
            text("Identity") | size(WIDTH, EQUAL, 10),
            text("Bearing") | size(WIDTH, EQUAL, 10),
            text("Range") | size(WIDTH, EQUAL, 10),
            text("Quality") | size(WIDTH, EQUAL, 8),
        }) | bold);

        for (const auto& det : impl_->detections) {
            Color id_color = Color::White;
            if (det.identity == IdentityAffiliation::HOSTILE) id_color = Color::Red;
            else if (det.identity == IdentityAffiliation::SUSPECT) id_color = Color::Yellow;

            det_rows.push_back(hbox({
                text(det._id.substr(0, 10)) | size(WIDTH, EQUAL, 12),
                text(det.platform_type) | size(WIDTH, EQUAL, 15),
                text(identityToString(det.identity)) | size(WIDTH, EQUAL, 10) | color(id_color),
                text(std::to_string(static_cast<int>(det.bearing)) + "°") | size(WIDTH, EQUAL, 10),
                text(std::to_string(static_cast<int>(det.range)) + "m") | size(WIDTH, EQUAL, 10),
                text(std::to_string(det.track_quality)) | size(WIDTH, EQUAL, 8),
            }));
        }

        return vbox({
            text("═══ SENSOR NODE: " + node_id_ + " ═══") | bold | center,
            separator(),
            hbox({
                text("Type: " + impl_->sensor_info.platform_type),
                text(" | "),
                text("Range: " + std::to_string(static_cast<int>(impl_->sensor_info.detection_range_m)) + "m"),
                text(" | "),
                text("Sim: " + std::string(impl_->simulating ? "ON" : "OFF")) |
                    color(impl_->simulating ? Color::Green : Color::GrayDark),
            }),
            separator(),
            text("Detections (" + std::to_string(impl_->detections.size()) + ")") | bold,
            vbox(det_rows) | frame | flex,
            separator(),
            buttons->Render() | center,
        }) | border;
    });

    impl_->screen.Loop(renderer);
}

void App::setup_c2_view() {
    c2_node_->start();

    c2_node_->on_sensor_update([this](const std::vector<Sensor>& s) {
        std::lock_guard<std::mutex> lock(impl_->data_mutex);
        impl_->sensors = s;
        impl_->refresh();
    });

    c2_node_->on_track_update([this](const std::vector<Track>& t) {
        std::lock_guard<std::mutex> lock(impl_->data_mutex);
        impl_->tracks = t;
        impl_->refresh();
    });

    c2_node_->on_alert_update([this](const std::vector<Alert>& a) {
        std::lock_guard<std::mutex> lock(impl_->data_mutex);
        impl_->alerts = a;
        impl_->refresh();
    });

    c2_node_->on_assignment_update([this](const std::vector<Assignment>& a) {
        std::lock_guard<std::mutex> lock(impl_->data_mutex);
        impl_->assignments = a;
        impl_->refresh();
    });

    auto engage_btn = Button("Engage Track", [this] {
        std::lock_guard<std::mutex> lock(impl_->data_mutex);
        if (impl_->selected_track < static_cast<int>(impl_->tracks.size())) {
            auto& track = impl_->tracks[impl_->selected_track];
            c2_node_->assign_interceptor(track._id, "EFFECTOR-1");
        }
    });

    auto ew_btn = Button("EW Jam", [this] {
        std::lock_guard<std::mutex> lock(impl_->data_mutex);
        if (impl_->selected_track < static_cast<int>(impl_->tracks.size())) {
            auto& track = impl_->tracks[impl_->selected_track];
            c2_node_->assign_ew(track._id, "EW-1");
        }
    });

    auto clear_btn = Button("Clear All", [this] {
        peer_->clear_all_data();
    });

    auto quit_btn = Button("Quit", [this] { stop(); });

    auto buttons = Container::Horizontal({engage_btn, ew_btn, clear_btn, quit_btn});

    auto renderer = Renderer(buttons, [this, buttons] {
        std::lock_guard<std::mutex> lock(impl_->data_mutex);

        // Tracks panel
        Elements track_rows;
        track_rows.push_back(hbox({
            text("#") | size(WIDTH, EQUAL, 6),
            text("Identity") | size(WIDTH, EQUAL, 10),
            text("Type") | size(WIDTH, EQUAL, 15),
            text("Threat") | size(WIDTH, EQUAL, 8),
            text("Dets") | size(WIDTH, EQUAL, 6),
        }) | bold);

        for (size_t i = 0; i < impl_->tracks.size(); ++i) {
            const auto& t = impl_->tracks[i];
            Color row_color = Color::White;
            if (t.identity == IdentityAffiliation::HOSTILE) row_color = Color::Red;

            auto row = hbox({
                text(t.track_number) | size(WIDTH, EQUAL, 6),
                text(identityToString(t.identity)) | size(WIDTH, EQUAL, 10),
                text(t.platform_type) | size(WIDTH, EQUAL, 15),
                text(std::to_string(t.track_quality)) | size(WIDTH, EQUAL, 8),
                text(std::to_string(t.detection_ids.size())) | size(WIDTH, EQUAL, 6),
            }) | color(row_color);

            if (static_cast<int>(i) == impl_->selected_track) {
                row = row | inverted;
            }
            track_rows.push_back(row);
        }

        // Alerts panel
        Elements alert_rows;
        for (const auto& a : impl_->alerts) {
            Color alert_color = Color::Yellow;
            if (a.category == AlertCategory::CRITICAL) alert_color = Color::Red;

            alert_rows.push_back(
                text("[" + alertCategoryToString(a.category) + "] " + a.message)
                | color(alert_color));
        }
        if (alert_rows.empty()) {
            alert_rows.push_back(text("No active alerts") | dim);
        }

        // Assignments panel
        Elements assign_rows;
        for (const auto& a : impl_->assignments) {
            assign_rows.push_back(hbox({
                text(a.effector_id) | size(WIDTH, EQUAL, 12),
                text(effectorTypeToString(a.effector_type)) | size(WIDTH, EQUAL, 12),
                text(assignmentStatusToString(a.status)) | size(WIDTH, EQUAL, 12),
            }));
        }
        if (assign_rows.empty()) {
            assign_rows.push_back(text("No active assignments") | dim);
        }

        return vbox({
            text("═══ C2 NODE: " + node_id_ + " ═══") | bold | center,
            separator(),
            hbox({
                text("Sensors: " + std::to_string(impl_->sensors.size())),
                text(" | Tracks: " + std::to_string(impl_->tracks.size())),
                text(" | Alerts: " + std::to_string(impl_->alerts.size())),
            }) | center,
            separator(),
            hbox({
                vbox({
                    text("TRACKS") | bold,
                    vbox(track_rows) | frame | flex,
                }) | flex,
                separator(),
                vbox({
                    text("ALERTS") | bold,
                    vbox(alert_rows) | frame,
                    separator(),
                    text("ASSIGNMENTS") | bold,
                    vbox(assign_rows) | frame,
                }) | size(WIDTH, EQUAL, 40),
            }) | flex,
            separator(),
            buttons->Render() | center,
        }) | border;
    });

    // Add keyboard navigation
    auto main_component = CatchEvent(renderer, [this](Event event) {
        if (event == Event::ArrowUp) {
            std::lock_guard<std::mutex> lock(impl_->data_mutex);
            if (impl_->selected_track > 0) impl_->selected_track--;
            return true;
        }
        if (event == Event::ArrowDown) {
            std::lock_guard<std::mutex> lock(impl_->data_mutex);
            if (impl_->selected_track < static_cast<int>(impl_->tracks.size()) - 1)
                impl_->selected_track++;
            return true;
        }
        return false;
    });

    impl_->screen.Loop(main_component);
}

void App::setup_effector_view() {
    effector_node_->start();
    impl_->effector_type = effector_node_->get_type();

    effector_node_->on_assignment_update([this](const Assignment* a) {
        std::lock_guard<std::mutex> lock(impl_->data_mutex);
        impl_->current_assignment = a;
        impl_->refresh();
    });

    // Progress update thread
    std::thread progress_thread([this] {
        while (running_) {
            {
                std::lock_guard<std::mutex> lock(impl_->data_mutex);
                impl_->progress = effector_node_->get_progress();
                impl_->eta = effector_node_->get_eta();
            }
            impl_->refresh();
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });

    auto confirm_btn = Button("Confirm Kill/Jam", [this] {
        if (impl_->effector_type == EffectorType::INTERCEPTOR) {
            effector_node_->confirm_kill();
        } else {
            effector_node_->confirm_jamming();
        }
    });

    auto abort_btn = Button("Abort", [this] {
        effector_node_->abort();
    });

    auto quit_btn = Button("Quit", [this] { stop(); });

    auto buttons = Container::Horizontal({confirm_btn, abort_btn, quit_btn});

    auto renderer = Renderer(buttons, [this, buttons] {
        std::lock_guard<std::mutex> lock(impl_->data_mutex);

        std::string status_text = "IDLE";
        Color status_color = Color::GrayDark;

        if (effector_node_->has_active_assignment()) {
            auto status = effector_node_->get_status();
            status_text = assignmentStatusToString(status);
            if (status == AssignmentStatus::ENGAGING) {
                status_color = Color::Yellow;
            } else if (status == AssignmentStatus::COMPLETE) {
                status_color = Color::Green;
            }
        }

        Element progress_bar = gauge(impl_->progress / 100.0) |
                               color(Color::Cyan) | flex;

        Elements content;
        content.push_back(text("═══ EFFECTOR NODE: " + node_id_ + " ═══") | bold | center);
        content.push_back(separator());
        content.push_back(hbox({
            text("Type: " + effector_node_->get_type_string()),
            text(" | "),
            text("Status: " + status_text) | color(status_color),
        }) | center);
        content.push_back(separator());

        if (effector_node_->has_active_assignment()) {
            content.push_back(text("ACTIVE ASSIGNMENT") | bold | center);
            content.push_back(separator());
            content.push_back(hbox({
                text("Progress: "),
                progress_bar,
                text(" " + std::to_string(impl_->progress) + "%"),
            }));
            content.push_back(text("ETA: " + std::to_string(impl_->eta) + "s") | center);
        } else {
            content.push_back(text("Awaiting assignment...") | dim | center);
        }

        content.push_back(filler());
        content.push_back(separator());
        content.push_back(buttons->Render() | center);

        return vbox(content) | border;
    });

    impl_->screen.Loop(renderer);
    progress_thread.join();
}

} // namespace cuas
