#ifndef CUAS_MESH_PEER_H
#define CUAS_MESH_PEER_H

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "models/sensor.h"
#include "models/detection.h"
#include "models/track.h"
#include "models/alert.h"
#include "models/assignment.h"

// Forward declaration for Ditto types
namespace ditto {
class StoreObserver;
}

namespace cuas {

/// Ditto peer for Counter-UAS Sensor Mesh
class MeshPeer {
public:
    /// Get Ditto SDK version
    static std::string get_ditto_sdk_version();

    /// Constructor
    MeshPeer(const std::string& app_id,
             const std::string& playground_token,
             const std::string& websocket_url,
             const std::string& auth_url,
             bool enable_cloud_sync,
             const std::string& persistence_dir);

    ~MeshPeer() noexcept;

    MeshPeer(const MeshPeer&) = delete;
    MeshPeer& operator=(const MeshPeer&) = delete;

    /// Start sync
    void start_sync();

    /// Stop sync
    void stop_sync();

    /// Check if sync is active
    bool is_sync_active() const;

    /// Get peer count (number of connected peers)
    int get_peer_count() const;

    // ==================== Sensor Operations ====================

    /// Add or update a sensor
    std::string upsert_sensor(const Sensor& sensor);

    /// Get all sensors
    std::vector<Sensor> get_sensors(bool include_deleted = false);

    /// Get sensor by ID
    Sensor get_sensor(const std::string& sensor_id);

    /// Update sensor heartbeat
    void update_sensor_heartbeat(const std::string& sensor_id);

    /// Register sensor observer
    std::shared_ptr<ditto::StoreObserver> register_sensor_observer(
        std::function<void(const std::vector<Sensor>&)> callback);

    // ==================== Detection Operations ====================

    /// Add a detection
    std::string add_detection(const Detection& detection);

    /// Get all detections
    std::vector<Detection> get_detections(bool include_deleted = false);

    /// Get detections by sensor
    std::vector<Detection> get_detections_by_sensor(const std::string& sensor_id);

    /// Delete a detection (soft delete)
    void delete_detection(const std::string& detection_id);

    /// Register detection observer
    std::shared_ptr<ditto::StoreObserver> register_detection_observer(
        std::function<void(const std::vector<Detection>&)> callback);

    // ==================== Track Operations ====================

    /// Add or update a track
    std::string upsert_track(const Track& track);

    /// Get all tracks
    std::vector<Track> get_tracks(bool include_deleted = false);

    /// Get track by ID
    Track get_track(const std::string& track_id);

    /// Update track status
    void update_track_status(const std::string& track_id, TrackStatus status);

    /// Add detection to track
    void add_detection_to_track(const std::string& track_id,
                                 const std::string& detection_id);

    /// Register track observer
    std::shared_ptr<ditto::StoreObserver> register_track_observer(
        std::function<void(const std::vector<Track>&)> callback);

    // ==================== Alert Operations ====================

    /// Add an alert
    std::string add_alert(const Alert& alert);

    /// Get all alerts
    std::vector<Alert> get_alerts(bool include_deleted = false);

    /// Get pending alerts
    std::vector<Alert> get_pending_alerts();

    /// Respond to alert
    void respond_to_alert(const std::string& alert_id, AlertResponse response);

    /// Close alert
    void close_alert(const std::string& alert_id);

    /// Register alert observer
    std::shared_ptr<ditto::StoreObserver> register_alert_observer(
        std::function<void(const std::vector<Alert>&)> callback);

    // ==================== Assignment Operations ====================

    /// Add an assignment
    std::string add_assignment(const Assignment& assignment);

    /// Get all assignments
    std::vector<Assignment> get_assignments(bool include_deleted = false);

    /// Get assignments for effector
    std::vector<Assignment> get_assignments_for_effector(const std::string& effector_id);

    /// Get active assignment for effector
    Assignment get_active_assignment(const std::string& effector_id);

    /// Update assignment status
    void update_assignment_status(const std::string& assignment_id,
                                   AssignmentStatus status);

    /// Update assignment progress (0-100)
    void update_assignment_progress(const std::string& assignment_id, int progress);

    /// Complete assignment with result
    void complete_assignment(const std::string& assignment_id,
                              const std::string& result);

    /// Register assignment observer
    std::shared_ptr<ditto::StoreObserver> register_assignment_observer(
        std::function<void(const std::vector<Assignment>&)> callback);

    // ==================== Utility ====================

    /// Execute raw DQL query
    std::string execute_dql(const std::string& query);

    /// Clear all data from all collections (reset database)
    void clear_all_data();

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace cuas

#endif // CUAS_MESH_PEER_H
