# Phase 3: Sync Bridge and POS UI - Research

**Researched:** 2026-03-04
**Domain:** Ditto DQL observers/subscriptions, Firestore SnapshotListeners, coroutine bridge patterns, Jetpack Compose Navigation, Material 3
**Confidence:** HIGH

---

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

**POS screen layout:**
- Product catalog displays as a 2-column grid of cards — product name, price, category badge on each card
- Category tabs at top for filtering: All | Beverages | Food | Merchandise
- Each product card shows current inventory count (e.g., "12 in stock")
- Orders list screen shows all orders from all terminals (not filtered by device)

**Order creation flow:**
- Tap product card to add 1 unit to current order; tap again to increment quantity
- Sticky bottom bar shows running total: "3 items — $14.97 [Submit Order]" — collapses when cart is empty
- After submit: brief snackbar "Order #1234 placed", cart clears, stay on catalog screen
- Orders are immutable after submission — only status progresses (OPEN -> FULFILLED)
- Inventory decrements immediately on order submission (Ditto CRDT counter + Firestore FieldValue.increment())

**Navigation structure:**
- Bottom navigation bar with 3 tabs: Catalog, Orders, Status
- StatusScreen from Phase 1 becomes the Status tab
- Material 3 NavigationBar component with Navigation Compose for routing
- Catalog tab is the start destination

**Bridge error visibility:**
- Phase 3 logs sync errors to Logcat only — no UI error indicators (Phase 4 scope)
- Bridge auto-retries Firestore write failures with exponential backoff (3 retries)
- Bridge pauses gracefully when Firebase is unreachable, resumes on reconnect (SYNC-04)

**Sync bridge behavior:**
- UI reads exclusively from Ditto local store — never from Firestore directly
- SyncSource tagging on all bridge-written docs prevents infinite sync loops
- Bridge checks `syncSource` field before writing: if doc was written by this bridge direction, skip it
- `ditto.startSync()` called in this phase after subscriptions are registered
- Ditto `registerSubscription` active for all 3 collections

### Claude's Discretion
- Exact bridge class architecture (single BridgeManager vs per-collection bridges)
- Coroutine scope/dispatcher strategy for bridge observers
- ChangeGuard implementation details (how to handle `hasPendingWrites` on Firestore side)
- DQL upsert syntax validation (flagged in STATE.md as needing live validation)
- Firestore connection state detection approach (`.info/connected` workaround)
- ViewModel architecture for each screen
- Compose navigation route naming
- Product card visual design details (spacing, colors, typography)

### Deferred Ideas (OUT OF SCOPE)
None — discussion stayed within phase scope
</user_constraints>

---

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|-----------------|
| SYNC-01 | Firebase-to-Ditto bridge — Firestore SnapshotListener writes changes into Ditto local store for all 3 collections | Firestore addSnapshotListener + ChangeGuard hasPendingWrites filter + Ditto INSERT INTO ... ON ID CONFLICT DO UPDATE |
| SYNC-02 | Ditto-to-Firebase bridge — Ditto registerObserver triggers Firestore writes for all 3 collections | ditto.store.registerObserver with DQL SELECT, syncSource field skip guard, Firestore set()/update() |
| SYNC-03 | ChangeGuard preventing infinite sync loops (hasPendingWrites on Firestore side, syncSource skip on Ditto side) | hasPendingWrites confirmed available on DocumentSnapshot; syncSource string field checked before write |
| SYNC-04 | Bridge pauses gracefully when Firebase is unreachable and resumes on reconnect | Firebase RTDB .info/connected listener confirmed approach; Firestore has no native connection callback |
| SYNC-05 | Ditto registerSubscription active for all 3 collections to pull data from P2P peers | ditto.sync.registerSubscription(query) documented; call before startSync() |
| POSU-01 | Product catalog screen displaying products from Ditto local store | ViewModel collects StateFlow from registerObserver; LazyVerticalGrid for 2-col grid |
| POSU-02 | Create order screen — user can select products, see running total, and submit order | Cart state in ViewModel; bottom bar driven by derivedStateOf; snackbar via SnackbarHostState |
| POSU-03 | Inventory decrements when order is placed | Ditto COUNTER INCREMENT BY -qty via DQL UPDATE; Firestore FieldValue.increment(-qty) |
| POSU-04 | Orders list screen showing all orders with status | registerObserver SELECT * FROM orders ORDER BY timestamp DESC; LazyColumn |
| POSU-05 | Jetpack Compose UI consistent with existing quickstart patterns (MVVM + ViewModels) | koinViewModel() injection; StateFlow<UiState> in ViewModel; collectAsStateWithLifecycle() in screens |
</phase_requirements>

