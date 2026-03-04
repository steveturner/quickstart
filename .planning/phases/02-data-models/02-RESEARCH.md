# Phase 2: Data Models - Research

**Researched:** 2026-03-04
**Domain:** Kotlin data classes, Ditto DQL CRDT counters, Firestore document mapping, seed data
**Confidence:** HIGH

---

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

**Field design & types:**
- Monetary values stored as Long cents (e.g., 999 = $9.99) — avoids floating-point rounding, standard POS practice
- Order.items is an embedded `List<OrderLineItem>` with productId, productName, quantity, unitPrice — denormalized for offline reads without product lookups
- Product.category is a plain String field (e.g., "Beverage", "Food", "Merchandise") — no separate category collection
- Order status: simple OPEN/FULFILLED enum — minimal for demo, v2 requirement POLH-03 adds progression later
- All models carry `_id: String` (canonical UUID), `deleted: Boolean`, `syncSource: String`

**Inventory delta events:**
- Use Ditto's built-in CRDT counter type for inventory quantity — concurrent offline decrements merge correctly
- Firestore side uses `FieldValue.increment()` for equivalent atomic decrement
- No separate event log collection needed — CRDT counter handles conflict resolution natively
- InventoryItem tracks `lastModifiedBy: String` (terminalId) for demo visibility

**Seed data catalog:**
- Coffee shop theme: ~8-10 items across Beverages, Food, Merchandise categories
- Seed data lives in a Kotlin `SeedData` object with hardcoded `List<Product>` — no external JSON files
- Seeder class writes to Firestore on first run (checks if collection is empty)
- Each product seeded with initial inventory quantity

**SyncSource tagging:**
- Every model has a `syncSource: String` field with values: "firestore", "ditto", "local"
- String (not enum) for Ditto DQL compatibility — DQL stores/queries strings natively
- Bridge checks syncSource before writing to prevent infinite sync loops
- `lastSyncedAt: Long` (epoch millis) included alongside syncSource for demo debugging

### Claude's Discretion
- Exact product names, prices, and quantities in seed data
- Kotlin data class naming conventions (e.g., `OrderLineItem` vs `LineItem`)
- Whether to use Kotlin value classes for IDs
- UUID generation strategy (UUID.randomUUID() vs other)
- Repository interface design within Koin modules

### Deferred Ideas (OUT OF SCOPE)
None — discussion stayed within phase scope
</user_constraints>

---

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|-----------------|
| DATA-01 | Products collection with canonical UUID IDs shared between Firestore and Ditto (name, price, category, imageUrl) | Kotlin data class pattern established; Ditto INSERT INTO + Firestore set() with manual UUID both support string _id as document ID |
| DATA-02 | Orders collection with canonical UUID IDs shared between Firestore and Ditto (items, total, status, timestamp, terminalId) | Embedded List<OrderLineItem> works in both Firestore (nested map array) and Ditto (nested document array); enum serialized as string |
| DATA-03 | Inventory collection with canonical UUID IDs shared between Firestore and Ditto (productId, quantity) | Ditto COUNTER type (v4.14) handles concurrent decrements; Firestore FieldValue.increment() is the equivalent |
| DATA-04 | Soft delete convention using `deleted` flag (not hard deletes) in both systems | Repo pattern from existing quickstart uses `deleted: Boolean` field; DQL WHERE NOT deleted filters confirmed |
| DATA-05 | SyncSource tagging on all bridge-written documents to prevent sync loops | String field "firestore"/"ditto"/"local" confirmed DQL-compatible; lastSyncedAt: Long as epoch millis |
</phase_requirements>

---

## Summary

