#ifndef CUAS_MESH_DRONE_SIM_H
#define CUAS_MESH_DRONE_SIM_H

#include <vector>
#include <string>
#include <memory>

#include "models/detection.h"

namespace cuas {

/// Simulated drone for generating detections
struct SimulatedDrone {
    std::string id;
    std::string platform_type;
    IdentityAffiliation identity;
    double latitude;
    double longitude;
    double altitude;
    double heading;     // degrees
    double speed;       // m/s
    bool active;
};

/// Drone simulator for generating realistic detection patterns
class DroneSim {
public:
    /// Constructor
    /// @param center_lat Sensor center latitude
    /// @param center_lon Sensor center longitude
    /// @param range_m Detection range in meters
    DroneSim(double center_lat, double center_lon, double range_m);

    ~DroneSim();

    /// Generate detections for current simulation state
    /// @param sensor_id The sensor generating detections
    /// @return Vector of detections
    std::vector<Detection> generate_detections(const std::string& sensor_id);

    /// Add a simulated drone
    void add_drone(const SimulatedDrone& drone);

    /// Remove a simulated drone
    void remove_drone(const std::string& drone_id);

    /// Get all active drones
    std::vector<SimulatedDrone> get_drones() const;

    /// Update drone positions (call periodically)
    void update(double delta_seconds);

    /// Spawn random hostile drones
    void spawn_random_hostiles(int count = 1);

    /// Clear all drones
    void clear();

private:
    double center_lat_;
    double center_lon_;
    double range_m_;

    std::vector<SimulatedDrone> drones_;
    int drone_counter_ = 0;

    /// Calculate bearing and range from sensor to drone
    void calculate_bearing_range(const SimulatedDrone& drone,
                                 double& bearing, double& range) const;

    /// Add position noise to simulate sensor error
    void add_noise(Detection& det) const;
};

} // namespace cuas

#endif // CUAS_MESH_DRONE_SIM_H
