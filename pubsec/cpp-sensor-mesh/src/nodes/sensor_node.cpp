#include "sensor_node.h"
#include "ditto/mesh_peer.h"
#include "simulation/drone_sim.h"

#include <chrono>
#include <thread>
#include <atomic>
#include <cmath>

namespace cuas {

class SensorNode::Impl {
public:
    std::shared_ptr<ditto::StoreObserver> detection_observer;
    std::unique_ptr<DroneSim> drone_sim;
    std::thread sim_thread;
    std::atomic<bool> sim_running{false};
};

SensorNode::SensorNode(std::shared_ptr<MeshPeer> peer,
                       const std::string& sensor_id,
                       const std::string& sensor_type,
                       double lat, double lon)
    : peer_(peer), impl_(std::make_unique<Impl>()) {

    // Initialize sensor
    sensor_._id = sensor_id;
    sensor_.name = sensor_id;
    sensor_.platform_type = sensor_type;
    sensor_.latitude = lat;
    sensor_.longitude = lon;
    sensor_.hae = 0.0;
    sensor_.status = SensorStatus::COVERING;
    sensor_.active = true;
    sensor_.last_heartbeat = Detection::now();

    // Set detection range based on sensor type
    if (sensor_type == "radar") {
        sensor_.detection_range_m = 10000.0;  // 10km
        sensor_.bearing_accuracy_deg = 1.0;
    } else if (sensor_type == "acoustic") {
        sensor_.detection_range_m = 3000.0;   // 3km
        sensor_.bearing_accuracy_deg = 5.0;
    } else if (sensor_type == "rf") {
        sensor_.detection_range_m = 5000.0;   // 5km
        sensor_.bearing_accuracy_deg = 3.0;
    } else {  // eo
        sensor_.detection_range_m = 2000.0;   // 2km
        sensor_.bearing_accuracy_deg = 0.5;
    }
}

SensorNode::~SensorNode() {
    stop();
}

void SensorNode::start() {
    if (running_) return;
    running_ = true;

    // Register sensor with mesh
    peer_->upsert_sensor(sensor_);

    // Register detection observer
    impl_->detection_observer = peer_->register_detection_observer(
        [this](const std::vector<Detection>& dets) {
            // Filter to our sensor's detections
            detections_.clear();
            for (const auto& d : dets) {
                if (d.sensor_id == sensor_._id) {
                    detections_.push_back(d);
                }
            }
            if (detection_callback_) {
                detection_callback_(detections_);
            }
        });
}

void SensorNode::stop() {
    if (!running_) return;

    stop_simulation();

    if (impl_->detection_observer) {
        impl_->detection_observer.reset();
    }

    sensor_.active = false;
    peer_->upsert_sensor(sensor_);

    running_ = false;
}

Detection SensorNode::inject_detection(const std::string& platform_type,
                                        IdentityAffiliation identity,
                                        double bearing, double range,
                                        int quality) {
    Detection det;
    det._id = Detection::generate_id();
    det.sensor_id = sensor_._id;
    det.environment = EnvironmentCategory::AIR;
    det.identity = identity;
    det.platform_type = platform_type;

    // Calculate position from bearing and range
    double bearing_rad = bearing * M_PI / 180.0;
    double lat_delta = (range / 111320.0) * std::cos(bearing_rad);
    double lon_delta = (range / (111320.0 * std::cos(sensor_.latitude * M_PI / 180.0))) * std::sin(bearing_rad);

    det.latitude = sensor_.latitude + lat_delta;
    det.longitude = sensor_.longitude + lon_delta;
    det.hae = 100.0;  // Default altitude 100m
    det.circular_error = range * 0.05;  // 5% error
    det.linear_error = 20.0;

    det.bearing = bearing;
    det.range = range;
    det.bearing_accuracy = sensor_.bearing_accuracy_deg;
    det.track_quality = quality;
    det.strength = quality;
    det.simulated = false;
    det.created = Detection::now();
    det.updated = det.created;

    peer_->add_detection(det);
    return det;
}

void SensorNode::start_simulation() {
    if (simulating_) return;
    simulating_ = true;
    impl_->sim_running = true;

    // Create drone simulator
    impl_->drone_sim = std::make_unique<DroneSim>(
        sensor_.latitude, sensor_.longitude, sensor_.detection_range_m);

    // Start simulation thread
    impl_->sim_thread = std::thread([this]() {
        while (impl_->sim_running) {
            // Generate simulated detections
            auto sim_dets = impl_->drone_sim->generate_detections(sensor_._id);
            for (auto& det : sim_dets) {
                det.simulated = true;
                det.bearing_accuracy = sensor_.bearing_accuracy_deg;
                peer_->add_detection(det);
            }

            // Sleep for 2-5 seconds between detections
            std::this_thread::sleep_for(std::chrono::milliseconds(2000 + rand() % 3000));
        }
    });
}

void SensorNode::stop_simulation() {
    if (!simulating_) return;
    simulating_ = false;
    impl_->sim_running = false;

    if (impl_->sim_thread.joinable()) {
        impl_->sim_thread.join();
    }

    impl_->drone_sim.reset();
}

void SensorNode::on_detection_update(std::function<void(const std::vector<Detection>&)> callback) {
    detection_callback_ = callback;
}

void SensorNode::heartbeat() {
    sensor_.last_heartbeat = Detection::now();
    peer_->update_sensor_heartbeat(sensor_._id);
}

} // namespace cuas
