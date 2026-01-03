// Stub Ditto SDK header for compilation without actual SDK
// This stub provides in-memory storage for demo purposes
// Replace with real Ditto SDK for production use
//
// IMPORTANT: This stub is only for local development and testing.
// For real P2P synchronization, use the actual Ditto SDK on Linux.

#ifndef DITTO_STUB_H
#define DITTO_STUB_H

#include <string>
#include <memory>
#include <functional>
#include <vector>
#include <set>
#include <map>
#include <mutex>
#include <thread>
#include <atomic>
#include <regex>
#include <nlohmann/json.hpp>

namespace ditto {

// Forward declarations
class Ditto;
class Store;
class Sync;
class SyncSubscription;
class StoreObserver;
class QueryResult;
class QueryResultItem;
class DocumentId;

// Global in-memory document store (shared across all instances for demo)
// NOTE: This only works within a single process. For multi-process sync,
// use the real Ditto SDK which provides P2P synchronization.
class InMemoryStore {
public:
    static InMemoryStore& instance() {
        static InMemoryStore store;
        return store;
    }

    void upsert(const std::string& collection, const nlohmann::json& doc) {
        std::vector<std::function<void(const std::vector<nlohmann::json>&)>> callbacks;
        std::vector<nlohmann::json> data;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            std::string id = doc.value("_id", "");
            if (id.empty()) return;

            collections_[collection][id] = doc;

            // Get callbacks and data while holding the lock
            auto it = observers_.find(collection);
            if (it != observers_.end()) {
                callbacks = it->second;
                data = query_unlocked(collection);
            }
        }

        // Call callbacks outside the lock to avoid deadlocks
        // Note: callbacks run synchronously and may cause recursive calls
        for (size_t i = 0; i < callbacks.size(); ++i) {
            callbacks[i](data);
        }
    }

    void insert(const std::string& collection, const nlohmann::json& doc) {
        upsert(collection, doc);
    }

    std::vector<nlohmann::json> query(const std::string& collection) {
        std::lock_guard<std::mutex> lock(mutex_);
        return query_unlocked(collection);
    }

    void register_observer(const std::string& collection,
                           std::function<void(const std::vector<nlohmann::json>&)> callback) {
        std::vector<nlohmann::json> data;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            observers_[collection].push_back(callback);
            data = query_unlocked(collection);
        }
        // Call callback outside the lock
        callback(data);
    }

    void notify_observers(const std::string& collection) {
        std::vector<std::function<void(const std::vector<nlohmann::json>&)>> callbacks;
        std::vector<nlohmann::json> data;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = observers_.find(collection);
            if (it != observers_.end()) {
                callbacks = it->second;
                data = query_unlocked(collection);
            }
        }

        for (const auto& cb : callbacks) {
            cb(data);
        }
    }

    void update_document(const std::string& collection, const std::string& id,
                         const std::map<std::string, nlohmann::json>& updates) {
        std::vector<std::function<void(const std::vector<nlohmann::json>&)>> callbacks;
        std::vector<nlohmann::json> data;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto coll_it = collections_.find(collection);
            if (coll_it == collections_.end()) return;

            auto doc_it = coll_it->second.find(id);
            if (doc_it == coll_it->second.end()) return;

            // Apply updates to the document
            for (const auto& [key, value] : updates) {
                doc_it->second[key] = value;
            }

            // Get callbacks and data while holding the lock
            auto obs_it = observers_.find(collection);
            if (obs_it != observers_.end()) {
                callbacks = obs_it->second;
                data = query_unlocked(collection);
            }
        }

        // Call callbacks outside the lock to avoid deadlocks
        for (const auto& cb : callbacks) {
            cb(data);
        }
    }

    void clear_collection(const std::string& collection) {
        std::vector<std::function<void(const std::vector<nlohmann::json>&)>> callbacks;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            collections_[collection].clear();

            // Get callbacks while holding the lock
            auto obs_it = observers_.find(collection);
            if (obs_it != observers_.end()) {
                callbacks = obs_it->second;
            }
        }

        // Notify observers with empty data
        std::vector<nlohmann::json> empty;
        for (const auto& cb : callbacks) {
            cb(empty);
        }
    }

    std::string extract_collection(const std::string& query) {
        // Simple extraction: look for FROM <collection>
        std::regex from_regex(R"(FROM\s+(\w+))", std::regex::icase);
        std::smatch match;
        if (std::regex_search(query, match, from_regex)) {
            return match[1].str();
        }
        // Also check for INSERT INTO or UPDATE
        std::regex into_regex(R"(INTO\s+(\w+))", std::regex::icase);
        if (std::regex_search(query, match, into_regex)) {
            return match[1].str();
        }
        std::regex update_regex(R"(UPDATE\s+(\w+))", std::regex::icase);
        if (std::regex_search(query, match, update_regex)) {
            return match[1].str();
        }
        return "";
    }

