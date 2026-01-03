#ifndef CUAS_MESH_SENSOR_NODE_H
#define CUAS_MESH_SENSOR_NODE_H

#include <memory>
#include <string>
#include <functional>
#include <vector>

#include "models/sensor.h"
#include "models/detection.h"

namespace cuas {

class MeshPeer;

/// Sensor node in the Counter-UAS mesh
class SensorNode {
public:
    /// Constructor
    SensorNode(std::shared_ptr<MeshPeer> peer,
               const std::string& sensor_id,
               const std::string& sensor_type,
               double lat, double lon);

    ~SensorNode();

    /// Start the sensor node
    void start();

    /// Stop the sensor node
    void stop();

    /// Get sensor info
    const Sensor& get_sensor() const { return sensor_; }

    /// Get local detections
    const std::vector<Detection>& get_detections() const { return detections_; }

    /// Manually inject a detection
    Detection inject_detection(const std::string& platform_type,
                                IdentityAffiliation identity,
                                double bearing, double range,
                                int quality);

    /// Start simulated drone detections
    void start_simulation();

    /// Stop simulated drone detections
    void stop_simulation();

    /// Check if simulation is running
    bool is_simulating() const { return simulating_; }

    /// Register callback for detection updates
    void on_detection_update(std::function<void(const std::vector<Detection>&)> callback);

    /// Update heartbeat
    void heartbeat();

private:
    std::shared_ptr<MeshPeer> peer_;
    Sensor sensor_;
    std::vector<Detection> detections_;
    bool running_ = false;
    bool simulating_ = false;
    std::function<void(const std::vector<Detection>&)> detection_callback_;

    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace cuas

#endif // CUAS_MESH_SENSOR_NODE_H
