#include "c2_node.h"
#include "ditto/mesh_peer.h"
#include "correlation/correlator.h"

#include <algorithm>
#include <mutex>

namespace cuas {

class C2Node::Impl {
public:
    std::shared_ptr<ditto::StoreObserver> sensor_observer;
    std::shared_ptr<ditto::StoreObserver> detection_observer;
    std::shared_ptr<ditto::StoreObserver> track_observer;
    std::shared_ptr<ditto::StoreObserver> alert_observer;
    std::shared_ptr<ditto::StoreObserver> assignment_observer;
    std::recursive_mutex mtx;  // Protect shared state from concurrent observer callbacks
};

C2Node::C2Node(std::shared_ptr<MeshPeer> peer, const std::string& node_id)
    : peer_(peer), node_id_(node_id),
      correlator_(std::make_unique<Correlator>()),
      impl_(std::make_unique<Impl>()) {}

C2Node::~C2Node() {
    stop();
}

void C2Node::start() {
    if (running_) return;
    running_ = true;

    // Register sensor observer
    impl_->sensor_observer = peer_->register_sensor_observer(
        [this](const std::vector<Sensor>& sensors) {
            std::lock_guard<std::recursive_mutex> lock(impl_->mtx);
            sensors_ = sensors;
            if (sensor_callback_) {
                sensor_callback_(sensors_);
            }
        });

    // Register detection observer
    impl_->detection_observer = peer_->register_detection_observer(
        [this](const std::vector<Detection>& detections) {
            std::lock_guard<std::recursive_mutex> lock(impl_->mtx);
            detections_ = detections;
            process_detections(detections);
        });

    // Register track observer
    impl_->track_observer = peer_->register_track_observer(
        [this](const std::vector<Track>& tracks) {
            std::lock_guard<std::recursive_mutex> lock(impl_->mtx);
            tracks_ = tracks;
            if (track_callback_) {
                track_callback_(tracks_);
            }
            check_for_threats();
        });

    // Register alert observer
    impl_->alert_observer = peer_->register_alert_observer(
        [this](const std::vector<Alert>& alerts) {
            std::lock_guard<std::recursive_mutex> lock(impl_->mtx);
            alerts_ = alerts;
            if (alert_callback_) {
                alert_callback_(alerts_);
            }
        });

    // Register assignment observer
    impl_->assignment_observer = peer_->register_assignment_observer(
        [this](const std::vector<Assignment>& assignments) {
            std::lock_guard<std::recursive_mutex> lock(impl_->mtx);
            assignments_ = assignments;
            if (assignment_callback_) {
                assignment_callback_(assignments_);
            }
        });
}

void C2Node::stop() {
    if (!running_) return;

    impl_->sensor_observer.reset();
    impl_->detection_observer.reset();
    impl_->track_observer.reset();
    impl_->alert_observer.reset();
    impl_->assignment_observer.reset();

    running_ = false;
}

std::vector<Sensor> C2Node::get_sensors() const {
    std::lock_guard<std::recursive_mutex> lock(impl_->mtx);
    return sensors_;
}

std::vector<Track> C2Node::get_tracks() const {
    std::lock_guard<std::recursive_mutex> lock(impl_->mtx);
    return tracks_;
}

std::vector<Alert> C2Node::get_alerts() const {
    std::lock_guard<std::recursive_mutex> lock(impl_->mtx);
    return alerts_;
}

std::vector<Assignment> C2Node::get_assignments() const {
    std::lock_guard<std::recursive_mutex> lock(impl_->mtx);
    return assignments_;
}

void C2Node::process_detections(const std::vector<Detection>& detections) {
    // Use correlator to fuse detections into tracks
    auto updated_tracks = correlator_->correlate(detections, tracks_);

    // Upsert updated tracks
    for (const auto& track : updated_tracks) {
        peer_->upsert_track(track);
    }
}

void C2Node::check_for_threats() {
    // Check for HOSTILE tracks without active alerts
    for (const auto& track : tracks_) {
        if (track.identity == IdentityAffiliation::HOSTILE &&
            track.status == TrackStatus::ACTIVE) {

            // Check if there's already an active alert for this track
            bool has_alert = false;
            for (const auto& alert : alerts_) {
                if (alert.track_id == track._id &&
                    alert.state != AlertState::CLOSED) {
                    has_alert = true;
                    break;
                }
            }

            if (!has_alert) {
                // Auto-create CAT_1 alert for HOSTILE track
                std::string msg = "HOSTILE " + track.platform_type +
                                  " detected - " + track.track_number;
                create_alert(track._id, msg, AlertCategory::CRITICAL);
            }
        }
    }
}

std::vector<Alert> C2Node::get_pending_alerts() const {
    std::lock_guard<std::recursive_mutex> lock(impl_->mtx);
    std::vector<Alert> pending;
    for (const auto& alert : alerts_) {
        if (alert.state == AlertState::AWAITING_RESPONSE) {
            pending.push_back(alert);
        }
    }
    return pending;
}

Alert C2Node::create_alert(const std::string& track_id,
                           const std::string& message,
                           AlertCategory category) {
    Alert alert = Alert::create_threat_alert(track_id, message, category);
    peer_->add_alert(alert);
    return alert;
}

void C2Node::respond_to_alert(const std::string& alert_id, AlertResponse response) {
    peer_->respond_to_alert(alert_id, response);
}

Assignment C2Node::assign_interceptor(const std::string& track_id,
                                       const std::string& effector_id) {
    Assignment assignment = Assignment::create_interceptor_assignment(track_id, effector_id);
    peer_->add_assignment(assignment);
    return assignment;
}

Assignment C2Node::assign_ew(const std::string& track_id,
                              const std::string& effector_id) {
    Assignment assignment = Assignment::create_ew_assignment(track_id, effector_id);
    peer_->add_assignment(assignment);
    return assignment;
}

std::vector<std::string> C2Node::get_available_effectors(EffectorType type) const {
    std::vector<std::string> available;
    // This would normally query effector registrations
    // For now, return empty - effectors register themselves
    return available;
}

void C2Node::on_sensor_update(std::function<void(const std::vector<Sensor>&)> callback) {
    sensor_callback_ = callback;
}

void C2Node::on_track_update(std::function<void(const std::vector<Track>&)> callback) {
    track_callback_ = callback;
}

void C2Node::on_alert_update(std::function<void(const std::vector<Alert>&)> callback) {
    alert_callback_ = callback;
}

void C2Node::on_assignment_update(std::function<void(const std::vector<Assignment>&)> callback) {
    assignment_callback_ = callback;
}

} // namespace cuas