---

## Summary

Phase 3 has three distinct sub-problems that must be wired together: the Firebase-to-Ditto bridge (cloud -> local store), the Ditto-to-Firebase bridge (local store -> cloud), and the POS UI that reads exclusively from the Ditto local store. The bridges are the architectural core — if the ChangeGuard loop prevention fails, every write cascades into infinite writes. If the wrong coroutine dispatcher is used in bridge observers, the app ANRs.

The DQL upsert syntax has been validated: `INSERT INTO collection DOCUMENTS (:doc) ON ID CONFLICT DO UPDATE` is correct for Ditto 4.14.x. There is no `SET` clause — the full document is replaced on conflict. Use `DO UPDATE_LOCAL_DIFF` when you only want changed fields updated (preferred for bridge writes to avoid spurious CRDT mutations). The Firestore-to-Ditto direction uses this upsert; the Ditto-to-Firebase direction uses Firestore `set()` with `SetOptions.merge()`.

Firestore has no native connection state API. The confirmed workaround is adding a Firebase Realtime Database listener to `.info/connected` — this is what the official Firebase presence guide uses. The RTDB listener disconnects after 60 seconds of inactivity, so a `keepSynced(true)` call on a dummy ref is needed to keep it alive. For Phase 3 purposes (bridge pauses gracefully), the RTDB `.info/connected` boolean is sufficient.

**Primary recommendation:** Use a single `SyncBridgeManager` singleton (Koin `single {}`) that owns all six listener handles (3 Firestore SnapshotListeners + 3 Ditto observers). Start it in `PosApplication.onCreate()` after `ditto.startSync()`. Use `Dispatchers.IO` for all bridge coroutine work.

---

## Standard Stack

### Core
| Library | Version | Purpose | Why Standard |
|---------|---------|---------|--------------|
| Ditto SDK (Kotlin) | 4.14.3 | Local store, P2P sync, DQL | Already in project (Phase 1) |
| Firebase Firestore | BoM 34.x (managed) | Cloud source of truth, SnapshotListeners | Already in project (Phase 1) |
| Firebase Realtime Database | BoM 34.x | `.info/connected` connection detection | Required for SYNC-04 — Firestore has no native connection callback |
| Navigation Compose | 2.7.x | Bottom nav + multi-screen routing | Standard Jetpack for Compose |
| Material 3 | bundled with Compose | NavigationBar, Card, Snackbar | Already in project via PosTheme |
| kotlinx-coroutines-play-services | 1.7.x | Firestore `.await()` extension | Already in project (Phase 2) |

### Supporting
| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| Koin | 3.5.x | DI for bridge manager, ViewModels | Already in project — use koinViewModel() in screens |
| Lifecycle ViewModel | 2.7.x | MVVM pattern, coroutine scope | viewModelScope for bridge-triggered UI updates |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| Single BridgeManager | Per-collection bridge classes | Per-collection is more testable but adds 6 files vs 1; single manager sufficient for demo scope |
| RTDB .info/connected | Custom ping with Firestore write | Official approach is RTDB; custom ping is unreliable and adds latency |
| DO UPDATE_LOCAL_DIFF | DO UPDATE | DO UPDATE touches all fields including unchanged CRDT counters — can cause spurious CRDT mutations |

**New dependency (add to libs.versions.toml):**
```toml
[libraries]
firebase-database = { group = "com.google.firebase", name = "firebase-database" }
```

```kotlin
// build.gradle.kts
implementation(libs.firebase.database)
```

## Architecture Patterns

