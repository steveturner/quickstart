#include "mesh_peer.h"
#include "collections.h"

#include "Ditto.h"

#include <iostream>
#include <mutex>
#include <sstream>
#include <stdexcept>

using namespace std;
using json = nlohmann::json;

namespace cuas {

// Helper to convert QueryResultItem to model
template<typename T>
static T item_from(const ditto::QueryResultItem& item) {
    return T::from_json(json::parse(item.json_string()));
}

// Helper to convert QueryResult to vector of models
template<typename T>
static vector<T> items_from(const ditto::QueryResult& result) {
    vector<T> items;
    items.reserve(result.item_count());
    for (size_t i = 0; i < result.item_count(); ++i) {
        items.emplace_back(item_from<T>(result.get_item(i)));
    }
    return items;
}

// Initialize Ditto instance
static shared_ptr<ditto::Ditto> init_ditto(
    const string& app_id,
    const string& playground_token,
    const string& websocket_url,
    const string& auth_url,
    bool enable_cloud_sync,
    const string& persistence_dir) {

    try {
        auto identity = ditto::Identity::OnlinePlayground(
            app_id, playground_token, enable_cloud_sync, auth_url);

        auto ditto = make_shared<ditto::Ditto>(identity, persistence_dir);

        ditto->update_transport_config([&websocket_url](ditto::TransportConfig& config) {
            config.enable_all_peer_to_peer();
            config.connect.websocket_urls.insert(websocket_url);
        });

        // Required for DQL compatibility
        ditto->disable_sync_with_v3();

        // Disable strict mode
        ditto->get_store().execute("ALTER SYSTEM SET DQL_STRICT_MODE = false");

        return ditto;
    } catch (const exception& err) {
        throw runtime_error("Failed to initialize Ditto: " + string(err.what()));
    }
}

// Private implementation
class MeshPeer::Impl {
private:
    // Use recursive_mutex to allow callbacks to call back into MeshPeer methods
    // This is necessary because the stub SDK calls observers synchronously
    shared_ptr<recursive_mutex> mtx;
    shared_ptr<ditto::Ditto> ditto;
    shared_ptr<ditto::SyncSubscription> sensors_sub;
    shared_ptr<ditto::SyncSubscription> detections_sub;
    shared_ptr<ditto::SyncSubscription> tracks_sub;
    shared_ptr<ditto::SyncSubscription> alerts_sub;
    shared_ptr<ditto::SyncSubscription> assignments_sub;

public:
    Impl(const string& app_id,
         const string& playground_token,
         const string& websocket_url,
         const string& auth_url,
         bool enable_cloud_sync,
         const string& persistence_dir)
        : mtx(make_shared<recursive_mutex>()),
          ditto(init_ditto(app_id, playground_token, websocket_url,
                          auth_url, enable_cloud_sync, persistence_dir)) {}

    ~Impl() noexcept {
        try {
            stop_sync();
        } catch (...) {}
    }

    void start_sync() {
        if (ditto->get_is_sync_active()) return;

        ditto->start_sync();

        // Register subscriptions for all collections
        sensors_sub = ditto->get_sync().register_subscription(
            string("SELECT * FROM ") + COLLECTION_SENSORS);
        detections_sub = ditto->get_sync().register_subscription(
            string("SELECT * FROM ") + COLLECTION_DETECTIONS);
        tracks_sub = ditto->get_sync().register_subscription(
            string("SELECT * FROM ") + COLLECTION_TRACKS);
        alerts_sub = ditto->get_sync().register_subscription(
            string("SELECT * FROM ") + COLLECTION_ALERTS);
        assignments_sub = ditto->get_sync().register_subscription(
            string("SELECT * FROM ") + COLLECTION_ASSIGNMENTS);
    }

    void stop_sync() {
        if (!ditto->get_is_sync_active()) return;

        if (sensors_sub) { sensors_sub->cancel(); sensors_sub.reset(); }
        if (detections_sub) { detections_sub->cancel(); detections_sub.reset(); }
        if (tracks_sub) { tracks_sub->cancel(); tracks_sub.reset(); }
        if (alerts_sub) { alerts_sub->cancel(); alerts_sub.reset(); }
        if (assignments_sub) { assignments_sub->cancel(); assignments_sub.reset(); }

        ditto->stop_sync();
    }

    bool is_sync_active() const {
        return ditto->get_is_sync_active();
    }

