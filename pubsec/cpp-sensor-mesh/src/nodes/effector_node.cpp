#include "effector_node.h"
#include "ditto/mesh_peer.h"

#include <chrono>
#include <thread>
#include <atomic>

namespace cuas {

class EffectorNode::Impl {
public:
    std::shared_ptr<ditto::StoreObserver> assignment_observer;
    std::thread progress_thread;
    std::atomic<bool> progress_running{false};
};

EffectorNode::EffectorNode(std::shared_ptr<MeshPeer> peer,
                           const std::string& effector_id,
                           EffectorType type,
                           double lat, double lon)
    : peer_(peer), effector_id_(effector_id), type_(type),
      latitude_(lat), longitude_(lon),
      impl_(std::make_unique<Impl>()) {}

EffectorNode::~EffectorNode() {
    stop();
}

void EffectorNode::start() {
    if (running_) return;
    running_ = true;

    // Register assignment observer - filter to our effector
    impl_->assignment_observer = peer_->register_assignment_observer(
        [this](const std::vector<Assignment>& assignments) {
            for (const auto& assignment : assignments) {
                if (assignment.effector_id == effector_id_ &&
                    assignment.status == AssignmentStatus::ASSIGNED) {
                    process_assignment(assignment);
                }
            }
        });
}

void EffectorNode::stop() {
    if (!running_) return;

    // Stop progress thread
    impl_->progress_running = false;
    if (impl_->progress_thread.joinable()) {
        impl_->progress_thread.join();
    }

    if (impl_->assignment_observer) {
        impl_->assignment_observer.reset();
    }

    running_ = false;
}

bool EffectorNode::has_active_assignment() const {
    return has_assignment_ &&
           current_assignment_.status != AssignmentStatus::COMPLETE &&
           current_assignment_.status != AssignmentStatus::ABORTED;
}

const Assignment* EffectorNode::get_current_assignment() const {
    if (has_assignment_) {
        return &current_assignment_;
    }
    return nullptr;
}

AssignmentStatus EffectorNode::get_status() const {
    if (!has_assignment_) {
        return AssignmentStatus::ASSIGNED;
    }
    return current_assignment_.status;
}

int64_t EffectorNode::get_eta() const {
    if (!has_assignment_ || current_assignment_.eta_seconds <= 0) {
        return 0;
    }

    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    int64_t elapsed_ms = now - assignment_start_time_;
    int64_t elapsed_sec = elapsed_ms / 1000;
    int64_t remaining = current_assignment_.eta_seconds - elapsed_sec;

    return remaining > 0 ? remaining : 0;
}

int EffectorNode::get_progress() const {
    if (!has_assignment_ || current_assignment_.eta_seconds <= 0) {
        return 0;
    }

    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    int64_t elapsed_ms = now - assignment_start_time_;
    int64_t total_ms = current_assignment_.eta_seconds * 1000;

    if (elapsed_ms >= total_ms) {
        return 100;
    }

    return static_cast<int>((elapsed_ms * 100) / total_ms);
}

void EffectorNode::confirm_kill() {
    if (!has_assignment_ || type_ != EffectorType::INTERCEPTOR) {
        return;
    }

    // Stop progress thread
    impl_->progress_running = false;

    current_assignment_.status = AssignmentStatus::COMPLETE;
    current_assignment_.progress = 100;
    peer_->update_assignment_status(current_assignment_._id, AssignmentStatus::COMPLETE);
    peer_->update_assignment_progress(current_assignment_._id, 100);

    if (assignment_callback_) {
        assignment_callback_(&current_assignment_);
    }
}

void EffectorNode::confirm_jamming() {
    if (!has_assignment_ || type_ != EffectorType::EW_JAMMER) {
        return;
    }

    // Stop progress thread
    impl_->progress_running = false;

    current_assignment_.status = AssignmentStatus::COMPLETE;
    current_assignment_.progress = 100;
    peer_->update_assignment_status(current_assignment_._id, AssignmentStatus::COMPLETE);
    peer_->update_assignment_progress(current_assignment_._id, 100);

    if (assignment_callback_) {
        assignment_callback_(&current_assignment_);
    }
}

void EffectorNode::abort() {
    if (!has_assignment_) {
        return;
    }

    // Stop progress thread
    impl_->progress_running = false;

    current_assignment_.status = AssignmentStatus::ABORTED;
    peer_->update_assignment_status(current_assignment_._id, AssignmentStatus::ABORTED);

    if (assignment_callback_) {
        assignment_callback_(&current_assignment_);
    }
}

void EffectorNode::on_assignment_update(std::function<void(const Assignment*)> callback) {
    assignment_callback_ = callback;
}

void EffectorNode::process_assignment(const Assignment& assignment) {
    // Accept the assignment
    current_assignment_ = assignment;
    current_assignment_.status = AssignmentStatus::ENGAGING;
    current_assignment_.progress = 0;
    has_assignment_ = true;

    assignment_start_time_ = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    // Update status in mesh
    peer_->update_assignment_status(current_assignment_._id, AssignmentStatus::ENGAGING);

    // Stop any existing progress thread
    impl_->progress_running = false;
    if (impl_->progress_thread.joinable()) {
        impl_->progress_thread.join();
    }

    // Start progress update thread with shared state to avoid use-after-free
    impl_->progress_running = true;
    auto progress_running = &impl_->progress_running;
    auto peer = peer_;  // Copy shared_ptr
    auto assignment_id = current_assignment_._id;
    auto eta_seconds = current_assignment_.eta_seconds;
    auto start_time = assignment_start_time_;

    impl_->progress_thread = std::thread([progress_running, peer, assignment_id, eta_seconds, start_time]() {
        int last_progress = -1;
        while (progress_running->load()) {
            auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            int64_t elapsed_ms = now - start_time;
            int64_t total_ms = eta_seconds * 1000;
            int progress = (total_ms > 0) ? static_cast<int>((elapsed_ms * 100) / total_ms) : 100;
            if (progress > 100) progress = 100;

            // Only update if progress changed (avoid flooding Ditto)
            if (progress != last_progress) {
                try {
                    peer->update_assignment_progress(assignment_id, progress);
                } catch (...) {
                    // Ignore errors during shutdown
                }
                last_progress = progress;
            }

            if (progress >= 100) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });

    if (assignment_callback_) {
        assignment_callback_(&current_assignment_);
    }
}

void EffectorNode::update_progress() {
    // Progress is calculated dynamically in get_progress()
    // This method can be used for periodic UI updates if needed
}

} // namespace cuas