### Recommended Project Structure
```
app/src/main/java/live/ditto/pubsec/pos/
├── data/
│   ├── model/          # (existing from Phase 2)
│   ├── seed/           # (existing from Phase 2)
│   └── repository/     # NEW: ProductRepository, OrderRepository, InventoryRepository
├── sync/               # NEW: SyncBridgeManager, ChangeGuard
├── ui/
│   ├── catalog/        # NEW: CatalogScreen, CatalogViewModel
│   ├── orders/         # NEW: OrdersScreen, OrdersViewModel
│   └── status/         # (existing StatusScreen — move here or leave in ui/)
└── MainActivity.kt     # Add NavHost + NavigationBar
```

### Pattern 1: ChangeGuard (Loop Prevention)

**What:** Every document written by the bridge carries `syncSource = "firestore"` (when written by Firestore->Ditto direction) or `syncSource = "ditto"` (when written by Ditto->Firestore direction). Each bridge direction checks the incoming document's syncSource and skips if it matches its own tag.

**Firestore -> Ditto direction — skip hasPendingWrites docs:**
```kotlin
// In Firestore SnapshotListener callback
for (change in snapshots.documentChanges) {
    val doc = change.document
    // hasPendingWrites = doc was written locally (by our Ditto->Firebase bridge write)
    // Skip it to avoid echo: Firestore write -> SnapshotListener -> Ditto write -> observer -> Firestore write
    if (doc.metadata.hasPendingWrites) continue
    val syncSource = doc.getString("syncSource") ?: ""
    if (syncSource == "firestore") continue  // already came from Firestore, skip
    writeToLocalDitto(doc)
}
```

**Ditto -> Firestore direction — skip docs tagged as "firestore":**
```kotlin
// In Ditto registerObserver callback
queryResult.use { result ->
    for (item in result.items) {
        val syncSource = item.value["syncSource"] as? String ?: ""
        if (syncSource == "firestore") continue  // written by Firestore->Ditto bridge, skip
        writeToFirestore(item)
    }
}
```

**Confidence:** HIGH — `hasPendingWrites` is documented on `DocumentSnapshot.metadata`. syncSource field skip is the project's established convention (DATA-05, Phase 2).

### Pattern 2: Ditto DQL Upsert (VALIDATED)

**What:** Bridge writes from Firestore into Ditto using INSERT with conflict policy. `DO UPDATE_LOCAL_DIFF` preferred over `DO UPDATE` to avoid touching unchanged CRDT counter fields.

```kotlin
// Source: https://docs.ditto.live/dql/insert
suspend fun upsertToDitto(ditto: Ditto, collectionName: String, doc: Map<String, Any?>) {
    ditto.store.execute(
        """
        INSERT INTO $collectionName
        DOCUMENTS (:doc)
        ON ID CONFLICT DO UPDATE_LOCAL_DIFF
        """.trimIndent(),
        mapOf("doc" to doc)
    )
}
```

**VALIDATED:** `ON ID CONFLICT DO UPDATE` and `ON ID CONFLICT DO UPDATE_LOCAL_DIFF` are confirmed in Ditto docs. There is NO `DO UPDATE SET` clause — the full document is the argument. STATE.md blocker resolved.

**Confidence:** HIGH — verified against https://docs.ditto.live/dql/insert

### Pattern 3: Ditto registerObserver

**What:** Observe a collection for changes. Returns a handle that must be closed when done.

```kotlin
// Source: https://docs.ditto.live/crud/observing-data-changes
val observer = ditto.store.registerObserver(
    "SELECT * FROM products WHERE deleted = false"
) { queryResult ->
    queryResult.use { result ->
        val items = result.items.map { it.value }
        // emit to StateFlow or call bridge write
    }
}
// Store observer handle — close in onCleared() or bridge teardown
observer.close()
```

**Dispatcher note:** The registerObserver callback fires on a Ditto internal thread. If you call `runBlocking` inside the lambda, you risk deadlock. Always launch a coroutine from a captured `CoroutineScope` instead:

```kotlin
// Correct pattern — capture scope, launch from callback
private val bridgeScope = CoroutineScope(SupervisorJob() + Dispatchers.IO)

val observer = ditto.store.registerObserver("SELECT * FROM products") { queryResult ->
    val snapshot = queryResult.use { it.items.map { item -> item.value } }
    bridgeScope.launch {
        writeSnapshotToFirestore(snapshot)
    }
}
```