Phase 2 creates the Kotlin data class layer, Firestore seed data, and repository stubs that become the schema contract for the Phase 3 sync bridge. Three collections — Products, Orders, InventoryItems — must share identical field names across Ditto and Firestore with no drift. The key technical challenge is the inventory counter: Ditto's `COUNTER` CRDT type (introduced in 4.14, currently in use at 4.14.3) handles concurrent offline decrements via `INCREMENT BY`, while Firestore uses `FieldValue.increment(-qty)` for equivalent semantics. Both approaches avoid last-write-wins integer overwrites that would silently drop concurrent edits.

All models are plain Kotlin data classes with no platform-specific annotations. Firestore serialization uses `toObject<T>()` and `set(obj)` with Kotlin data classes that have default constructor parameter values. Ditto serialization uses `mapOf()` passed to `store.execute()` — the same field names appear in both, which is the design intent. Repository interfaces live in `repositoryModule` (currently an empty Koin stub in `di/RepositoryModule.kt`) and will be populated this phase.

**Primary recommendation:** Write data classes first, derive the Firestore seed writer and Ditto DQL insert strings from those same field names — never hardcode field names separately in the bridge.

---

## Standard Stack

### Core
| Library | Version | Purpose | Why Standard |
|---------|---------|---------|--------------|
| Ditto SDK | 4.14.3 | Local CRDT store with COUNTER type for inventory | Already declared in libs.versions.toml; COUNTER type introduced in 4.14 |
| Firebase Firestore | BoM 34.10.0 (no version.ref) | Cloud source of truth; FieldValue.increment() for atomic quantity delta | Already declared; BoM manages version |
| Koin BOM | 4.1.0 | DI for repository singletons | Already wired in PosApplication; repositoryModule stub exists |
| kotlin-stdlib | via Kotlin 2.1.0 | UUID.randomUUID(), data classes, enums | Standard; no extra dependency needed |

### Supporting
| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| kotlinx-coroutines-test | 1.10.2 | Test suspend functions, StateFlow | Already in testImplementation; needed for seeder tests |
| mockk | 1.13.12 | Mock FirebaseFirestore in JVM unit tests | Already in testImplementation; use for seeder and repository tests |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| Plain data classes | @Serializable (kotlinx.serialization) | Serialization adds dependency; data classes + mapOf() are sufficient for Ditto and Firestore |
| UUID.randomUUID() | ditto.store.newDocumentID() | Ditto's method returns a Ditto-specific ID type; UUID.randomUUID().toString() produces plain strings compatible with both Firestore document IDs and Ditto _id |
| Ditto COUNTER | Separate delta event collection | Decision locked: CRDT counter is the chosen approach; no event log collection needed |

**Installation:** No new dependencies required — Ditto 4.14.3, Firestore, and Koin are already declared in `gradle/libs.versions.toml`.

---

## Architecture Patterns

### Recommended Project Structure
```
live/ditto/pubsec/pos/
├── data/
│   ├── model/
│   │   ├── Product.kt          # data class + Firestore/Ditto field names
│   │   ├── Order.kt            # data class with embedded List<OrderLineItem>
│   │   ├── OrderLineItem.kt    # embedded value object
│   │   ├── InventoryItem.kt    # data class with quantity as plain Int (bridge owns counter logic)
│   │   └── OrderStatus.kt      # enum class OPEN / FULFILLED
│   └── seed/
│       ├── SeedData.kt         # hardcoded List<Product> + List<InventoryItem>
│       └── FirestoreSeeder.kt  # writes seed data to Firestore if collection is empty
├── di/
│   └── RepositoryModule.kt     # add repository singletons here (stub already exists)
└── (rest of package unchanged)
```

### Pattern 1: Dual-Use Data Class

**What:** A single Kotlin data class whose field names match both the Firestore document fields and the Ditto DQL column names exactly. No separate DTO or mapping layer.

**When to use:** For all three model types (Product, Order, InventoryItem).

