#ifndef CUAS_MESH_C2_NODE_H
#define CUAS_MESH_C2_NODE_H

#include <memory>
#include <string>
#include <functional>
#include <vector>

#include "models/sensor.h"
#include "models/detection.h"
#include "models/track.h"
#include "models/alert.h"
#include "models/assignment.h"

namespace cuas {

class MeshPeer;
class Correlator;

/// C2 (Command and Control) node in the Counter-UAS mesh
class C2Node {
public:
    /// Constructor
    C2Node(std::shared_ptr<MeshPeer> peer, const std::string& node_id);

    ~C2Node();

    /// Start the C2 node
    void start();

    /// Stop the C2 node
    void stop();

    /// Get node ID
    const std::string& get_id() const { return node_id_; }

    /// Get all sensors (thread-safe copy)
    std::vector<Sensor> get_sensors() const;

    /// Get all tracks (thread-safe copy)
    std::vector<Track> get_tracks() const;

    /// Get all alerts (thread-safe copy)
    std::vector<Alert> get_alerts() const;

    /// Get pending alerts
    std::vector<Alert> get_pending_alerts() const;

    /// Get all assignments (thread-safe copy)
    std::vector<Assignment> get_assignments() const;

    /// Create alert for a track
    Alert create_alert(const std::string& track_id,
                       const std::string& message,
                       AlertCategory category = AlertCategory::CRITICAL);

    /// Respond to an alert
    void respond_to_alert(const std::string& alert_id, AlertResponse response);

    /// Assign interceptor to track
    Assignment assign_interceptor(const std::string& track_id,
                                   const std::string& effector_id);

    /// Assign EW jammer to track
    Assignment assign_ew(const std::string& track_id,
                          const std::string& effector_id);

    /// Get available effectors (from assignments collection)
    std::vector<std::string> get_available_effectors(EffectorType type) const;

    /// Register callbacks
    void on_sensor_update(std::function<void(const std::vector<Sensor>&)> callback);
    void on_track_update(std::function<void(const std::vector<Track>&)> callback);
    void on_alert_update(std::function<void(const std::vector<Alert>&)> callback);
    void on_assignment_update(std::function<void(const std::vector<Assignment>&)> callback);

private:
    std::shared_ptr<MeshPeer> peer_;
    std::string node_id_;
    std::unique_ptr<Correlator> correlator_;

    std::vector<Sensor> sensors_;
    std::vector<Detection> detections_;
    std::vector<Track> tracks_;
    std::vector<Alert> alerts_;
    std::vector<Assignment> assignments_;

    bool running_ = false;

    std::function<void(const std::vector<Sensor>&)> sensor_callback_;
    std::function<void(const std::vector<Track>&)> track_callback_;
    std::function<void(const std::vector<Alert>&)> alert_callback_;
    std::function<void(const std::vector<Assignment>&)> assignment_callback_;

    class Impl;
    std::unique_ptr<Impl> impl_;

    /// Process new detections and update tracks
    void process_detections(const std::vector<Detection>& detections);

    /// Check for HOSTILE tracks without alerts
    void check_for_threats();
};

} // namespace cuas

#endif // CUAS_MESH_C2_NODE_H
