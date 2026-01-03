#ifndef CUAS_MESH_CORRELATOR_H
#define CUAS_MESH_CORRELATOR_H

#include <vector>
#include <string>
#include <map>

#include "models/detection.h"
#include "models/track.h"

namespace cuas {

/// Configuration for correlator
struct CorrelatorConfig {
    double correlation_distance_m = 500.0;   // Max distance to correlate
    double correlation_time_ms = 10000;      // Max time window (10s)
    int min_detections_for_track = 2;        // Min detections to form track
    int track_timeout_ms = 30000;            // Track drops after 30s no update
};

/// Detection-to-track correlator using simple nearest neighbor
class Correlator {
public:
    Correlator();
    explicit Correlator(const CorrelatorConfig& config);

    ~Correlator();

    /// Correlate new detections with existing tracks
    /// @param detections New detections to process
    /// @param existing_tracks Current track list
    /// @return Updated/new tracks
    std::vector<Track> correlate(const std::vector<Detection>& detections,
                                  const std::vector<Track>& existing_tracks);

    /// Get configuration
    const CorrelatorConfig& get_config() const { return config_; }

    /// Update configuration
    void set_config(const CorrelatorConfig& config) { config_ = config; }

    /// Clear correlation state
    void reset();

private:
    CorrelatorConfig config_;

    // Track correlation state
    std::map<std::string, std::vector<Detection>> track_detections_;
    int track_counter_ = 0;

    /// Calculate distance between two positions in meters
    double calculate_distance(double lat1, double lon1,
                              double lat2, double lon2) const;

    /// Find best matching track for a detection
    /// @return Track ID or empty string if no match
    std::string find_matching_track(const Detection& det,
                                     const std::vector<Track>& tracks) const;

    /// Create new track from detection
    Track create_track(const Detection& det);

    /// Update track with new detection
    void update_track(Track& track, const Detection& det);

    /// Calculate fused position from detections
    void fuse_position(Track& track, const std::vector<Detection>& detections);

    /// Determine track identity from detections
    IdentityAffiliation determine_identity(const std::vector<Detection>& detections) const;

    /// Check if track should be dropped
    bool is_track_stale(const Track& track) const;
};

} // namespace cuas

#endif // CUAS_MESH_CORRELATOR_H
