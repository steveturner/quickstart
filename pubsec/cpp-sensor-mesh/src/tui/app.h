#ifndef CUAS_MESH_TUI_APP_H
#define CUAS_MESH_TUI_APP_H

#include <memory>
#include <string>
#include <vector>
#include <functional>

#include "models/sensor.h"
#include "models/track.h"
#include "models/alert.h"
#include "models/assignment.h"

namespace cuas {

class MeshPeer;
class SensorNode;
class C2Node;
class EffectorNode;

/// Mode for the TUI
enum class NodeMode {
    SENSOR,
    C2,
    EFFECTOR
};

/// TUI Application
class App {
public:
    App(NodeMode mode, const std::string& node_id);
    ~App();

    /// Initialize with mesh peer
    void init(std::shared_ptr<MeshPeer> peer);

    /// Run the TUI (blocking)
    void run();

    /// Stop the TUI
    void stop();

private:
    NodeMode mode_;
    std::string node_id_;
    std::shared_ptr<MeshPeer> peer_;

    // Node instances (only one active based on mode)
    std::unique_ptr<SensorNode> sensor_node_;
    std::unique_ptr<C2Node> c2_node_;
    std::unique_ptr<EffectorNode> effector_node_;

    bool running_ = false;

    class Impl;
    std::unique_ptr<Impl> impl_;

    // TUI update methods
    void setup_sensor_view();
    void setup_c2_view();
    void setup_effector_view();
};

} // namespace cuas

#endif // CUAS_MESH_TUI_APP_H