private:
    InMemoryStore() = default;

    // Internal query without locking - must be called with mutex held
    std::vector<nlohmann::json> query_unlocked(const std::string& collection) {
        std::vector<nlohmann::json> results;
        auto it = collections_.find(collection);
        if (it != collections_.end()) {
            for (const auto& [id, doc] : it->second) {
                // Filter out deleted documents
                if (!doc.value("deleted", false)) {
                    results.push_back(doc);
                }
            }
        }
        return results;
    }

    std::mutex mutex_;
    std::map<std::string, std::map<std::string, nlohmann::json>> collections_;
    std::map<std::string, std::vector<std::function<void(const std::vector<nlohmann::json>&)>>> observers_;
};

// Document ID
class DocumentId {
public:
    DocumentId() : id_("stub-" + std::to_string(counter_++)) {}
    explicit DocumentId(const std::string& id) : id_(id) {}
    std::string to_string() const { return id_; }
private:
    std::string id_;
    static inline int counter_ = 0;
};

// Transport config
struct TransportConfig {
    struct Connect {
        std::set<std::string> websocket_urls;
    } connect;

    void enable_all_peer_to_peer() {}
};

// Identity
class Identity {
public:
    static Identity OnlinePlayground(
        const std::string& app_id,
        const std::string& token,
        bool enable_cloud_sync,
        const std::string& auth_url) {
        return Identity();
    }
};

// Query result item
class QueryResultItem {
public:
    QueryResultItem() = default;
    explicit QueryResultItem(const nlohmann::json& j) : data_(j) {}
    std::string json_string() const { return data_.dump(); }
private:
    nlohmann::json data_;
};

// Query result
class QueryResult {
public:
    QueryResult() = default;
    explicit QueryResult(const std::vector<nlohmann::json>& items) {
        for (const auto& item : items) {
            items_.emplace_back(item);
            std::string id = item.value("_id", "");
            if (!id.empty()) {
                mutated_ids_.emplace_back(id);
            }
        }
    }

    size_t item_count() const { return items_.size(); }
    QueryResultItem get_item(size_t index) const {
        if (index < items_.size()) return items_[index];
        return QueryResultItem();
    }

    // For iteration support (used by real SDK)
    const std::vector<QueryResultItem>& items() const { return items_; }

    std::vector<DocumentId> mutated_document_ids() const { return mutated_ids_; }

private:
    std::vector<QueryResultItem> items_;
    std::vector<DocumentId> mutated_ids_;
};

// Store observer
class StoreObserver {
public:
    virtual ~StoreObserver() = default;
};

// Sync subscription
class SyncSubscription {
public:
    void cancel() {}
};

// Sync
class Sync {
public:
    std::shared_ptr<SyncSubscription> register_subscription(const std::string& query) {
        return std::make_shared<SyncSubscription>();
    }
};