**STATE.md blocker resolved:** Never use `runBlocking` in the observer lambda. Capture a `CoroutineScope(Dispatchers.IO)` and launch from it.

**Confidence:** HIGH — pattern confirmed from Ditto docs; ANR risk from `runBlocking` is standard Android coroutines guidance.

### Pattern 4: Ditto registerSubscription

**What:** Tells Ditto what data to pull from P2P peers. Must be called before `startSync()`.

```kotlin
// Register subscriptions for all 3 collections
val productsSub = ditto.sync.registerSubscription("SELECT * FROM products WHERE deleted = false")
val ordersSub = ditto.sync.registerSubscription("SELECT * FROM orders")
val inventorySub = ditto.sync.registerSubscription("SELECT * FROM inventory")

// Then start sync
ditto.startSync()
```

**Confidence:** MEDIUM — registerSubscription API shape confirmed from docs references; exact method path `ditto.sync.registerSubscription` consistent with Ditto 4.x architecture. Validate against actual SDK during implementation.

### Pattern 5: Firestore Connection Detection (VALIDATED)

**What:** Firestore has no native connection state callback. Use Firebase Realtime Database `.info/connected`.

```kotlin
// Source: https://firebase.google.com/docs/firestore/solutions/presence
// STATE.md blocker resolved — this is the confirmed Firebase approach

val database = Firebase.database
val connectedRef = database.getReference(".info/connected")

// Keep connection alive (RTDB disconnects after 60s inactivity)
database.getReference("keepAlive").keepSynced(true)

connectedRef.addValueEventListener(object : ValueEventListener {
    override fun onDataChange(snapshot: DataSnapshot) {
        val connected = snapshot.getValue(Boolean::class.java) ?: false
        _isFirebaseConnected.value = connected
        if (connected) {
            bridgeScope.launch { flushPendingWrites() }
        }
    }
    override fun onCancelled(error: DatabaseError) {
        Log.e("Bridge", "RTDB listener cancelled: ${error.message}")
    }
})
```

**Known limitation:** `.info/connected` reflects RTDB connectivity, not Firestore connectivity specifically. In practice they track the same underlying network; acceptable for Phase 3 demo purposes.

