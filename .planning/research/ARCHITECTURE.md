# Architecture Research

**Domain:** Firebase-Ditto bidirectional sync bridge — Android POS application
**Researched:** 2026-03-03
**Confidence:** MEDIUM — Core patterns are well-established; Firebase-Ditto specific bridge patterns synthesized from first principles since no prior art exists for this exact combination.

## Standard Architecture

### System Overview

```
┌──────────────────────────────────────────────────────────────────────┐
│                         CLOUD LAYER                                  │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │                    Firebase Firestore                         │    │
│  │          (products, orders, inventory collections)            │    │
│  └─────────────────────────────────────────────────────────────┘    │
└──────────────────────────────────────┬───────────────────────────────┘
                                       │ Firestore SDK (WebSocket/gRPC)
                                       │ addSnapshotListener (cloud → device)
                                       │ set/update (device → cloud)
┌──────────────────────────────────────▼───────────────────────────────┐
│                         BRIDGE LAYER                                 │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │                     SyncBridgeService                         │    │
│  │                                                               │    │
│  │  FirebaseListener ──→ [transform] ──→ DittoWriter             │    │
│  │  DittoObserver    ──→ [transform] ──→ FirebaseWriter          │    │
│  │                                                               │    │
│  │  ChangeGuard: prevents re-sync of already-propagated writes  │    │
│  └──────────┬────────────────────────────────────┬─────────────┘    │
└─────────────┼────────────────────────────────────┼──────────────────┘
              │ DQL (INSERT/UPDATE/SELECT)           │ registerObserver
              ▼                                      ▼
┌──────────────────────────────────────────────────────────────────────┐
│                         LOCAL/EDGE LAYER                             │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │                     Ditto Local Store                         │    │
│  │          (products, orders, inventory collections)            │    │
│  └─────────────────────────────────────────────────────────────┘    │
│                                                                      │
│  ◄──── Ditto P2P Mesh Sync (Bluetooth LE, P2P Wi-Fi, LAN) ─────►   │
│         (other Android POS terminals on the same mesh)               │
└──────────────────────────────────────┬───────────────────────────────┘
                                       │ StateFlow / LiveData
┌──────────────────────────────────────▼───────────────────────────────┐
│                         UI LAYER (Jetpack Compose)                   │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐               │
│  │  Products    │  │   Orders     │  │  Inventory   │               │
│  │  ViewModel   │  │  ViewModel   │  │  ViewModel   │               │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘               │
│         │                 │                  │                       │
│  ┌──────▼─────────────────▼──────────────────▼──────┐               │
│  │              Compose Screen Layer                  │               │
│  │    ProductsScreen / OrdersScreen / InventoryScreen │               │
│  │         + ConnectivityStatusBar                    │               │
│  └────────────────────────────────────────────────────┘               │
└──────────────────────────────────────────────────────────────────────┘
```

### Component Responsibilities

| Component | Responsibility | Typical Implementation |
|-----------|----------------|------------------------|
| `SyncBridgeService` | Orchestrates bidirectional sync between Firebase and Ditto; owns ChangeGuard | Android `Service` or application-scoped singleton via Hilt |
| `FirebaseListener` | Listens to Firestore `addSnapshotListener`; detects ADDED/MODIFIED/REMOVED changes | Kotlin coroutines wrapping Firestore snapshot callbacks |
| `DittoObserver` | Registers `ditto.store.registerObserver` on each collection; fires on local store changes | Ditto SDK observer, held for app lifetime |
| `FirebaseWriter` | Writes Ditto-originated changes to Firestore (upsert/soft-delete) | Firestore `set()` with merge option or `update()` |
| `DittoWriter` | Writes Firebase-originated changes to local Ditto store using DQL | `ditto.store.execute()` with INSERT/UPDATE |
| `ChangeGuard` | Prevents sync loops by tracking in-flight writes per record | In-memory `Set<String>` of document IDs being synced; cleared after write completes |
| `ProductsRepository` | Exposes `products` as a `StateFlow`; UI-facing read API for Ditto local store | Reads via `registerObserver`, publishes to StateFlow |
| `OrdersRepository` | Same pattern for orders; handles order creation and state transitions | Same pattern |
| `InventoryRepository` | Same pattern for inventory; handles quantity adjustments | Same pattern |
| `*ViewModel` (3x) | Consumes repository flows; exposes UI state; handles user actions | `ViewModel` + `viewModelScope` |
| `ConnectivityMonitor` | Tracks Firebase connection status and Ditto sync status; exposes as StateFlow | `FirebaseFirestore.getInstance().disableNetwork()` + Ditto transport events |
| `PosApplication` | Application subclass; initializes Ditto singleton; wires Hilt graph | `Application` subclass, one Ditto instance per app lifecycle |