**Example:**
```kotlin
// Source: established in this project's Task model pattern (android-kotlin quickstart)
// and Firestore's toObject<T>() requirement for data classes with default values

data class Product(
    val _id: String = "",           // canonical UUID — must match Ditto _id and Firestore document ID
    val name: String = "",
    val category: String = "",      // "Beverage" | "Food" | "Merchandise" (plain String per decision)
    val price: Long = 0L,           // cents: 499 = $4.99
    val imageUrl: String = "",
    val deleted: Boolean = false,
    val syncSource: String = "local",  // "firestore" | "ditto" | "local"
    val lastSyncedAt: Long = 0L,    // epoch millis
)
```

**Why default values matter:** Firestore's `toObject<T>()` reflection requires a no-arg constructor — Kotlin data classes with all-default-value properties satisfy this without a separate no-arg constructor declaration.

### Pattern 2: Embedded List for Order Line Items

**What:** `Order.items` is a `List<OrderLineItem>` stored as a nested array in both Firestore and Ditto. Denormalized so POS can display order details without cross-collection lookups while offline.

**Example:**
```kotlin
data class OrderLineItem(
    val productId: String = "",
    val productName: String = "",   // denormalized snapshot at order time
    val quantity: Int = 0,
    val unitPrice: Long = 0L,       // cents
)

data class Order(
    val _id: String = "",
    val items: List<OrderLineItem> = emptyList(),
    val total: Long = 0L,           // cents; sum of (qty * unitPrice) for each line item
    val status: String = "OPEN",    // "OPEN" | "FULFILLED" — stored as String for DQL compat
    val timestamp: Long = 0L,       // epoch millis at order creation
    val terminalId: String = "",    // which POS terminal created this order
    val deleted: Boolean = false,
    val syncSource: String = "local",
    val lastSyncedAt: Long = 0L,
)
```

**Note:** `OrderStatus` enum is used in app code for type safety; serialize to/from String at the repository boundary. Do NOT annotate with `@Serializable` — plain `.name` and `enumValueOf<OrderStatus>()` is sufficient.

### Pattern 3: Inventory Item with Counter Semantics

**What:** The `InventoryItem` data class holds `quantity: Int` as a plain integer for Kotlin-land operations and UI display. The repository layer owns the delta-event logic: it calls `store.execute("UPDATE COLLECTION ... APPLY quantity INCREMENT BY :delta")` for Ditto and `docRef.update("quantity", FieldValue.increment(-qty))` for Firestore. The data class itself does NOT contain Ditto COUNTER type knowledge.

**Example:**
```kotlin
data class InventoryItem(
    val _id: String = "",           // canonical UUID — same as Firestore document ID
    val productId: String = "",     // foreign key to Product._id
    val quantity: Int = 0,          // read from Ditto COUNTER value; written via INCREMENT BY
    val lastModifiedBy: String = "", // terminalId for demo visibility
    val deleted: Boolean = false,
    val syncSource: String = "local",
    val lastSyncedAt: Long = 0L,
)
```

### Pattern 4: Ditto DQL Upsert (for bridge writes in Phase 3, but schema must anticipate it)

**What:** The bridge (Phase 3) will upsert documents from Firestore into Ditto. The schema must be stable before that code is written. The DQL upsert syntax is:

```sql
INSERT INTO inventory DOCUMENTS (:doc) ON ID CONFLICT DO UPDATE
```

`DO UPDATE` overwrites all fields. `DO UPDATE_LOCAL_DIFF` (SDK 4.12+) only updates changed fields — prefer this to minimize unnecessary replication. However, for COUNTER fields, upsert replaces the counter value; use `INCREMENT BY` in a separate `UPDATE COLLECTION` statement for quantity deltas.

### Pattern 5: Ditto COUNTER for Inventory

**What:** Quantity decrements during order placement use the CRDT counter to resolve concurrent offline edits correctly.

```sql
-- Initialize inventory quantity when seeding (plain integer initial value)
INSERT INTO inventory DOCUMENTS (:doc) ON ID CONFLICT DO NOTHING

-- Decrement when an order is placed
UPDATE COLLECTION inventory (quantity COUNTER)
APPLY quantity INCREMENT BY :delta
WHERE _id = :inventoryId
```