    int get_peer_count() const {
        // This is a simplified version - actual peer count requires presence API
        return is_sync_active() ? 1 : 0;
    }

    // ==================== Sensor Operations ====================

    string upsert_sensor(const Sensor& sensor) {
        lock_guard<recursive_mutex> lock(*mtx);
        try {
            auto j = sensor.to_json();
            string cmd = string("INSERT INTO ") + COLLECTION_SENSORS +
                        " DOCUMENTS (:doc) ON ID CONFLICT DO UPDATE";
            ditto->get_store().execute(cmd, {{"doc", j}});
            return sensor._id;
        } catch (const exception& err) {
            throw runtime_error("Failed to upsert sensor: " + string(err.what()));
        }
    }

    vector<Sensor> get_sensors(bool include_deleted) {
        lock_guard<recursive_mutex> lock(*mtx);
        try {
            string query = string("SELECT * FROM ") + COLLECTION_SENSORS;
            if (!include_deleted) {
                query += " WHERE NOT deleted";
            }
            auto result = ditto->get_store().execute(query);
            return items_from<Sensor>(result);
        } catch (const exception& err) {
            throw runtime_error("Failed to get sensors: " + string(err.what()));
        }
    }

    Sensor get_sensor(const string& sensor_id) {
        lock_guard<recursive_mutex> lock(*mtx);
        try {
            string query = string("SELECT * FROM ") + COLLECTION_SENSORS +
                          " WHERE _id = :id";
            auto result = ditto->get_store().execute(query, {{"id", sensor_id}});
            if (result.item_count() == 0) {
                throw runtime_error("Sensor not found: " + sensor_id);
            }
            return item_from<Sensor>(result.get_item(0));
        } catch (const exception& err) {
            throw runtime_error("Failed to get sensor: " + string(err.what()));
        }
    }

    void update_sensor_heartbeat(const string& sensor_id) {
        lock_guard<recursive_mutex> lock(*mtx);
        try {
            auto now = chrono::duration_cast<chrono::milliseconds>(
                chrono::system_clock::now().time_since_epoch()).count();
            string cmd = string("UPDATE ") + COLLECTION_SENSORS +
                        " SET last_heartbeat = :ts WHERE _id = :id";
            ditto->get_store().execute(cmd, {{"ts", now}, {"id", sensor_id}});
        } catch (const exception& err) {
            throw runtime_error("Failed to update heartbeat: " + string(err.what()));
        }
    }

    shared_ptr<ditto::StoreObserver> register_sensor_observer(
        function<void(const vector<Sensor>&)> callback) {
        try {
            string query = string("SELECT * FROM ") + COLLECTION_SENSORS +
                          " WHERE NOT deleted";
            return ditto->get_store().register_observer(query,
                [callback](const ditto::QueryResult& result) {
                    callback(items_from<Sensor>(result));
                });
        } catch (const exception& err) {
            throw runtime_error("Failed to register sensor observer: " + string(err.what()));
        }
    }

    // ==================== Detection Operations ====================

    string add_detection(const Detection& detection) {
        lock_guard<recursive_mutex> lock(*mtx);
        try {
            auto j = detection.to_json();
            string cmd = string("INSERT INTO ") + COLLECTION_DETECTIONS +
                        " DOCUMENTS (:doc)";
            auto result = ditto->get_store().execute(cmd, {{"doc", j}});
            return result.mutated_document_ids()[0].to_string();
        } catch (const exception& err) {
            throw runtime_error("Failed to add detection: " + string(err.what()));
        }
    }

    vector<Detection> get_detections(bool include_deleted) {
        lock_guard<recursive_mutex> lock(*mtx);
        try {
            string query = string("SELECT * FROM ") + COLLECTION_DETECTIONS;
            if (!include_deleted) {
                query += " WHERE NOT deleted";
            }
            query += " ORDER BY created DESC";
            auto result = ditto->get_store().execute(query);
            return items_from<Detection>(result);
        } catch (const exception& err) {
            throw runtime_error("Failed to get detections: " + string(err.what()));
        }
    }

    vector<Detection> get_detections_by_sensor(const string& sensor_id) {
        lock_guard<recursive_mutex> lock(*mtx);
        try {
            string query = string("SELECT * FROM ") + COLLECTION_DETECTIONS +
                          " WHERE sensor_id = :sid AND NOT deleted ORDER BY created DESC";
            auto result = ditto->get_store().execute(query, {{"sid", sensor_id}});
            return items_from<Detection>(result);
        } catch (const exception& err) {
            throw runtime_error("Failed to get detections by sensor: " + string(err.what()));
        }
    }