## Recommended Project Structure

```
app/src/main/java/live/ditto/pos/
├── PosApplication.kt           # Application subclass; Ditto init; Hilt
├── data/
│   ├── model/                  # Plain Kotlin data classes (no Android deps)
│   │   ├── Product.kt
│   │   ├── Order.kt
│   │   └── InventoryItem.kt
│   ├── ditto/                  # Ditto local store access
│   │   ├── DittoHandler.kt     # Ditto singleton holder (matches existing quickstart pattern)
│   │   ├── ProductsRepository.kt
│   │   ├── OrdersRepository.kt
│   │   └── InventoryRepository.kt
│   └── firebase/               # Firestore remote access
│       ├── FirestoreProducts.kt
│       ├── FirestoreOrders.kt
│       └── FirestoreInventory.kt
├── sync/                       # Bridge — the unique heart of this app
│   ├── SyncBridgeService.kt    # Orchestrator; starts/stops all listeners
│   ├── ChangeGuard.kt          # Sync loop prevention
│   ├── ProductSyncBridge.kt    # Firebase ↔ Ditto bridge for products
│   ├── OrderSyncBridge.kt      # Firebase ↔ Ditto bridge for orders
│   └── InventorySyncBridge.kt  # Firebase ↔ Ditto bridge for inventory
├── ui/
│   ├── products/
│   │   ├── ProductsScreen.kt
│   │   └── ProductsViewModel.kt
│   ├── orders/
│   │   ├── OrdersScreen.kt
│   │   └── OrdersViewModel.kt
│   ├── inventory/
│   │   ├── InventoryScreen.kt
│   │   └── InventoryViewModel.kt
│   └── common/
│       ├── ConnectivityStatusBar.kt   # Firebase + Ditto mesh indicators
│       └── Theme.kt
├── connectivity/
│   └── ConnectivityMonitor.kt         # Observes Firebase + Ditto connection states
└── MainActivity.kt
```

### Structure Rationale

- **data/ditto/ vs data/firebase/:** Separating the two data sources makes it obvious which code touches which system. It also means ChangeGuard can be applied at the sync/ boundary without leaking into data/ logic.
- **sync/:** The bridge is the most novel part of this app. Isolating it here means the rest of the architecture (repositories, ViewModels, UI) is conventional Android — reviewers can focus their attention.
- **data/model/:** Pure Kotlin data classes with no Android or SDK dependencies. Both Ditto and Firebase adapters read from and write to these, which is the mapping layer.
- **connectivity/:** Separate from sync so UI can observe connection state without pulling in sync internals.

## Architectural Patterns

### Pattern 1: ChangeGuard (Sync Loop Prevention)

**What:** A shared, thread-safe set of document IDs currently being propagated. Before writing to system B because of a change from system A, add the ID to the guard. After the write completes, remove it. When a change event arrives for an ID that is already in the guard, skip it — it is the echo of a write this device just made.

**When to use:** Always, in every bidirectional sync bridge. Without it, every write triggers an infinite loop: Firebase change → write to Ditto → Ditto observer fires → write to Firebase → repeat.

**Trade-offs:** Simple and effective for single-device bridge. Does not prevent loops when two different devices are bridging the same data simultaneously — for this demo (single device acts as bridge), it is sufficient.

**Example:**
```kotlin
class ChangeGuard {
    private val inFlight = Collections.synchronizedSet(mutableSetOf<String>())

    fun guard(id: String, block: suspend () -> Unit) {
        if (inFlight.contains(id)) return   // already being synced; skip
        inFlight.add(id)
        try {
            runBlocking { block() }
        } finally {
            inFlight.remove(id)
        }
    }
}
```

### Pattern 2: Unidirectional Data Flow Through Ditto (UI reads only from Ditto)