`delta` is negative for a decrement (e.g., `-2` removes 2 units). This is the key demo differentiator: two terminals can both decrement offline and the CRDT merges correctly on reconnect.

**Source:** Ditto 4.14 release notes — COUNTER type, INCREMENT BY, RESTART WITH confirmed available.

### Pattern 6: Firestore Atomic Increment

**What:** Equivalent to Ditto's COUNTER — Firestore's `FieldValue.increment()` performs a server-side atomic delta without reading the document first.

```kotlin
// Source: firebase/snippets-android official Kotlin examples
inventoryRef.update("quantity", FieldValue.increment(-quantity.toLong()))
```

This is a Phase 3 concern (bridge code), but the schema must declare `quantity` as a numeric field — no special annotation needed.

### Pattern 7: Firestore Seed Writer

**What:** A `FirestoreSeeder` class checks if the `products` collection is empty, then writes the hardcoded `SeedData.products` list. Each product's `_id` becomes the Firestore document ID.

```kotlin
class FirestoreSeeder(private val db: FirebaseFirestore) {

    suspend fun seedIfEmpty() {
        val snapshot = db.collection("products").limit(1).get().await()
        if (snapshot.isEmpty) {
            val batch = db.batch()
            SeedData.products.forEach { product ->
                val ref = db.collection("products").document(product._id)
                batch.set(ref, product)
            }
            SeedData.inventoryItems.forEach { item ->
                val ref = db.collection("inventory").document(item._id)
                batch.set(ref, item)
            }
            batch.commit().await()
        }
    }
}
```

`batch.commit().await()` requires `kotlinx-coroutines-play-services` or the `Tasks.await()` extension. Check if `firebase-firestore` BoM 34.10.0 includes `.await()` extension — it does via `com.google.firebase:firebase-firestore` which bundles KTX since BoM 33+.

### Anti-Patterns to Avoid

- **Field name drift:** Defining separate DTO/entity classes with different field names for Ditto vs Firestore. The single data class IS the contract — the bridge reads and writes the same field names in both systems.
- **Hardcoded field strings in multiple places:** If `"quantity"` appears in the seeder, the repository, and the bridge as separate string literals, a rename breaks them silently. Define field name constants or use property references.
- **Physical deletes:** Never call `documentRef.delete()` or `DELETE FROM collection WHERE ...`. Always set `deleted = true`.
- **Auto-generated IDs:** Never use `db.collection("products").add(obj)` (Firestore auto-ID) or omit `_id` in Ditto INSERT. Always supply the UUID.
- **Kotlin value classes for IDs:** Not recommended here — Firestore reflection and Ditto's string-typed `_id` field both work cleanly with `String`. Value classes add boilerplate for no practical gain in this context.
- **Float/Double for money:** Locked decision — use `Long` cents. Never `Double`.

---

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Concurrent inventory decrement merge | Custom LWW integer or event log | Ditto COUNTER `INCREMENT BY` | Two offline decrements on LWW integers silently drop one; CRDT counter merges both correctly |
| Atomic cloud quantity update | Read-modify-write on Firestore quantity | `FieldValue.increment(-n)` | Eliminates race condition between terminals on the same Firestore document |
| UUID generation | Custom ID scheme | `UUID.randomUUID().toString()` | Standard, guaranteed unique across terminals without coordination |
| Firestore no-arg constructor | Manual `@JvmOverloads` or factory methods | Kotlin data class with all-default parameters | Firestore's `toObject<T>()` reflection handles this automatically |
| Batch seed write | Sequential `.set()` calls with individual awaits | `db.batch()` + `batch.commit().await()` | Atomically writes all seed documents; avoids partial-seed state on first run |

**Key insight:** The CRDT counter is the entire reason Ditto exists in this architecture. Building a custom delta-event log recreates what the SDK already provides — and gets the merge semantics wrong under network partition.