    void delete_detection(const string& detection_id) {
        lock_guard<recursive_mutex> lock(*mtx);
        try {
            string cmd = string("UPDATE ") + COLLECTION_DETECTIONS +
                        " SET deleted = true WHERE _id = :id";
            ditto->get_store().execute(cmd, {{"id", detection_id}});
        } catch (const exception& err) {
            throw runtime_error("Failed to delete detection: " + string(err.what()));
        }
    }

    shared_ptr<ditto::StoreObserver> register_detection_observer(
        function<void(const vector<Detection>&)> callback) {
        try {
            string query = string("SELECT * FROM ") + COLLECTION_DETECTIONS +
                          " WHERE NOT deleted ORDER BY created DESC";
            return ditto->get_store().register_observer(query,
                [callback](const ditto::QueryResult& result) {
                    callback(items_from<Detection>(result));
                });
        } catch (const exception& err) {
            throw runtime_error("Failed to register detection observer: " + string(err.what()));
        }
    }

    // ==================== Track Operations ====================

    string upsert_track(const Track& track) {
        lock_guard<recursive_mutex> lock(*mtx);
        try {
            auto j = track.to_json();
            string cmd = string("INSERT INTO ") + COLLECTION_TRACKS +
                        " DOCUMENTS (:doc) ON ID CONFLICT DO UPDATE";
            ditto->get_store().execute(cmd, {{"doc", j}});
            return track._id;
        } catch (const exception& err) {
            throw runtime_error("Failed to upsert track: " + string(err.what()));
        }
    }

    vector<Track> get_tracks(bool include_deleted) {
        lock_guard<recursive_mutex> lock(*mtx);
        try {
            string query = string("SELECT * FROM ") + COLLECTION_TRACKS;
            if (!include_deleted) {
                query += " WHERE NOT deleted";
            }
            query += " ORDER BY last_update DESC";
            auto result = ditto->get_store().execute(query);
            return items_from<Track>(result);
        } catch (const exception& err) {
            throw runtime_error("Failed to get tracks: " + string(err.what()));
        }
    }

    Track get_track(const string& track_id) {
        lock_guard<recursive_mutex> lock(*mtx);
        try {
            string query = string("SELECT * FROM ") + COLLECTION_TRACKS +
                          " WHERE _id = :id";
            auto result = ditto->get_store().execute(query, {{"id", track_id}});
            if (result.item_count() == 0) {
                throw runtime_error("Track not found: " + track_id);
            }
            return item_from<Track>(result.get_item(0));
        } catch (const exception& err) {
            throw runtime_error("Failed to get track: " + string(err.what()));
        }
    }

    void update_track_status(const string& track_id, TrackStatus status) {
        lock_guard<recursive_mutex> lock(*mtx);
        try {
            auto now = chrono::duration_cast<chrono::milliseconds>(
                chrono::system_clock::now().time_since_epoch()).count();
            string cmd = string("UPDATE ") + COLLECTION_TRACKS +
                        " SET status = :status, last_update = :ts WHERE _id = :id";
            ditto->get_store().execute(cmd, {
                {"status", trackStatusToString(status)},
                {"ts", now},
                {"id", track_id}
            });
        } catch (const exception& err) {
            throw runtime_error("Failed to update track status: " + string(err.what()));
        }
    }

    void add_detection_to_track(const string& track_id, const string& detection_id) {
        // Note: Array operations in DQL may require different approach
        // This is a simplified version
        lock_guard<recursive_mutex> lock(*mtx);
        try {
            auto track = get_track(track_id);
            track.detection_ids.push_back(detection_id);
            upsert_track(track);
        } catch (const exception& err) {
            throw runtime_error("Failed to add detection to track: " + string(err.what()));
        }
    }

    shared_ptr<ditto::StoreObserver> register_track_observer(
        function<void(const vector<Track>&)> callback) {
        try {
            string query = string("SELECT * FROM ") + COLLECTION_TRACKS +
                          " WHERE NOT deleted ORDER BY last_update DESC";
            return ditto->get_store().register_observer(query,
                [callback](const ditto::QueryResult& result) {
                    callback(items_from<Track>(result));
                });
        } catch (const exception& err) {
            throw runtime_error("Failed to register track observer: " + string(err.what()));
        }
    }

    // ==================== Alert Operations ====================

