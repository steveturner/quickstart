#include "drone_sim.h"

#include <cmath>
#include <random>
#include <chrono>

namespace cuas {

namespace {
    std::mt19937& get_rng() {
        static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
        return rng;
    }

    double random_double(double min, double max) {
        std::uniform_real_distribution<double> dist(min, max);
        return dist(get_rng());
    }

    int random_int(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(get_rng());
    }

    const std::vector<std::string> DRONE_TYPES = {
        "DJI Mavic", "DJI Phantom", "Parrot Anafi", "Autel Evo",
        "Custom Hex", "Fixed Wing", "Unknown sUAS"
    };
}

DroneSim::DroneSim(double center_lat, double center_lon, double range_m)
    : center_lat_(center_lat), center_lon_(center_lon), range_m_(range_m) {}

DroneSim::~DroneSim() = default;

std::vector<Detection> DroneSim::generate_detections(const std::string& sensor_id) {
    std::vector<Detection> detections;

    for (const auto& drone : drones_) {
        if (!drone.active) continue;

        double bearing, range;
        calculate_bearing_range(drone, bearing, range);

        // Only detect if within range
        if (range > range_m_) continue;

        Detection det;
        det._id = Detection::generate_id();
        det.sensor_id = sensor_id;
        det.environment = EnvironmentCategory::AIR;
        det.identity = drone.identity;
        det.platform_type = drone.platform_type;
        det.latitude = drone.latitude;
        det.longitude = drone.longitude;
        det.hae = drone.altitude;
        det.bearing = bearing;
        det.range = range;
        det.track_quality = random_int(60, 95);
        det.strength = det.track_quality;
        det.simulated = true;
        det.created = Detection::now();
        det.updated = det.created;

        // Add noise to simulate real sensor
        add_noise(det);

        detections.push_back(det);
    }

    return detections;
}

void DroneSim::add_drone(const SimulatedDrone& drone) {
    drones_.push_back(drone);
}

void DroneSim::remove_drone(const std::string& drone_id) {
    drones_.erase(
        std::remove_if(drones_.begin(), drones_.end(),
            [&](const SimulatedDrone& d) { return d.id == drone_id; }),
        drones_.end());
}

std::vector<SimulatedDrone> DroneSim::get_drones() const {
    return drones_;
}

void DroneSim::update(double delta_seconds) {
    const double DEG_TO_RAD = M_PI / 180.0;
    const double METERS_PER_DEG_LAT = 111320.0;

    for (auto& drone : drones_) {
        if (!drone.active) continue;

        // Move drone based on heading and speed
        double distance = drone.speed * delta_seconds;
        double heading_rad = drone.heading * DEG_TO_RAD;

        double lat_delta = (distance / METERS_PER_DEG_LAT) * std::cos(heading_rad);
        double lon_delta = (distance / (METERS_PER_DEG_LAT * std::cos(drone.latitude * DEG_TO_RAD))) * std::sin(heading_rad);

        drone.latitude += lat_delta;
        drone.longitude += lon_delta;

        // Random heading changes (simulate realistic flight)
        if (random_int(0, 10) == 0) {
            drone.heading += random_double(-15.0, 15.0);
            if (drone.heading < 0) drone.heading += 360.0;
            if (drone.heading >= 360) drone.heading -= 360.0;
        }

        // Random altitude changes
        if (random_int(0, 20) == 0) {
            drone.altitude += random_double(-10.0, 10.0);
            drone.altitude = std::max(20.0, std::min(400.0, drone.altitude));
        }
    }
}

void DroneSim::spawn_random_hostiles(int count) {
    const double DEG_TO_RAD = M_PI / 180.0;
    const double METERS_PER_DEG_LAT = 111320.0;

    for (int i = 0; i < count; ++i) {
        SimulatedDrone drone;
        drone.id = "SIM-" + std::to_string(++drone_counter_);
        drone.platform_type = DRONE_TYPES[random_int(0, DRONE_TYPES.size() - 1)];
        drone.identity = IdentityAffiliation::HOSTILE;
        drone.active = true;

        // Spawn at edge of detection range
        double spawn_bearing = random_double(0, 360) * DEG_TO_RAD;
        double spawn_range = range_m_ * random_double(0.7, 0.95);

        double lat_delta = (spawn_range / METERS_PER_DEG_LAT) * std::cos(spawn_bearing);
        double lon_delta = (spawn_range / (METERS_PER_DEG_LAT * std::cos(center_lat_ * DEG_TO_RAD))) * std::sin(spawn_bearing);

        drone.latitude = center_lat_ + lat_delta;
        drone.longitude = center_lon_ + lon_delta;
        drone.altitude = random_double(50, 200);

        // Head toward center
        drone.heading = std::fmod(spawn_bearing * 180.0 / M_PI + 180.0, 360.0);
        drone.speed = random_double(5, 25);  // 5-25 m/s

        drones_.push_back(drone);
    }
}

void DroneSim::clear() {
    drones_.clear();
}

void DroneSim::calculate_bearing_range(const SimulatedDrone& drone,
                                        double& bearing, double& range) const {
    const double DEG_TO_RAD = M_PI / 180.0;
    const double RAD_TO_DEG = 180.0 / M_PI;
    const double METERS_PER_DEG_LAT = 111320.0;

    double lat_diff = drone.latitude - center_lat_;
    double lon_diff = drone.longitude - center_lon_;

    double lat_m = lat_diff * METERS_PER_DEG_LAT;
    double lon_m = lon_diff * METERS_PER_DEG_LAT * std::cos(center_lat_ * DEG_TO_RAD);

    range = std::sqrt(lat_m * lat_m + lon_m * lon_m);
    bearing = std::atan2(lon_m, lat_m) * RAD_TO_DEG;
    if (bearing < 0) bearing += 360.0;
}

void DroneSim::add_noise(Detection& det) const {
    // Add position noise (5% of range)
    double noise_m = det.range * 0.05;
    double noise_lat = random_double(-noise_m, noise_m) / 111320.0;
    double noise_lon = random_double(-noise_m, noise_m) / (111320.0 * std::cos(det.latitude * M_PI / 180.0));

    det.latitude += noise_lat;
    det.longitude += noise_lon;
    det.hae += random_double(-5, 5);

    // Set error estimates
    det.circular_error = noise_m;
    det.linear_error = 15.0;
    det.bearing_accuracy = random_double(1.0, 3.0);
}

} // namespace cuas