---

## Common Pitfalls

### Pitfall 1: Forgetting `_id` must be both the Ditto document ID and the Firestore document ID

**What goes wrong:** Developer uses Firestore's auto-generated document ID (from `.add()`) and stores it in a separate `id` field, while Ditto uses its own UUID for `_id`. The two stores diverge — bridge writes create duplicate documents.

**Why it happens:** Firestore's `.add()` API auto-generates an ID; it's the path of least resistance.

**How to avoid:** Always use `db.collection("products").document(product._id).set(product)` — supply the UUID as the Firestore document ID explicitly. Mirror this in Ditto with `INSERT INTO products DOCUMENTS ({_id: :id, ...})`.

**Warning signs:** Firestore console shows documents with IDs that don't match the `_id` field values inside the documents.

### Pitfall 2: Ditto COUNTER field initialized incorrectly

**What goes wrong:** Developer inserts an InventoryItem with `quantity: 20` as a plain integer, then tries to `APPLY quantity INCREMENT BY -5`. Ditto throws because `quantity` was never declared as a COUNTER — it's stored as a register (LWW integer).

**Why it happens:** The COUNTER type must be declared at first INSERT or the field defaults to register semantics.

**How to avoid:** Use `INSERT INTO COLLECTION inventory (quantity COUNTER) DOCUMENTS (...)` when first creating inventory documents. The `COLLECTION` keyword with type declarations tells Ditto the field is a COUNTER. Alternatively, use `UPDATE COLLECTION inventory (quantity COUNTER) APPLY quantity RESTART WITH :initialQty WHERE _id = :id` immediately after a plain INSERT.

**Warning signs:** DQL `UPDATE COLLECTION ... APPLY ... INCREMENT BY` throws a type mismatch error at runtime.

### Pitfall 3: `toObject<T>()` fails when data class lacks default constructor

**What goes wrong:** Firestore reflection cannot deserialize a document into a Kotlin data class if any property lacks a default value. Result: `NullPointerException` or `RuntimeException: No no-arg constructor found`.

**Why it happens:** Kotlin data classes with required (non-default) constructor parameters don't expose a no-arg constructor to Java reflection unless `@JvmOverloads` or all-default params are used.

**How to avoid:** All data class properties MUST have default values (as shown in examples above). Test deserialization with a unit test before writing bridge code.

**Warning signs:** `toObject<Product>()` returns null or throws at runtime on a valid Firestore document.

### Pitfall 4: SyncSource tag not set on seed data, triggering phantom bridge writes

**What goes wrong:** Seed data written to Firestore without `syncSource = "firestore"` causes the Phase 3 bridge to see these documents as "untagged" and attempts to write them back to Ditto — creating a write loop on first launch.

**Why it happens:** SeedData.products are constructed with `syncSource = "local"` (default) and the Firestore seeder doesn't override it.

**How to avoid:** `SeedData.products` must hardcode `syncSource = "firestore"` on every item. The seeder writes these directly; the Phase 3 bridge skips documents already tagged "firestore" on the Ditto-to-Firestore direction.

**Warning signs:** Firestore shows rapidly incrementing `lastSyncedAt` values on seed documents immediately after app launch.

### Pitfall 5: `FieldValue.increment()` used for inventory initialization (not just delta)

**What goes wrong:** Seeder uses `update("quantity", FieldValue.increment(initialQty))` on a document that doesn't exist yet — Firestore's `update()` fails if the document doesn't exist.

**Why it happens:** `update()` requires the document to exist; `set()` creates or overwrites.

**How to avoid:** Seed writes use `batch.set(ref, inventoryItem)` with the full object. `FieldValue.increment()` is only used for subsequent delta updates from order placement (Phase 3).

---

## Code Examples

Verified patterns from official sources:

### Firestore Kotlin Data Class Mapping
```kotlin
// Source: firebase/snippets-android official Kotlin examples
// Data class must have ALL properties with default values for toObject<T>() reflection

data class Product(
    val _id: String = "",
    val name: String = "",
    val category: String = "",
    val price: Long = 0L,
    val imageUrl: String = "",
    val deleted: Boolean = false,
    val syncSource: String = "local",
    val lastSyncedAt: Long = 0L,
)

// Read from Firestore:
val product = documentSnapshot.toObject<Product>()

// Write to Firestore (with explicit document ID):
db.collection("products").document(product._id).set(product).await()
```

### Ditto DQL Insert (plain document, no COUNTER)
```kotlin
// Source: Ditto docs.ditto.live/dql/insert — INSERT INTO DOCUMENTS syntax
ditto.store.execute(
    "INSERT INTO products DOCUMENTS (:doc) ON ID CONFLICT DO UPDATE_LOCAL_DIFF",
    mapOf("doc" to mapOf(
        "_id" to product._id,
        "name" to product.name,
        "category" to product.category,
        "price" to product.price,
        "imageUrl" to product.imageUrl,
        "deleted" to product.deleted,
        "syncSource" to product.syncSource,
        "lastSyncedAt" to product.lastSyncedAt,
    ))
)
```

### Ditto DQL Insert with COUNTER Field (inventory)
```kotlin
// Source: Ditto 4.14 release notes — COUNTER type
// INSERT INTO COLLECTION declares field types; quantity initialized as COUNTER
ditto.store.execute("""
    INSERT INTO COLLECTION inventory (quantity COUNTER)
    DOCUMENTS (:doc)
    ON ID CONFLICT DO NOTHING
""", mapOf("doc" to mapOf(
    "_id" to item._id,
    "productId" to item.productId,
    "quantity" to item.quantity,   // initial integer value loaded into COUNTER
    "lastModifiedBy" to item.lastModifiedBy,
    "deleted" to item.deleted,
    "syncSource" to item.syncSource,
    "lastSyncedAt" to item.lastSyncedAt,
)))
```

### Ditto DQL Inventory Decrement
```kotlin
// Source: Ditto 4.14 release notes — COUNTER INCREMENT BY
// delta is negative for decrement (e.g., -2 removes 2 units)
ditto.store.execute("""
    UPDATE COLLECTION inventory (quantity COUNTER)
    APPLY quantity INCREMENT BY :delta
    WHERE _id = :inventoryId
""", mapOf(
    "delta" to -quantityToRemove,
    "inventoryId" to inventoryItem._id,
))
```

### Firestore Atomic Decrement
```kotlin
// Source: firebase/snippets-android — FieldValue.increment()
db.collection("inventory").document(inventoryItem._id)
    .update("quantity", FieldValue.increment(-quantityToRemove.toLong()))
    .await()
```

### UUID Generation
```kotlin
// Source: existing quickstart Task.kt pattern (android-kotlin/QuickStartTasks)
import java.util.UUID

val id = UUID.randomUUID().toString()
```

### SeedData Object Pattern
```kotlin
// Pattern: single Kotlin object with hardcoded lists; no external JSON
object SeedData {
    val products: List<Product> = listOf(
        Product(
            _id = "a1b2c3d4-e5f6-7890-abcd-ef1234567890",
            name = "Espresso",
            category = "Beverage",
            price = 299,  // $2.99
            imageUrl = "",
            syncSource = "firestore",  // CRITICAL: must be set on seed data
        ),
        // ... ~8-10 items
    )

    val inventoryItems: List<InventoryItem> = products.map { product ->
        InventoryItem(
            _id = UUID.randomUUID().toString(),
            productId = product._id,
            quantity = 50,
            syncSource = "firestore",
        )
    }
}
```

**Note:** Product UUIDs in SeedData must be hardcoded (not generated at runtime) so re-running the seeder produces the same IDs and `DO NOTHING` / empty-check prevents duplicates.