    string add_alert(const Alert& alert) {
        lock_guard<recursive_mutex> lock(*mtx);
        try {
            auto j = alert.to_json();
            string cmd = string("INSERT INTO ") + COLLECTION_ALERTS +
                        " DOCUMENTS (:doc)";
            auto result = ditto->get_store().execute(cmd, {{"doc", j}});
            return result.mutated_document_ids()[0].to_string();
        } catch (const exception& err) {
            throw runtime_error("Failed to add alert: " + string(err.what()));
        }
    }

    vector<Alert> get_alerts(bool include_deleted) {
        lock_guard<recursive_mutex> lock(*mtx);
        try {
            string query = string("SELECT * FROM ") + COLLECTION_ALERTS;
            if (!include_deleted) {
                query += " WHERE NOT deleted";
            }
            query += " ORDER BY created DESC";
            auto result = ditto->get_store().execute(query);
            return items_from<Alert>(result);
        } catch (const exception& err) {
            throw runtime_error("Failed to get alerts: " + string(err.what()));
        }
    }

    vector<Alert> get_pending_alerts() {
        lock_guard<recursive_mutex> lock(*mtx);
        try {
            string query = string("SELECT * FROM ") + COLLECTION_ALERTS +
                          " WHERE state = 1 AND NOT deleted ORDER BY created DESC";
            auto result = ditto->get_store().execute(query);
            return items_from<Alert>(result);
        } catch (const exception& err) {
            throw runtime_error("Failed to get pending alerts: " + string(err.what()));
        }
    }

    void respond_to_alert(const string& alert_id, AlertResponse response) {
        lock_guard<recursive_mutex> lock(*mtx);
        try {
            string cmd = string("UPDATE ") + COLLECTION_ALERTS +
                        " SET response = :resp, state = 2 WHERE _id = :id";
            ditto->get_store().execute(cmd, {
                {"resp", static_cast<int>(response)},
                {"id", alert_id}
            });
        } catch (const exception& err) {
            throw runtime_error("Failed to respond to alert: " + string(err.what()));
        }
    }

    void close_alert(const string& alert_id) {
        lock_guard<recursive_mutex> lock(*mtx);
        try {
            string cmd = string("UPDATE ") + COLLECTION_ALERTS +
                        " SET state = 0 WHERE _id = :id";
            ditto->get_store().execute(cmd, {{"id", alert_id}});
        } catch (const exception& err) {
            throw runtime_error("Failed to close alert: " + string(err.what()));
        }
    }

    shared_ptr<ditto::StoreObserver> register_alert_observer(
        function<void(const vector<Alert>&)> callback) {
        try {
            string query = string("SELECT * FROM ") + COLLECTION_ALERTS +
                          " WHERE NOT deleted ORDER BY created DESC";
            return ditto->get_store().register_observer(query,
                [callback](const ditto::QueryResult& result) {
                    callback(items_from<Alert>(result));
                });
        } catch (const exception& err) {
            throw runtime_error("Failed to register alert observer: " + string(err.what()));
        }
    }

    // ==================== Assignment Operations ====================

    string add_assignment(const Assignment& assignment) {
        lock_guard<recursive_mutex> lock(*mtx);
        try {
            auto j = assignment.to_json();
            string cmd = string("INSERT INTO ") + COLLECTION_ASSIGNMENTS +
                        " DOCUMENTS (:doc)";
            auto result = ditto->get_store().execute(cmd, {{"doc", j}});
            return result.mutated_document_ids()[0].to_string();
        } catch (const exception& err) {
            throw runtime_error("Failed to add assignment: " + string(err.what()));
        }
    }

    vector<Assignment> get_assignments(bool include_deleted) {
        lock_guard<recursive_mutex> lock(*mtx);
        try {
            string query = string("SELECT * FROM ") + COLLECTION_ASSIGNMENTS;
            if (!include_deleted) {
                query += " WHERE NOT deleted";
            }
            query += " ORDER BY created DESC";
            auto result = ditto->get_store().execute(query);
            return items_from<Assignment>(result);
        } catch (const exception& err) {
            throw runtime_error("Failed to get assignments: " + string(err.what()));
        }
    }

    vector<Assignment> get_assignments_for_effector(const string& effector_id) {
        lock_guard<recursive_mutex> lock(*mtx);
        try {
            string query = string("SELECT * FROM ") + COLLECTION_ASSIGNMENTS +
                          " WHERE effector_id = :eid AND NOT deleted ORDER BY created DESC";
            auto result = ditto->get_store().execute(query, {{"eid", effector_id}});
            return items_from<Assignment>(result);
        } catch (const exception& err) {
            throw runtime_error("Failed to get assignments for effector: " + string(err.what()));
        }
    }