**What:** The UI always reads from the local Ditto store, never directly from Firestore. Firestore changes flow through the bridge into Ditto, then Ditto's `registerObserver` propagates to the ViewModel. This gives offline-first behavior automatically.

**When to use:** Any app where local availability matters more than cloud freshness. When offline, Ditto continues serving data. When online, Ditto reflects Firebase data within seconds of the bridge propagating it.

**Trade-offs:** The UI is always one "bridge hop" behind Firestore changes, adding ~100-500ms latency (negligible for POS). In exchange, the UI code is simple and identical regardless of connectivity state.

**Example:**
```kotlin
class ProductsRepository @Inject constructor(private val ditto: Ditto) {
    val products: StateFlow<List<Product>> = flow {
        val observer = ditto.store.registerObserver(
            "SELECT * FROM products WHERE NOT deleted ORDER BY name ASC"
        ) { result ->
            val list = result.items.map { Product.fromDittoItem(it) }
            emit(list)
        }
        awaitCancellation()
        observer.close()
    }.stateIn(
        scope = CoroutineScope(Dispatchers.IO),
        started = SharingStarted.WhileSubscribed(5000),
        initialValue = emptyList()
    )
}
```

### Pattern 3: Firestore Snapshot Listener → Ditto Upsert

**What:** A Firestore `addSnapshotListener` fires for ADDED/MODIFIED documents. The bridge maps the Firestore document to a Kotlin model, then writes it to the Ditto local store using DQL `INSERT INTO ... INITIAL DOCUMENTS` (for idempotent upsert) or `UPDATE ... SET`.

**When to use:** Firebase → Ditto direction of the bridge. Fires whenever Firestore connectivity exists and a cloud change arrives.

**Trade-offs:** Firestore's SDK handles reconnection automatically. On reconnect, it re-delivers all changed documents since last connection — the bridge must handle this gracefully (idempotent writes, not duplicate orders). DQL's `INSERT INTO ... INITIAL DOCUMENTS` only inserts if the document does not already exist, so use `UPDATE` for existing documents or a manual upsert.

**Example:**
```kotlin
class ProductSyncBridge @Inject constructor(
    private val firestore: FirebaseFirestore,
    private val ditto: Ditto,
    private val changeGuard: ChangeGuard
) {
    fun startFirebaseToDitto() {
        firestore.collection("products")
            .addSnapshotListener { snapshot, error ->
                if (error != null || snapshot == null) return@addSnapshotListener
                for (change in snapshot.documentChanges) {
                    val id = change.document.id
                    val data = change.document.data
                    changeGuard.guard(id) {
                        when (change.type) {
                            DocumentChange.Type.ADDED, DocumentChange.Type.MODIFIED ->
                                ditto.store.execute(
                                    "INSERT INTO products DOCUMENTS (:doc) ON ID CONFLICT DO UPDATE SET ...",
                                    mapOf("doc" to data + mapOf("_id" to id))
                                )
                            DocumentChange.Type.REMOVED ->
                                ditto.store.execute(
                                    "UPDATE products SET deleted = true WHERE _id = :id",
                                    mapOf("id" to id)
                                )
                        }
                    }
                }
            }
    }
}
```

## Data Flow

### Firebase → Ditto (cloud change reaches a terminal)

```
Firestore cloud write (any client)
    ↓  (WebSocket/gRPC, Firestore SDK delivers snapshot)
FirebaseListener.onSnapshot
    ↓  (documentChanges: ADDED/MODIFIED/REMOVED)
ChangeGuard.guard(id)  ← skip if already in-flight
    ↓
Map Firestore document → Kotlin model → Ditto document map
    ↓
DittoWriter: ditto.store.execute(INSERT or UPDATE)
    ↓
Ditto local store updated
    ↓
registerObserver fires (on all registered observers)
    ↓
ProductsRepository emits updated StateFlow
    ↓
ViewModel collects → UI recomposes
```

### Ditto → Firebase (local/P2P change reaches cloud)

