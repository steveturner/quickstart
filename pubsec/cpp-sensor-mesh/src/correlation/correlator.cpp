#include "correlator.h"

#include <cmath>
#include <algorithm>
#include <chrono>

namespace cuas {

namespace {
    int64_t now_ms() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }
}

Correlator::Correlator() : config_() {}

Correlator::Correlator(const CorrelatorConfig& config) : config_(config) {}

Correlator::~Correlator() = default;

std::vector<Track> Correlator::correlate(const std::vector<Detection>& detections,
                                          const std::vector<Track>& existing_tracks) {
    std::vector<Track> updated_tracks;

    // Copy existing tracks
    std::map<std::string, Track> track_map;
    for (const auto& t : existing_tracks) {
        track_map[t._id] = t;
    }

    // Process each detection
    for (const auto& det : detections) {
        std::string matched_track_id = find_matching_track(det, existing_tracks);

        if (!matched_track_id.empty()) {
            // Update existing track
            auto& track = track_map[matched_track_id];
            update_track(track, det);
            track_detections_[matched_track_id].push_back(det);

            // Fuse position from all recent detections
            fuse_position(track, track_detections_[matched_track_id]);
        } else {
            // Create new track
            Track new_track = create_track(det);
            track_map[new_track._id] = new_track;
            track_detections_[new_track._id] = {det};
        }
    }

    // Check for stale tracks and collect results
    for (auto& [id, track] : track_map) {
        if (is_track_stale(track)) {
            track.status = TrackStatus::LOST;
        }
        updated_tracks.push_back(track);
    }

    // Clean up old detections
    int64_t cutoff = now_ms() - config_.correlation_time_ms;
    for (auto& [id, dets] : track_detections_) {
        dets.erase(
            std::remove_if(dets.begin(), dets.end(),
                [cutoff](const Detection& d) { return d.created < cutoff; }),
            dets.end());
    }

    return updated_tracks;
}

void Correlator::reset() {
    track_detections_.clear();
    track_counter_ = 0;
}

double Correlator::calculate_distance(double lat1, double lon1,
                                       double lat2, double lon2) const {
    const double DEG_TO_RAD = M_PI / 180.0;
    const double EARTH_RADIUS_M = 6371000.0;

    double lat1_rad = lat1 * DEG_TO_RAD;
    double lat2_rad = lat2 * DEG_TO_RAD;
    double delta_lat = (lat2 - lat1) * DEG_TO_RAD;
    double delta_lon = (lon2 - lon1) * DEG_TO_RAD;

    double a = std::sin(delta_lat / 2) * std::sin(delta_lat / 2) +
               std::cos(lat1_rad) * std::cos(lat2_rad) *
               std::sin(delta_lon / 2) * std::sin(delta_lon / 2);

    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));

    return EARTH_RADIUS_M * c;
}

std::string Correlator::find_matching_track(const Detection& det,
                                             const std::vector<Track>& tracks) const {
    std::string best_match;
    double best_distance = config_.correlation_distance_m;

    for (const auto& track : tracks) {
        if (track.status == TrackStatus::LOST) continue;

        double dist = calculate_distance(det.latitude, det.longitude,
                                         track.latitude, track.longitude);

        if (dist < best_distance) {
            best_distance = dist;
            best_match = track._id;
        }
    }

    return best_match;
}

Track Correlator::create_track(const Detection& det) {
    Track track;
    track._id = Track::generate_id();
    track.track_number = Track::generate_track_number();
    track.status = TrackStatus::ACTIVE;
    track.identity = det.identity;
    track.platform_type = det.platform_type;
    track.latitude = det.latitude;
    track.longitude = det.longitude;
    track.hae = det.hae;
    track.track_quality = det.track_quality;
    track.detection_ids.push_back(det._id);
    track.last_update = now_ms();

    return track;
}

void Correlator::update_track(Track& track, const Detection& det) {
    track.detection_ids.push_back(det._id);
    track.last_update = now_ms();

    // Update identity if detection provides stronger signal
    if (det.identity == IdentityAffiliation::HOSTILE) {
        track.identity = IdentityAffiliation::HOSTILE;
    }

    // Platform type from latest detection
    if (!det.platform_type.empty()) {
        track.platform_type = det.platform_type;
    }

    // Improve track quality with more detections
    track.track_quality = std::min(100, track.track_quality + 5);
}

void Correlator::fuse_position(Track& track, const std::vector<Detection>& detections) {
    if (detections.empty()) return;

    // Simple weighted average based on track quality
    double total_weight = 0;
    double lat_sum = 0;
    double lon_sum = 0;
    double alt_sum = 0;

    for (const auto& det : detections) {
        double weight = det.track_quality / 100.0;
        total_weight += weight;
        lat_sum += det.latitude * weight;
        lon_sum += det.longitude * weight;
        alt_sum += det.hae * weight;
    }

    if (total_weight > 0) {
        track.latitude = lat_sum / total_weight;
        track.longitude = lon_sum / total_weight;
        track.hae = alt_sum / total_weight;
    }

    // Determine identity from all detections
    track.identity = determine_identity(detections);
}

IdentityAffiliation Correlator::determine_identity(const std::vector<Detection>& detections) const {
    // Most severe identity wins
    IdentityAffiliation result = IdentityAffiliation::UNKNOWN;

    for (const auto& det : detections) {
        if (det.identity == IdentityAffiliation::HOSTILE) {
            return IdentityAffiliation::HOSTILE;
        }
        if (det.identity == IdentityAffiliation::SUSPECT && result != IdentityAffiliation::HOSTILE) {
            result = IdentityAffiliation::SUSPECT;
        }
        if (det.identity == IdentityAffiliation::PENDING && result == IdentityAffiliation::UNKNOWN) {
            result = IdentityAffiliation::PENDING;
        }
    }

    return result;
}

bool Correlator::is_track_stale(const Track& track) const {
    int64_t now = now_ms();
    return (now - track.last_update) > config_.track_timeout_ms;
}

} // namespace cuas