    Assignment get_active_assignment(const string& effector_id) {
        lock_guard<recursive_mutex> lock(*mtx);
        try {
            string query = string("SELECT * FROM ") + COLLECTION_ASSIGNMENTS +
                          " WHERE effector_id = :eid AND status != 'complete' AND status != 'aborted' AND NOT deleted";
            auto result = ditto->get_store().execute(query, {{"eid", effector_id}});
            if (result.item_count() == 0) {
                throw runtime_error("No active assignment for effector: " + effector_id);
            }
            return item_from<Assignment>(result.get_item(0));
        } catch (const exception& err) {
            throw runtime_error("Failed to get active assignment: " + string(err.what()));
        }
    }

    void update_assignment_status(const string& assignment_id, AssignmentStatus status) {
        lock_guard<recursive_mutex> lock(*mtx);
        try {
            string cmd = string("UPDATE ") + COLLECTION_ASSIGNMENTS +
                        " SET status = :status WHERE _id = :id";
            ditto->get_store().execute(cmd, {
                {"status", assignmentStatusToString(status)},
                {"id", assignment_id}
            });
        } catch (const exception& err) {
            throw runtime_error("Failed to update assignment status: " + string(err.what()));
        }
    }

    void update_assignment_progress(const string& assignment_id, int progress) {
        lock_guard<recursive_mutex> lock(*mtx);
        try {
            string cmd = string("UPDATE ") + COLLECTION_ASSIGNMENTS +
                        " SET progress = :progress WHERE _id = :id";
            ditto->get_store().execute(cmd, {
                {"progress", progress},
                {"id", assignment_id}
            });
        } catch (const exception& err) {
            throw runtime_error("Failed to update assignment progress: " + string(err.what()));
        }
    }

    void complete_assignment(const string& assignment_id, const string& result_str) {
        lock_guard<recursive_mutex> lock(*mtx);
        try {
            string cmd = string("UPDATE ") + COLLECTION_ASSIGNMENTS +
                        " SET status = 'complete', result = :result WHERE _id = :id";
            ditto->get_store().execute(cmd, {
                {"result", result_str},
                {"id", assignment_id}
            });
        } catch (const exception& err) {
            throw runtime_error("Failed to complete assignment: " + string(err.what()));
        }
    }

    shared_ptr<ditto::StoreObserver> register_assignment_observer(
        function<void(const vector<Assignment>&)> callback) {
        try {
            string query = string("SELECT * FROM ") + COLLECTION_ASSIGNMENTS +
                          " WHERE NOT deleted ORDER BY created DESC";
            return ditto->get_store().register_observer(query,
                [callback](const ditto::QueryResult& result) {
                    callback(items_from<Assignment>(result));
                });
        } catch (const exception& err) {
            throw runtime_error("Failed to register assignment observer: " + string(err.what()));
        }
    }

    string execute_dql(const string& query) {
        lock_guard<recursive_mutex> lock(*mtx);
        try {
            auto result = ditto->get_store().execute(query);
            json output;
            output["item_count"] = result.item_count();
            vector<string> items;
            for (size_t i = 0; i < result.item_count(); ++i) {
                items.push_back(result.get_item(i).json_string());
            }
            output["items"] = items;
            return output.dump();
        } catch (const exception& err) {
            throw runtime_error("Failed to execute DQL: " + string(err.what()));
        }
    }