---

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| `PN_COUNTER` DQL type | `COUNTER` type with RESTART WITH | Ditto 4.14 | COUNTER is preferred; PN_COUNTER still works but is legacy |
| `isPersistenceEnabled = false` Firestore setting | `setLocalCacheSettings(memoryCacheSettings {})` | Firebase BoM ~33+ | Already applied in Phase 1 AppModule |
| `firebase-firestore-ktx` artifact | `firebase-firestore` (KTX bundled) | Firebase BoM 34.0.0 | Already applied in Phase 1 |
| `ditto.store.upsert()` legacy API | `INSERT INTO ... ON ID CONFLICT DO UPDATE` DQL | Ditto 4.x DQL migration | Must use DQL; legacy API removed |

**Deprecated/outdated:**
- `PN_INCREMENT BY` (legacy DQL counter syntax): still works per 4.14 release notes but prefer `COUNTER` type with `INCREMENT BY`.
- `DQL_STRICT_MODE=false` flag: needed if using counter types in older SDK versions; not required at 4.14.3 per current docs (verify during implementation).

---

## Open Questions

1. **`INSERT INTO COLLECTION` exact syntax for initializing COUNTER**
   - What we know: Release notes confirm `COUNTER` type exists; `UPDATE COLLECTION inventory (quantity COUNTER) APPLY quantity INCREMENT BY` is confirmed syntax
   - What's unclear: Whether `INSERT INTO COLLECTION inventory (quantity COUNTER) DOCUMENTS (...)` correctly initializes the counter with an integer seed value, or if a separate `UPDATE COLLECTION ... RESTART WITH` is needed after a plain `INSERT INTO`
   - Recommendation: In the plan, include a verification task that inserts one test inventory document, reads it back, and confirms `APPLY quantity INCREMENT BY` works — before writing the full seeder

2. **`DQL_STRICT_MODE` requirement at 4.14.3**
   - What we know: Legacy-to-DQL reference notes `"Available in 4.11 and later with DQL_STRICT_MODE=false"` for counter types
   - What's unclear: Whether strict mode must be explicitly disabled at 4.14.3 or if the COUNTER type works in the default mode
   - Recommendation: The plan should include a smoke test that confirms `ditto.disableSyncWithV3()` (already called) plus counter INSERT/UPDATE works without additional configuration

3. **`Firestore.Tasks.await()` extension availability**
   - What we know: `firebase-firestore` BoM 34.10.0 bundles KTX since BoM 33+; `kotlinx-coroutines-play-services` provides `Task.await()`
   - What's unclear: Whether `kotlinx-coroutines-play-services` is already in the dependency graph or needs to be added
   - Recommendation: Plan Wave 0 should check `build.gradle.kts` — if `.await()` is used in the seeder, add `kotlinx-coroutines-play-services` if absent

---

## Validation Architecture

### Test Framework
| Property | Value |
|----------|-------|
| Framework | JUnit 4 + kotlinx-coroutines-test 1.10.2 + mockk 1.13.12 |
| Config file | No separate config — standard Android Gradle test setup |
| Quick run command | `./gradlew :app:test` (from `pubsec/android-firebase-pos/`) |
| Full suite command | `./gradlew :app:test :app:connectedAndroidTest` |

### Phase Requirements -> Test Map
| Req ID | Behavior | Test Type | Automated Command | File Exists? |
|--------|----------|-----------|-------------------|-------------|
| DATA-01 | Product data class has correct fields; Firestore `toObject<Product>()` succeeds | unit | `./gradlew :app:test --tests "*.ProductModelTest"` | Wave 0 |
| DATA-02 | Order with embedded List<OrderLineItem> serializes/deserializes correctly | unit | `./gradlew :app:test --tests "*.OrderModelTest"` | Wave 0 |
| DATA-03 | InventoryItem data class correct; Ditto COUNTER smoke test (insert + increment) | unit (mock) | `./gradlew :app:test --tests "*.InventoryModelTest"` | Wave 0 |
| DATA-04 | No physical delete calls exist in the codebase; deleted flag present on all models | unit | `./gradlew :app:test --tests "*.SoftDeleteConventionTest"` | Wave 0 |
| DATA-05 | SyncSource field present on all models; seed data has syncSource = "firestore" | unit | `./gradlew :app:test --tests "*.SyncSourceTagTest"` | Wave 0 |

