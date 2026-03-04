---
phase: 02-data-models
plan: 01
subsystem: database
tags: [kotlin, android, ditto, firestore, data-models, schema]

# Dependency graph
requires:
  - phase: 01-foundation
    provides: Koin DI modules, AppModule (Ditto + Firestore instances), test infrastructure with JUnit
provides:
  - Product data class with all POS fields and defaults (Long cents, syncSource, deleted)
  - Order data class with embedded List<OrderLineItem>, status as String
  - InventoryItem data class with Int quantity (CRDT logic deferred to Phase 3)
  - OrderLineItem embedded value object (denormalized for offline reads)
  - OrderStatus enum (OPEN, FULFILLED)
  - Collections object with collection and field name constants
  - 5 unit test classes covering all models, soft-delete convention, syncSource tagging
affects: [03-repositories, 04-ui, seeder, bridge, dql-queries]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - All monetary values as Long cents (e.g., 499 = $4.99) — no floating-point
    - Soft delete via deleted: Boolean = false on all collection models
    - SyncSource tagging: syncSource: String = "local" on all models, values are "local"/"firestore"/"ditto"
    - All fields have defaults — satisfies Firestore toObject<T>() no-arg constructor requirement
    - status stored as String in Order data class for DQL compatibility; OrderStatus enum for app-layer type safety
    - Collection name constants in Collections object; field name constants in nested Collections.Fields object

key-files:
  created:
    - pubsec/android-firebase-pos/app/src/main/java/live/ditto/pubsec/pos/data/model/Product.kt
    - pubsec/android-firebase-pos/app/src/main/java/live/ditto/pubsec/pos/data/model/Order.kt
    - pubsec/android-firebase-pos/app/src/main/java/live/ditto/pubsec/pos/data/model/OrderLineItem.kt
    - pubsec/android-firebase-pos/app/src/main/java/live/ditto/pubsec/pos/data/model/InventoryItem.kt
    - pubsec/android-firebase-pos/app/src/main/java/live/ditto/pubsec/pos/data/model/OrderStatus.kt
    - pubsec/android-firebase-pos/app/src/main/java/live/ditto/pubsec/pos/data/model/Collections.kt
    - pubsec/android-firebase-pos/app/src/test/java/live/ditto/pubsec/pos/ProductModelTest.kt
    - pubsec/android-firebase-pos/app/src/test/java/live/ditto/pubsec/pos/OrderModelTest.kt
    - pubsec/android-firebase-pos/app/src/test/java/live/ditto/pubsec/pos/InventoryModelTest.kt
    - pubsec/android-firebase-pos/app/src/test/java/live/ditto/pubsec/pos/SoftDeleteConventionTest.kt
    - pubsec/android-firebase-pos/app/src/test/java/live/ditto/pubsec/pos/SyncSourceTagTest.kt
  modified: []

key-decisions:
  - "All data class properties are val (immutable); mutation via copy() only — enforces data integrity"
  - "Order.status is String not enum in data class — DQL stores/queries strings, enum only at app layer"
  - "Collections.Fields nested object added beyond plan spec — prevents hardcoded strings in repository/bridge code"
  - "InventoryItem.quantity is Int — CRDT counter mechanics are a Phase 3 repository concern, not model concern"

patterns-established:
  - "Soft delete pattern: all collection models carry deleted: Boolean = false; no hard deletes"
  - "SyncSource pattern: syncSource String + lastSyncedAt Long on every synced model for loop prevention and debug"
  - "Monetary amounts as Long cents: price, unitPrice, total all Long — never Double or Float"
  - "Denormalization for offline: OrderLineItem embeds productName and unitPrice — no cross-collection lookup needed"

requirements-completed: [DATA-01, DATA-02, DATA-03, DATA-04, DATA-05]

# Metrics
duration: 2min
completed: 2026-03-04
---

# Phase 2 Plan 01: Data Models Summary

**Kotlin data classes for Product, Order, and InventoryItem with soft-delete, syncSource tagging, Long-cent pricing, and embedded OrderLineItem — schema contract locked before any write code**

## Performance

- **Duration:** 2 min
- **Started:** 2026-03-04T18:12:27Z
- **Completed:** 2026-03-04T18:14:16Z
- **Tasks:** 2
- **Files modified:** 11 (6 model files + 5 test files)

## Accomplishments
- Created 6 model/constant files under `data/model/` package: Product, Order, OrderLineItem, InventoryItem, OrderStatus, Collections
- Written 5 unit test files covering field types, defaults, soft-delete convention, and syncSource tagging
- All tests pass alongside pre-existing KoinModuleTest, FirestoreSettingsTest, ExampleUnitTest — `./gradlew :app:test` exits 0
- Added `Collections.Fields` nested object (beyond plan spec) to prevent hardcoded strings in downstream repository and bridge code

## Task Commits

Each task was committed atomically:

1. **Task 1: Create data model classes and constants** - `f0e2905` (feat)
2. **Task 2: Write unit tests for all data models** - `c72e84b` (test)

## Files Created/Modified

- `data/model/Product.kt` - Product with _id, name, category, price (Long cents), imageUrl, deleted, syncSource, lastSyncedAt
- `data/model/OrderLineItem.kt` - Embedded value object: productId, productName, quantity, unitPrice (Long)
- `data/model/OrderStatus.kt` - enum class with OPEN and FULFILLED values only
- `data/model/Order.kt` - Order with embedded List<OrderLineItem>, status as String, terminalId, all sync fields
- `data/model/InventoryItem.kt` - InventoryItem with Int quantity, lastModifiedBy (terminalId), all sync fields
- `data/model/Collections.kt` - Object with PRODUCTS/ORDERS/INVENTORY constants + nested Fields object
- `test/.../ProductModelTest.kt` - Default constructor, id type, price type, explicit values, copy round-trip
- `test/.../OrderModelTest.kt` - Empty items default, OrderLineItem embedding, OrderStatus enum assertion
- `test/.../InventoryModelTest.kt` - Quantity Int type, productId/lastModifiedBy field assertions
- `test/.../SoftDeleteConventionTest.kt` - deleted=false default on all three models, copy mutation
- `test/.../SyncSourceTagTest.kt` - syncSource="local" default, lastSyncedAt=0L, valid value mutations

## Decisions Made

- `Collections.Fields` nested object added beyond plan spec to prevent hardcoded strings in repository/bridge code written in Phase 3 — harmless addition that avoids future Rule 2 auto-fixes
- `Order.status` stored as String in data class, not enum, for Ditto DQL compatibility — `OrderStatus.name` at write site, `enumValueOf<OrderStatus>()` at read site
- All properties `val` only — immutability enforced at model layer; copy() used for all mutations in tests

## Deviations from Plan

None - plan executed exactly as written, except one beneficial addition: `Collections.Fields` nested object included in `Collections.kt` (plan mentioned it as optional "if useful" — chosen yes to avoid downstream hardcoded strings).

## Issues Encountered

None — data classes compiled cleanly on first attempt, all tests passed on first run. Kotlin compiler emitted "Check for instance is always 'true'" warnings on `is String`, `is Int`, `is Long` checks in tests — these are intentional behavioral documentation tests, not logic errors.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- Schema contract is now the single source of truth — field names in these files are identical for both Ditto DQL columns and Firestore document fields
- `repositoryModule` stub in `di/RepositoryModule.kt` is the hook for Phase 3 repository classes
- Collections and field name constants in `Collections.kt` prevent string drift between Ditto and Firestore operations
- Phase 3 can implement Ditto DQL upsert and Firestore bridge using these model types directly

---
*Phase: 02-data-models*
*Completed: 2026-03-04*