// Store
class Store {
public:
    QueryResult execute(const std::string& query) {
        auto& store = InMemoryStore::instance();
        std::string collection = store.extract_collection(query);

        // Handle EVICT (clear collection)
        if (query.find("EVICT") != std::string::npos && !collection.empty()) {
            store.clear_collection(collection);
            return QueryResult();
        }

        if (!collection.empty()) {
            auto results = store.query(collection);
            return QueryResult(results);
        }
        return QueryResult();
    }

    // Support initializer list with string keys (matches real SDK API)
    QueryResult execute(const std::string& query,
                        std::initializer_list<std::pair<std::string, nlohmann::json>> args) {
        std::map<std::string, nlohmann::json> args_map;
        for (const auto& [k, v] : args) {
            args_map[k] = v;
        }
        return execute(query, args_map);
    }

    QueryResult execute(const std::string& query,
                        const std::map<std::string, nlohmann::json>& args) {
        auto& store = InMemoryStore::instance();
        std::string collection = store.extract_collection(query);

        // Handle INSERT
        if (query.find("INSERT") != std::string::npos) {
            // Find the document argument (could be named anything)
            for (const auto& [key, value] : args) {
                if (value.is_object()) {
                    nlohmann::json doc = value;
                    // Generate ID if not present
                    if (!doc.contains("_id") || doc["_id"].get<std::string>().empty()) {
                        doc["_id"] = DocumentId().to_string();
                    }
                    store.insert(collection, doc);
                    return QueryResult({doc});
                }
            }
        }

        // Handle UPDATE
        if (query.find("UPDATE") != std::string::npos) {
            // Parse SET clause to extract field updates
            // Format: UPDATE collection SET field = :param, ... WHERE _id = :id
            std::map<std::string, nlohmann::json> updates;

            // Extract SET values from args
            for (const auto& [key, value] : args) {
                if (key == "id") continue; // Skip the WHERE id param
                updates[key] = value;
            }

            // Find the target document by id
            std::string target_id;
            auto id_it = args.find("id");
            if (id_it != args.end()) {
                target_id = id_it->second.get<std::string>();
            }

            // Update the document in the store
            if (!target_id.empty() && !updates.empty()) {
                store.update_document(collection, target_id, updates);
            }

            return QueryResult();
        }

        // Handle SELECT
        if (!collection.empty()) {
            auto results = store.query(collection);
            return QueryResult(results);
        }

        return QueryResult();
    }

    std::shared_ptr<StoreObserver> register_observer(
        const std::string& query,
        std::function<void(const QueryResult&)> callback) {

        auto& store = InMemoryStore::instance();
        std::string collection = store.extract_collection(query);

        if (!collection.empty()) {
            store.register_observer(collection,
                [callback](const std::vector<nlohmann::json>& data) {
                    callback(QueryResult(data));
                });
        }

        return std::make_shared<StoreObserver>();
    }

    std::shared_ptr<SyncSubscription> register_subscription(const std::string& query) {
        return std::make_shared<SyncSubscription>();
    }
};

// Main Ditto class
class Ditto {
public:
    Ditto(const Identity& identity, const std::string& persistence_dir = "") {
        store_ = std::make_shared<Store>();
        sync_ = std::make_shared<Sync>();
    }

    void update_transport_config(std::function<void(TransportConfig&)> updater) {
        TransportConfig config;
        updater(config);
    }

    void disable_sync_with_v3() {}

    void start_sync() { sync_active_ = true; }
    void stop_sync() { sync_active_ = false; }

    bool get_is_sync_active() const { return sync_active_; }

    Store& store() { return *store_; }
    Store& get_store() { return *store_; }
    Sync& get_sync() { return *sync_; }

    static std::string get_sdk_version() { return "stub-1.0.0 (demo mode)"; }

private:
    std::shared_ptr<Store> store_;
    std::shared_ptr<Sync> sync_;
    bool sync_active_ = false;
};

} // namespace ditto

#endif // DITTO_STUB_H