**Note on DATA-03:** Ditto JNI requires Android runtime — the unit test for inventory COUNTER behavior should mock the Ditto store (using mockk relaxed) and verify the DQL string and arguments, not actual CRDT merge. The CRDT semantics are Ditto's responsibility; we verify our invocation.

### Sampling Rate
- **Per task commit:** `./gradlew :app:test` (JVM unit tests only — fast, no emulator)
- **Per wave merge:** `./gradlew :app:test`
- **Phase gate:** `./gradlew :app:test` green before `/gsd:verify-work` (no instrumented tests required for data model phase)

### Wave 0 Gaps
- [ ] `app/src/test/java/live/ditto/pubsec/pos/ProductModelTest.kt` — covers DATA-01
- [ ] `app/src/test/java/live/ditto/pubsec/pos/OrderModelTest.kt` — covers DATA-02
- [ ] `app/src/test/java/live/ditto/pubsec/pos/InventoryModelTest.kt` — covers DATA-03
- [ ] `app/src/test/java/live/ditto/pubsec/pos/SoftDeleteConventionTest.kt` — covers DATA-04
- [ ] `app/src/test/java/live/ditto/pubsec/pos/SyncSourceTagTest.kt` — covers DATA-05
- [ ] Check `build.gradle.kts` for `kotlinx-coroutines-play-services` — needed if seeder uses `.await()` on Firestore Tasks

---

## Sources

### Primary (HIGH confidence)
- `docs.ditto.live/sdk/latest/release-notes/kotlin` — COUNTER type, INCREMENT BY, RESTART WITH confirmed in Ditto 4.14
- `docs.ditto.live/dql/insert` — ON ID CONFLICT DO UPDATE / DO UPDATE_LOCAL_DIFF / DO NOTHING confirmed
- `github.com/firebase/snippets-android` — Official Kotlin FieldValue.increment() and toObject<T>() patterns
- Existing project codebase — libs.versions.toml (Ditto 4.14.3, Firebase BoM 34.10.0, Koin 4.1.0, mockk 1.13.12 confirmed)
- Existing quickstart `android-kotlin/QuickStartTasks/data/Task.kt` — UUID.randomUUID(), `deleted: Boolean`, `_id: String` pattern

### Secondary (MEDIUM confidence)
- `docs.ditto.live/dql/legacy-to-dql-refs` — PN_INCREMENT BY syntax (legacy); COUNTER preferred in 4.14
- Web search result summary of Ditto 4.14 COUNTER type semantics (cross-verified with release notes page)

### Tertiary (LOW confidence)
- `DQL_STRICT_MODE=false` requirement for counter types — mentioned in legacy-to-DQL reference but unclear if still required at 4.14.3; flagged as open question

---

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH — all versions confirmed in project's libs.versions.toml
- Architecture patterns: HIGH — dual-use data class and Firestore toObject<T>() confirmed from official sources; Ditto CRDT counter confirmed from release notes
- Pitfalls: HIGH — UUID/ID divergence and COUNTER initialization are well-documented Ditto gotchas; Firestore no-arg constructor requirement confirmed from official SDK behavior
- DQL COUNTER init syntax: MEDIUM — INCREMENT BY and RESTART WITH confirmed; exact INSERT INTO COLLECTION initialization syntax needs live verification

**Research date:** 2026-03-04
**Valid until:** 2026-04-04 (stable APIs; Ditto SDK version pinned at 4.14.3)