```
User action (UI) or P2P mesh sync delivers change
    ↓
ViewModel calls Repository.create/update/delete
    ↓
Repository: ditto.store.execute(INSERT or UPDATE)
    ↓
Ditto local store updated
    ↓
DittoObserver (registerObserver) fires
    ↓
ChangeGuard.guard(id) ← skip if Firebase already pushed this
    ↓
Map Ditto document → Firestore document map
    ↓
FirebaseWriter: firestore.collection(...).document(id).set(data, SetOptions.merge())
    ↓
Firestore cloud updated → propagates to all other clients
```

### P2P Mesh Sync (Ditto terminal → terminal, no Firebase)

```
Terminal A: user creates order
    ↓
ditto.store.execute(INSERT INTO orders ...)
    ↓
Ditto P2P mesh (Bluetooth LE / Wi-Fi Direct / LAN)
    ↓
Terminal B: registerObserver fires with new order
    ↓
Terminal B UI updates
    ↓  (when either terminal regains internet)
Terminal B's DittoObserver → FirebaseWriter → Firestore updated
```

### Key Data Flows

1. **New order (offline):** User creates order on Terminal A. Written to Ditto local store. Syncs to Terminal B via mesh immediately. Firestore receives the order when any terminal regains internet.

2. **Product catalog update (from back-office):** Back-office admin writes to Firestore. Firestore SDK pushes snapshot to all connected terminals. Bridge writes to Ditto. All terminals (including offline-at-that-moment terminals) receive the update once they connect to at least one bridged terminal via mesh.

3. **Inventory decrement (concurrent):** Two terminals decrement inventory simultaneously. Ditto uses CRDT last-write-wins on numeric fields. Firebase uses last-write-wins via server timestamp. The correct resolution strategy for inventory is server-side: inventory decrements should be written to Firebase as transactions to prevent over-selling. Ditto tracks the optimistic local state for UI responsiveness; Firebase resolves the authoritative count.

## Scaling Considerations

This is a single-store POS demo — scaling is not a primary concern. Practical limits are:

| Scale | Architecture Adjustment |
|-------|--------------------------|
| 1-5 terminals, single location | Default architecture: one bridge per terminal (each terminal runs the bridge independently) |
| 5-20 terminals, single location | Designate one terminal as "bridge terminal"; others are pure Ditto peers. Reduces Firestore read/write cost significantly. |
| Multi-location | Add `locationId` to all documents; scope Ditto subscriptions and Firebase listeners by location. Bridge service filters by location. |

### Scaling Priorities

1. **First bottleneck:** Firestore listener cost. Every terminal running its own bridge listener multiplies Firestore reads by terminal count. Mitigation: designate a bridge terminal per location.
2. **Second bottleneck:** ChangeGuard false positives under high write frequency. Mitigation: use a TTL-based guard instead of a permanent set.

## Anti-Patterns

### Anti-Pattern 1: Bidirectional Sync Without Loop Prevention

**What people do:** Wire a Firestore listener that writes to Ditto, and a Ditto observer that writes to Firebase, with no guard between them.

**Why it's wrong:** Every write loops infinitely. Firebase write triggers Ditto observer → Firebase write → Ditto observer → crash or quota exhaustion within seconds.

**Do this instead:** Always implement ChangeGuard before writing either direction. The guard is the most critical correctness requirement of the entire bridge.

### Anti-Pattern 2: UI Reads From Both Firebase and Ditto

**What people do:** Show product catalog from Firestore, and orders from Ditto, to avoid the bridge complexity.

**Why it's wrong:** When Firebase is unavailable, half the UI breaks. The user cannot browse products to create an offline order. The entire value proposition of the demo (offline-first POS) is broken.

**Do this instead:** UI reads exclusively from Ditto. Firebase data flows through the bridge into Ditto first. Consistent source of truth for the UI regardless of connectivity.

### Anti-Pattern 3: Synchronous Bridge on Main Thread

**What people do:** Register Firestore listeners in an Activity and call Ditto store execute() from the listener callback without switching dispatchers.

**Why it's wrong:** Firestore callbacks arrive on the main thread. `ditto.store.execute()` is a blocking I/O call. This produces ANR (Application Not Responding) errors under load.

**Do this instead:** Switch to `Dispatchers.IO` before calling `ditto.store.execute()`. Use `CoroutineScope(Dispatchers.IO)` in the bridge service, not the main dispatcher.

### Anti-Pattern 4: Storing the Full Firebase Document Schema in Ditto Without Adaptation

