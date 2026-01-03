#ifndef CUAS_MESH_EFFECTOR_NODE_H
#define CUAS_MESH_EFFECTOR_NODE_H

#include <memory>
#include <string>
#include <functional>
#include <vector>

#include "models/assignment.h"

namespace cuas {

class MeshPeer;

/// Effector node (interceptor or EW jammer) in the Counter-UAS mesh
class EffectorNode {
public:
    /// Constructor
    EffectorNode(std::shared_ptr<MeshPeer> peer,
                 const std::string& effector_id,
                 EffectorType type,
                 double lat, double lon);

    ~EffectorNode();

    /// Start the effector node
    void start();

    /// Stop the effector node
    void stop();

    /// Get effector ID
    const std::string& get_id() const { return effector_id_; }

    /// Get effector type
    EffectorType get_type() const { return type_; }

    /// Get effector type as string
    std::string get_type_string() const { return effectorTypeToString(type_); }

    /// Get current position
    double get_latitude() const { return latitude_; }
    double get_longitude() const { return longitude_; }

    /// Check if effector has active assignment
    bool has_active_assignment() const;

    /// Get current assignment (if any)
    const Assignment* get_current_assignment() const;

    /// Get assignment status
    AssignmentStatus get_status() const;

    /// Get ETA to target (seconds)
    int64_t get_eta() const;

    /// Get progress percentage (0-100)
    int get_progress() const;

    /// Confirm kill (interceptor)
    void confirm_kill();

    /// Confirm signal jamming (EW)
    void confirm_jamming();

    /// Abort current assignment
    void abort();

    /// Register callback for assignment updates
    void on_assignment_update(std::function<void(const Assignment*)> callback);

private:
    std::shared_ptr<MeshPeer> peer_;
    std::string effector_id_;
    EffectorType type_;
    double latitude_;
    double longitude_;
    double hae_ = 0.0;

    bool running_ = false;
    Assignment current_assignment_;
    bool has_assignment_ = false;
    int64_t assignment_start_time_ = 0;

    std::function<void(const Assignment*)> assignment_callback_;

    class Impl;
    std::unique_ptr<Impl> impl_;

    /// Process new assignment
    void process_assignment(const Assignment& assignment);

    /// Update assignment progress
    void update_progress();
};

} // namespace cuas

#endif // CUAS_MESH_EFFECTOR_NODE_H