**Confidence:** HIGH — confirmed from Firebase official presence guide (https://firebase.google.com/docs/firestore/solutions/presence) and GitHub issue #947.

### Pattern 6: Compose Navigation with Bottom Bar

**What:** Material 3 NavigationBar + Navigation Compose for 3-tab POS layout.

```kotlin
// In MainActivity.kt — replace existing setContent
val navController = rememberNavController()
val currentBackStack by navController.currentBackStackEntryAsState()
val currentRoute = currentBackStack?.destination?.route

Scaffold(
    bottomBar = {
        NavigationBar {
            NavigationBarItem(
                selected = currentRoute == "catalog",
                onClick = { navController.navigate("catalog") { launchSingleTop = true } },
                icon = { Icon(Icons.Default.ShoppingCart, contentDescription = null) },
                label = { Text("Catalog") }
            )
            NavigationBarItem(
                selected = currentRoute == "orders",
                onClick = { navController.navigate("orders") { launchSingleTop = true } },
                icon = { Icon(Icons.Default.List, contentDescription = null) },
                label = { Text("Orders") }
            )
            NavigationBarItem(
                selected = currentRoute == "status",
                onClick = { navController.navigate("status") { launchSingleTop = true } },
                icon = { Icon(Icons.Default.Info, contentDescription = null) },
                label = { Text("Status") }
            )
        }
    }
) { paddingValues ->
    NavHost(navController, startDestination = "catalog", modifier = Modifier.padding(paddingValues)) {
        composable("catalog") { CatalogScreen() }
        composable("orders") { OrdersScreen() }
        composable("status") { StatusScreen() }
    }
}
```

**Confidence:** HIGH — standard Navigation Compose + Material 3 pattern.

### Pattern 7: ViewModel with Ditto StateFlow

**What:** ViewModel exposes StateFlow derived from Ditto observer; screen collects with lifecycle-awareness.

```kotlin
class CatalogViewModel(private val ditto: Ditto) : ViewModel() {
    private val _products = MutableStateFlow<List<Product>>(emptyList())
    val products: StateFlow<List<Product>> = _products.asStateFlow()

    // Cart state
    private val _cart = MutableStateFlow<Map<String, Int>>(emptyMap()) // productId -> qty
    val cart: StateFlow<Map<String, Int>> = _cart.asStateFlow()

    private var observer: DittoStoreObserver? = null

    init {
        observer = ditto.store.registerObserver(
            "SELECT * FROM ${Collections.PRODUCTS} WHERE ${Collections.Fields.DELETED} = false"
        ) { queryResult ->
            val items = queryResult.use { it.items.map { item ->
                Product.fromMap(item.value)
            }}
            _products.value = items
        }
    }

    fun addToCart(productId: String) {
        _cart.update { current ->
            current.toMutableMap().apply { this[productId] = (this[productId] ?: 0) + 1 }
        }
    }

    fun submitOrder(products: List<Product>) {
        viewModelScope.launch(Dispatchers.IO) {
            // 1. Build Order data class
            // 2. INSERT INTO orders DOCUMENTS (:order) ON ID CONFLICT DO NOTHING
            // 3. UPDATE inventory via CRDT counter for each item
            // 4. Clear cart
            _cart.value = emptyMap()
        }
    }

    override fun onCleared() {
        observer?.close()
        super.onCleared()
    }
}
```

**In Compose screen:**
```kotlin
@Composable
fun CatalogScreen(vm: CatalogViewModel = koinViewModel()) {
    val products by vm.products.collectAsStateWithLifecycle()
    val cart by vm.cart.collectAsStateWithLifecycle()
    // ...
}
```

**Confidence:** HIGH — standard MVVM + Compose pattern, consistent with project conventions.

### Pattern 8: Inventory CRDT Decrement

**What:** Ditto-side decrement uses DQL UPDATE with INCREMENT BY (negative value). Firestore uses FieldValue.increment().

```kotlin
// Ditto CRDT counter decrement
ditto.store.execute(
    """
    UPDATE ${Collections.INVENTORY}
    SET quantity = INCREMENT(-:qty)
    WHERE _id = :inventoryId
    """.trimIndent(),
    mapOf("qty" to quantity, "inventoryId" to inventoryId)
)

// Firestore atomic decrement (equivalent)
firestore.collection(Collections.INVENTORY)
    .document(inventoryId)
    .update(Collections.Fields.QUANTITY, FieldValue.increment(-quantity.toLong()))
    .await()
```

**Confidence:** MEDIUM — INCREMENT BY negative is the Ditto CRDT counter convention; verify exact DQL syntax `INCREMENT(-:qty)` vs `INCREMENT BY -:qty` against SDK during implementation. The Phase 2 research confirmed COUNTER type works this way.

### Anti-Patterns to Avoid

- **runBlocking in Ditto observer lambda:** Causes ANR. Launch coroutines from a captured scope instead.
- **DO UPDATE instead of DO UPDATE_LOCAL_DIFF for bridge writes:** DO UPDATE touches all fields, triggering unnecessary CRDT mutations on unchanged counters.
- **Reading from Firestore in ViewModels:** All UI reads go through Ditto. Firestore reads in ViewModels break the offline-first guarantee.
- **Registering subscriptions after startSync():** Subscriptions must be registered before `ditto.startSync()` or data won't sync.
- **Not closing observer handles:** Memory leak and continued callbacks after ViewModel destroyed. Close in `onCleared()`.
- **Firestore offline persistence:** Project decision (FOUN-02) is persistence disabled. Don't re-enable it — Ditto is the offline store.

---

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Sync loop prevention | Custom write-locking scheme | hasPendingWrites + syncSource field | Race conditions in custom locking; metadata.hasPendingWrites is atomic per Firestore |
| Firestore connection events | Custom HTTP ping | Firebase RTDB .info/connected | Official API, handles reconnect automatically |
| Bottom nav routing | Manual back stack | Navigation Compose | Handles back stack, deep links, animation |
| Coroutine lifecycle | Manual thread management | viewModelScope + Dispatchers.IO | Automatic cancellation on ViewModel clear |
| Exponential backoff | Custom retry loop | kotlinx.coroutines retry pattern or manual 3-attempt loop | Simple 3-retry with delay is fine at this scope |

---

## Common Pitfalls

### Pitfall 1: Sync Loop via Missing ChangeGuard
**What goes wrong:** Bridge writes document to Ditto -> Ditto observer fires -> bridge writes to Firestore -> Firestore SnapshotListener fires -> bridge writes to Ditto -> infinite loop.
**Why it happens:** Observer and listener are always-on; without filtering they see their own writes.
**How to avoid:** Check `metadata.hasPendingWrites` in Firestore listener (skip locally-written docs). Check `syncSource` field in Ditto observer (skip docs tagged with the other system's source string).
**Warning signs:** Logcat shows rapid-fire bridge write messages; Firestore write count spikes in Firebase console.

### Pitfall 2: ANR from runBlocking in Observer
**What goes wrong:** `runBlocking { writeToFirestore() }` inside a Ditto observer lambda blocks the Ditto internal thread.
**Why it happens:** Ditto callbacks run on an internal thread; runBlocking ties up that thread waiting for a coroutine.
**How to avoid:** Capture `CoroutineScope(SupervisorJob() + Dispatchers.IO)` in BridgeManager. Call `bridgeScope.launch { }` from within the lambda.
**Warning signs:** ANR dialog after first sync event; "Application Not Responding" in Logcat.

### Pitfall 3: RTDB keepSynced Not Called
**What goes wrong:** `.info/connected` listener stops firing after 60 seconds of no other RTDB activity.
**Why it happens:** RTDB disconnects from backend after 60s inactivity unless a ref has keepSynced(true).
**How to avoid:** Call `database.getReference("keepAlive").keepSynced(true)` once at bridge startup.
**Warning signs:** Bridge appears stuck even though network is available; connection toggles stop being detected after first minute.

### Pitfall 4: Subscriptions Registered After startSync
**What goes wrong:** Ditto starts sync before subscriptions are registered — peers don't know what data to replicate.
**Why it happens:** Lifecycle ordering mistake (startSync in Application.onCreate before BridgeManager init).
**How to avoid:** Call `registerSubscription` for all 3 collections, then `ditto.startSync()`. BridgeManager.start() should encapsulate both.
**Warning signs:** Two terminals on same LAN don't replicate — peer count > 0 but no data appears.

### Pitfall 5: DO UPDATE Touching CRDT Counters
**What goes wrong:** Bridge upsert with `DO UPDATE` overwrites inventory counter CRDT state, losing concurrent offline decrements.
**Why it happens:** `DO UPDATE` replaces the whole document field, including CRDT counter, with a plain integer.
**How to avoid:** Use `DO UPDATE_LOCAL_DIFF` for bridge upserts. Better yet, don't include the CRDT `quantity` field in the Firestore->Ditto bridge write at all — let the separate inventory decrement path own it.
**Warning signs:** Inventory count jumps back to old value after reconnect.

---

## Code Examples

### SyncBridgeManager skeleton
```kotlin
// Source: patterns validated from docs.ditto.live + Firebase presence guide
class SyncBridgeManager(
    private val ditto: Ditto,
    private val firestore: FirebaseFirestore
) {
    private val bridgeScope = CoroutineScope(SupervisorJob() + Dispatchers.IO)
    private val listeners = mutableListOf<ListenerRegistration>()
    private val observers = mutableListOf<DittoStoreObserver>()

    fun start() {
        setupSubscriptions()
        ditto.startSync()
        setupFirestoreToDittoListeners()
        setupDittoToFirestoreObservers()
        setupConnectionDetection()
    }

    fun stop() {
        listeners.forEach { it.remove() }
        observers.forEach { it.close() }
        bridgeScope.cancel()
    }

    private fun setupSubscriptions() {
        observers += ditto.sync.registerSubscription("SELECT * FROM products")
        observers += ditto.sync.registerSubscription("SELECT * FROM orders")
        observers += ditto.sync.registerSubscription("SELECT * FROM inventory")
    }

    private fun setupFirestoreToDittoListeners() {
        val listener = firestore.collection(Collections.PRODUCTS)
            .addSnapshotListener { snapshots, error ->
                if (error != null || snapshots == null) return@addSnapshotListener
                for (change in snapshots.documentChanges) {
                    val doc = change.document
                    if (doc.metadata.hasPendingWrites) continue
                    val data = doc.data.toMutableMap()
                    data["_id"] = doc.id
                    data["syncSource"] = "firestore"
                    bridgeScope.launch {
                        ditto.store.execute(
                            "INSERT INTO products DOCUMENTS (:doc) ON ID CONFLICT DO UPDATE_LOCAL_DIFF",
                            mapOf("doc" to data)
                        )
                    }
                }
            }
        listeners += listener
        // Repeat pattern for orders, inventory collections
    }

    private fun setupDittoToFirestoreObservers() {
        val observer = ditto.store.registerObserver(
            "SELECT * FROM ${Collections.PRODUCTS} WHERE syncSource != 'firestore'"
        ) { queryResult ->
            val items = queryResult.use { it.items.map { item -> item.value.toMutableMap() } }
            bridgeScope.launch {
                for (doc in items) {
                    val id = doc["_id"] as? String ?: continue
                    doc["syncSource"] = "ditto"
                    firestore.collection(Collections.PRODUCTS)
                        .document(id)
                        .set(doc, SetOptions.merge())
                        .await()
                }
            }
        }
        observers += observer
        // Repeat for orders, inventory
    }
}
```

### Koin registration
```kotlin
// In di/RepositoryModule.kt (or new SyncModule.kt)
val syncModule = module {
    single { SyncBridgeManager(get(), get()) }
}

// In PosApplication.kt — start after Koin initialized
get<SyncBridgeManager>().start()
```

---

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| Ditto legacy query API | DQL (SQL-like) | Ditto 4.x | INSERT/SELECT/UPDATE syntax replaces store.write{} |
| firebase-firestore-ktx | Bundled in firebase-firestore since BoM 34.0.0 | BoM 34.0.0 | No separate -ktx dependency needed |
| Manual observer lifecycle | close() on returned handle | Current | Store handle reference, call close() in teardown |

**Deprecated/outdated:**
- `ditto.store.write { }` legacy API: replaced by `ditto.store.execute(dqlString)` in Ditto 4.x
- `firebase-firestore-ktx` artifact: retired in BoM 34.0.0, bundled into main artifact

---

## Open Questions

1. **Exact DQL syntax for CRDT INCREMENT**
   - What we know: Ditto supports COUNTER type with INCREMENT; Phase 2 research confirms it works
   - What's unclear: Whether DQL syntax is `INCREMENT(-:qty)` or `INCREMENT BY -:qty` in 4.14.3
   - Recommendation: Test during implementation task; fall back to `INCREMENT(-1)` literal if parameterized form fails

2. **registerSubscription return type**
   - What we know: Method exists on `ditto.sync`; returns a handle to close
   - What's unclear: Whether the return type is `DittoSyncSubscription` or another type name — affects type annotation in BridgeManager
   - Recommendation: Let IDE infer type on first use; store as `Any` in list if heterogeneous

3. **Firestore listener accuracy for Phase 3 offline scenario**
   - What we know: `.info/connected` reflects RTDB not Firestore connectivity specifically
   - What's unclear: Whether they diverge in real demo conditions (WiFi up, Firebase partially blocked)
   - Recommendation: Acceptable for Phase 3; Phase 4 can add more precise detection if demo shows divergence

---

## Validation Architecture

### Test Framework
| Property | Value |
|----------|-------|
| Framework | JUnit 4 + MockK (established in Phase 1-2) |
| Config file | `app/build.gradle.kts` (testImplementation declarations) |
| Quick run command | `./gradlew test` |
| Full suite command | `./gradlew test connectedAndroidTest` |

### Phase Requirements -> Test Map
| Req ID | Behavior | Test Type | Automated Command | File Exists? |
|--------|----------|-----------|-------------------|-------------|
| SYNC-01 | Firestore SnapshotListener writes to Ditto local store | unit | `./gradlew test --tests "*SyncBridgeManagerTest.firestoreToDittoWritesDocument"` | Wave 0 |
| SYNC-02 | Ditto observer triggers Firestore write | unit | `./gradlew test --tests "*SyncBridgeManagerTest.dittoToFirestoreWritesDocument"` | Wave 0 |
| SYNC-03 | ChangeGuard skips hasPendingWrites docs and syncSource-tagged docs | unit | `./gradlew test --tests "*SyncBridgeManagerTest.changeGuardSkipsPendingWrites"` | Wave 0 |
| SYNC-04 | Bridge pauses when Firebase unreachable | unit (mockk RTDB) | `./gradlew test --tests "*SyncBridgeManagerTest.bridgePausesOnDisconnect"` | Wave 0 |
| SYNC-05 | registerSubscription called for all 3 collections | unit | `./gradlew test --tests "*SyncBridgeManagerTest.subscriptionsRegisteredBeforeStartSync"` | Wave 0 |
| POSU-01 | CatalogViewModel emits products from Ditto | unit | `./gradlew test --tests "*CatalogViewModelTest.productsFlowEmitsFromDitto"` | Wave 0 |
| POSU-02 | Cart total calculates correctly; submit clears cart | unit | `./gradlew test --tests "*CatalogViewModelTest.submitOrderClearsCart"` | Wave 0 |
| POSU-03 | Inventory decrements on order submit | unit | `./gradlew test --tests "*CatalogViewModelTest.inventoryDecrementsOnSubmit"` | Wave 0 |
| POSU-04 | OrdersViewModel emits all orders from Ditto | unit | `./gradlew test --tests "*OrdersViewModelTest.ordersFlowEmitsAll"` | Wave 0 |
| POSU-05 | Compose screens render without crash | smoke | `./gradlew connectedAndroidTest --tests "*CatalogScreenTest"` | Wave 0 |

### Sampling Rate
- **Per task commit:** `./gradlew test`
- **Per wave merge:** `./gradlew test`
- **Phase gate:** Full suite green before `/gsd:verify-work`

### Wave 0 Gaps
- [ ] `app/src/test/java/live/ditto/pubsec/pos/sync/SyncBridgeManagerTest.kt` — covers SYNC-01 through SYNC-05
- [ ] `app/src/test/java/live/ditto/pubsec/pos/ui/catalog/CatalogViewModelTest.kt` — covers POSU-01, 02, 03
- [ ] `app/src/test/java/live/ditto/pubsec/pos/ui/orders/OrdersViewModelTest.kt` — covers POSU-04
- [ ] `app/src/androidTest/java/live/ditto/pubsec/pos/ui/CatalogScreenTest.kt` — covers POSU-05 (smoke)

---

## Sources

### Primary (HIGH confidence)
- https://docs.ditto.live/dql/insert — INSERT INTO ... ON ID CONFLICT DO UPDATE / DO UPDATE_LOCAL_DIFF syntax validated
- https://docs.ditto.live/crud/observing-data-changes — registerObserver Kotlin API, close() lifecycle
- https://firebase.google.com/docs/firestore/solutions/presence — Official Firebase presence guide, RTDB .info/connected workaround

### Secondary (MEDIUM confidence)
- https://github.com/firebase/firebase-android-sdk/issues/947 — Firestore connection state listener GitHub issue confirming no native API
- Phase 2 RESEARCH.md — COUNTER type, DO UPDATE semantics, existing stack versions

### Tertiary (LOW confidence)
- WebSearch results on RTDB keepSynced(true) for 60-second disconnect behavior — needs verification against Firebase Android SDK docs

---

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH — all libraries already in project from Phases 1-2; RTDB is confirmed required addition
- Architecture (ChangeGuard): HIGH — hasPendingWrites + syncSource pattern validated from official docs
- DQL upsert syntax: HIGH — validated against docs.ditto.live/dql/insert (STATE.md blocker resolved)
- Firestore connection detection: HIGH — RTDB .info/connected confirmed official approach (STATE.md blocker resolved)
- Coroutine dispatcher model: HIGH — bridgeScope.launch pattern resolves ANR risk (STATE.md blocker resolved)
- CRDT INCREMENT DQL syntax: MEDIUM — exact parameterized form needs runtime validation

**Research date:** 2026-03-04
**Valid until:** 2026-04-04 (stable APIs; Ditto 4.14.x and Firebase BoM 34.x unlikely to break within 30 days)