**What people do:** Copy Firebase document fields directly into Ditto with no mapping layer.

**Why it's wrong:** Firebase uses string IDs as document keys; Ditto uses `_id`. Firebase timestamps are `Timestamp` objects; Ditto expects ISO-8601 strings or epoch milliseconds. Schema mismatches produce silent failures or type errors in DQL queries.

**Do this instead:** Maintain explicit mapping functions in the model layer (`Product.fromFirestore(doc)`, `Product.toDittoMap()`). Keep the schema contract in one place.

## Integration Points

### External Services

| Service | Integration Pattern | Notes |
|---------|---------------------|-------|
| Firebase Firestore | `addSnapshotListener` for reads; `set(merge=true)` for writes | Firestore SDK handles auth, reconnection, and offline cache automatically. Disable Firestore offline persistence to avoid double-caching with Ditto. |
| Ditto Cloud (BigPeer) | `ditto.startSync()` + `registerSubscription` | Optional for demo; provides cloud backup of Ditto data and cross-network sync. Does not replace Firebase as cloud source of truth in this architecture. |
| Ditto P2P Mesh | Automatic after `ditto.startSync()` — no additional code | Bluetooth LE, Wi-Fi Direct, and LAN transports enabled by default. Requires manifest permissions and runtime permission requests. |

### Internal Boundaries

| Boundary | Communication | Notes |
|----------|---------------|-------|
| SyncBridgeService ↔ Repositories | Bridge writes directly to Ditto store (not via repositories). Repositories observe the store. | Repositories are read-paths for UI. Bridge is the write-path from cloud. Keeping them separate avoids circular dependencies. |
| ViewModel ↔ Repository | `StateFlow` collection in `viewModelScope` | ViewModels never call Ditto or Firestore directly. |
| Repository ↔ Ditto store | `registerObserver` for reads; `ditto.store.execute()` for writes | Both are async. Observers live for the application lifetime; close in a `DisposableHandle` or `onCleared()`. |
| ConnectivityMonitor ↔ UI | `StateFlow<ConnectivityState>` | Exposes Firebase connection status (from Firestore's `addSnapshotListener` metadata) and Ditto sync active status separately. Both signals needed for the demo UI. |

## Build Order Implications

The component dependencies create a natural build order:

1. **Data models first** (`Product`, `Order`, `InventoryItem`) — no dependencies, needed by everything else.
2. **Ditto initialization** (`PosApplication`, `DittoHandler`) — required before any Ditto store access.
3. **Repositories** (Ditto-side read paths) — can be built and tested with mock data before Firebase exists.
4. **ViewModels + UI screens** — consume repositories; can be built with hardcoded Ditto data.
5. **Firebase integration** (`FirestoreProducts` etc.) — add Firestore read/write; still no bridge yet.
6. **ChangeGuard** — small, testable in isolation; must exist before bridge.
7. **SyncBridgeService** — builds on all of the above; the most complex component, built last.
8. **ConnectivityMonitor + status UI** — add last as a demo layer on top of working sync.

## Sources

- Ditto SDK: observer API — https://docs.ditto.live/crud/observing-data-changes (HIGH confidence)
- Ditto SDK: subscription and sync — https://docs.ditto.live/sync/syncing-data (HIGH confidence)
- Ditto SDK: Kotlin install guide and initialization — https://docs.ditto.live/install-guides/kotlin (HIGH confidence)
- Ditto POS/KDS demo app — https://github.com/getditto/demoapp-pos-kds (MEDIUM confidence — P2P patterns, not Firebase bridge)
- Firebase Firestore snapshot listeners — https://firebase.google.com/docs/firestore/query-data/listen (HIGH confidence)
- Bidirectional sync loop prevention patterns — https://www.workato.com/product-hub/how-to-prevent-infinite-loops-in-bi-directional-data-syncs/ (MEDIUM confidence)
- Android offline-first architecture — https://androidengineers.substack.com/p/the-complete-guide-to-offline-first (MEDIUM confidence)
- Existing ditto-quickstart Android/Kotlin patterns (DittoHandler, ViewModel, registerObserver usage) — local codebase (HIGH confidence)

---
*Architecture research for: Firebase-Ditto bidirectional sync bridge, Android POS*
*Researched: 2026-03-03*