    void clear_all_data() {
        lock_guard<recursive_mutex> lock(*mtx);
        try {
            // EVICT removes data from local store and syncs deletion to peers
            ditto->get_store().execute(string("EVICT FROM ") + COLLECTION_SENSORS);
            ditto->get_store().execute(string("EVICT FROM ") + COLLECTION_DETECTIONS);
            ditto->get_store().execute(string("EVICT FROM ") + COLLECTION_TRACKS);
            ditto->get_store().execute(string("EVICT FROM ") + COLLECTION_ALERTS);
            ditto->get_store().execute(string("EVICT FROM ") + COLLECTION_ASSIGNMENTS);
        } catch (const exception& err) {
            throw runtime_error("Failed to clear data: " + string(err.what()));
        }
    }
};

// ==================== MeshPeer Public Methods ====================

string MeshPeer::get_ditto_sdk_version() {
    return ditto::Ditto::get_sdk_version();
}

MeshPeer::MeshPeer(const string& app_id,
                   const string& playground_token,
                   const string& websocket_url,
                   const string& auth_url,
                   bool enable_cloud_sync,
                   const string& persistence_dir)
    : impl(make_unique<Impl>(app_id, playground_token, websocket_url,
                             auth_url, enable_cloud_sync, persistence_dir)) {}

MeshPeer::~MeshPeer() noexcept = default;

void MeshPeer::start_sync() { impl->start_sync(); }
void MeshPeer::stop_sync() { impl->stop_sync(); }
bool MeshPeer::is_sync_active() const { return impl->is_sync_active(); }
int MeshPeer::get_peer_count() const { return impl->get_peer_count(); }

string MeshPeer::upsert_sensor(const Sensor& s) { return impl->upsert_sensor(s); }
vector<Sensor> MeshPeer::get_sensors(bool d) { return impl->get_sensors(d); }
Sensor MeshPeer::get_sensor(const string& id) { return impl->get_sensor(id); }
void MeshPeer::update_sensor_heartbeat(const string& id) { impl->update_sensor_heartbeat(id); }
shared_ptr<ditto::StoreObserver> MeshPeer::register_sensor_observer(
    function<void(const vector<Sensor>&)> cb) { return impl->register_sensor_observer(cb); }

string MeshPeer::add_detection(const Detection& d) { return impl->add_detection(d); }
vector<Detection> MeshPeer::get_detections(bool d) { return impl->get_detections(d); }
vector<Detection> MeshPeer::get_detections_by_sensor(const string& id) {
    return impl->get_detections_by_sensor(id); }
void MeshPeer::delete_detection(const string& id) { impl->delete_detection(id); }
shared_ptr<ditto::StoreObserver> MeshPeer::register_detection_observer(
    function<void(const vector<Detection>&)> cb) { return impl->register_detection_observer(cb); }

string MeshPeer::upsert_track(const Track& t) { return impl->upsert_track(t); }
vector<Track> MeshPeer::get_tracks(bool d) { return impl->get_tracks(d); }
Track MeshPeer::get_track(const string& id) { return impl->get_track(id); }
void MeshPeer::update_track_status(const string& id, TrackStatus s) {
    impl->update_track_status(id, s); }
void MeshPeer::add_detection_to_track(const string& tid, const string& did) {
    impl->add_detection_to_track(tid, did); }
shared_ptr<ditto::StoreObserver> MeshPeer::register_track_observer(
    function<void(const vector<Track>&)> cb) { return impl->register_track_observer(cb); }

string MeshPeer::add_alert(const Alert& a) { return impl->add_alert(a); }
vector<Alert> MeshPeer::get_alerts(bool d) { return impl->get_alerts(d); }
vector<Alert> MeshPeer::get_pending_alerts() { return impl->get_pending_alerts(); }
void MeshPeer::respond_to_alert(const string& id, AlertResponse r) {
    impl->respond_to_alert(id, r); }
void MeshPeer::close_alert(const string& id) { impl->close_alert(id); }
shared_ptr<ditto::StoreObserver> MeshPeer::register_alert_observer(
    function<void(const vector<Alert>&)> cb) { return impl->register_alert_observer(cb); }

string MeshPeer::add_assignment(const Assignment& a) { return impl->add_assignment(a); }
vector<Assignment> MeshPeer::get_assignments(bool d) { return impl->get_assignments(d); }
vector<Assignment> MeshPeer::get_assignments_for_effector(const string& id) {
    return impl->get_assignments_for_effector(id); }
Assignment MeshPeer::get_active_assignment(const string& id) {
    return impl->get_active_assignment(id); }
void MeshPeer::update_assignment_status(const string& id, AssignmentStatus s) {
    impl->update_assignment_status(id, s); }
void MeshPeer::update_assignment_progress(const string& id, int p) {
    impl->update_assignment_progress(id, p); }
void MeshPeer::complete_assignment(const string& id, const string& r) {
    impl->complete_assignment(id, r); }
shared_ptr<ditto::StoreObserver> MeshPeer::register_assignment_observer(
    function<void(const vector<Assignment>&)> cb) { return impl->register_assignment_observer(cb); }

string MeshPeer::execute_dql(const string& q) { return impl->execute_dql(q); }

void MeshPeer::clear_all_data() { impl->clear_all_data(); }

} // namespace cuas
