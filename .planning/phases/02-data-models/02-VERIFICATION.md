---
phase: 02-data-models
verified: 2026-03-04T00:00:00Z
status: passed
score: 13/14 must-haves verified
re_verification: false
human_verification:
  - test: "Confirm Firestore seeded data is visible in Firebase console"
    expected: "products and inventory collections populated with 9 items each after first app run"
    why_human: "Requires a live Firebase project, device/emulator, and Firebase console access — cannot verify Firestore writes programmatically from this codebase alone"
  - test: "Confirm inventory delta-event pattern scope alignment"
    expected: "Success Criterion 5 ('Inventory write operations use delta events rather than absolute quantity overwrites') is a Phase 3 repository concern — InventoryItem.quantity is Int and FieldValue.increment() / Ditto CRDT counter wiring is deferred to Phase 3. Phase 2 only locks the schema shape. Verify this deferral is intentional and accepted."
    why_human: "The ROADMAP success criterion 5 sounds like a write-operation requirement, but all three PLANs explicitly defer CRDT/increment logic to Phase 3. A human must confirm this deferral is accepted as Phase 2 scope."
---

# Phase 2: Data Models Verification Report

**Phase Goal:** Schema contracts for all three collections are finalized and enforced before any write code exists — canonical UUID IDs, soft delete convention, and inventory delta-event pattern locked in
**Verified:** 2026-03-04
**Status:** human_needed (all automated checks pass; 2 items require human confirmation)
**Re-verification:** No — initial verification

---

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|-------|--------|---------|
| 1 | Product data class has `_id, name, category, price (Long cents), imageUrl, deleted, syncSource, lastSyncedAt` fields with defaults | VERIFIED | `Product.kt` line 3-12: all 8 fields present, all have defaults, `price: Long = 0L` |
| 2 | Order data class has `_id, items (List<OrderLineItem>), total (Long cents), status (String), timestamp, terminalId, deleted, syncSource, lastSyncedAt` with defaults | VERIFIED | `Order.kt` line 3-13: all 9 fields present, `total: Long = 0L`, `status: String = "OPEN"` |
| 3 | InventoryItem data class has `_id, productId, quantity (Int), lastModifiedBy, deleted, syncSource, lastSyncedAt` with defaults | VERIFIED | `InventoryItem.kt` line 3-11: all 7 fields present, `quantity: Int = 0` |
| 4 | All three models have `deleted: Boolean = false` (soft delete convention) | VERIFIED | Confirmed in all three data classes; 9-test `SoftDeleteConventionTest` passes |
| 5 | All three models have `syncSource: String = "local"` default | VERIFIED | Confirmed in all three data classes; 12-test `SyncSourceTagTest` passes |
| 6 | OrderStatus enum has OPEN and FULFILLED values | VERIFIED | `OrderStatus.kt`: `enum class OrderStatus { OPEN, FULFILLED }` — exactly 2 values |
| 7 | Collection and field name constants exist for DQL/Firestore references | VERIFIED | `Collections.kt`: `object Collections` with PRODUCTS/ORDERS/INVENTORY + nested `Collections.Fields` with 17 field name constants |
| 8 | SeedData.products contains 8-10 coffee shop items across Beverage, Food, Merchandise categories | VERIFIED | `SeedData.kt`: 9 products (Espresso, Latte, Cappuccino, Cold Brew, Croissant, Turkey Sandwich, Blueberry Muffin, Coffee Mug, Tote Bag) across all 3 categories |
| 9 | SeedData.inventoryItems has one entry per product with matching productId | VERIFIED | `SeedData.kt`: 9 InventoryItems, each with `productId` matching a product `_id` |
| 10 | All seed products and inventory items have `syncSource = "firestore"` | VERIFIED | All 18 seed entries have `syncSource = "firestore"` hardcoded |
| 11 | All seed product `_id` values are hardcoded UUIDs (not generated at runtime) | VERIFIED | UUIDs are string literals (e.g., `"550e8400-e29b-41d4-a716-446655440001"`); no `UUID.randomUUID()` call anywhere in SeedData.kt |
| 12 | FirestoreSeeder checks if products collection is empty before writing | VERIFIED | `FirestoreSeeder.kt` line 10-11: `db.collection(Collections.PRODUCTS).limit(1).get().await()` then `if (snapshot.isEmpty)` |
| 13 | FirestoreSeeder uses batch write with explicit document IDs (product._id as Firestore doc ID) | VERIFIED | `FirestoreSeeder.kt` lines 13-19: `document(product._id)` / `document(item._id)` with `batch.set()` |
| 14 | FirestoreSeeder is available via Koin DI | VERIFIED | `RepositoryModule.kt`: `single { FirestoreSeeder(get()) }` — no longer an empty stub |

**Score:** 14/14 truths automated-verified (2 additional human confirmations needed for SC2 and SC5 from ROADMAP)

---

### Required Artifacts

**Plan 01 Artifacts**

| Artifact | Expected | Status | Details |
|----------|----------|--------|---------|
| `pubsec/android-firebase-pos/app/src/main/java/live/ditto/pubsec/pos/data/model/Product.kt` | Product data class | VERIFIED | 12 lines, `data class Product` with 8 fields, all `val`, all with defaults |
| `pubsec/android-firebase-pos/app/src/main/java/live/ditto/pubsec/pos/data/model/Order.kt` | Order data class with embedded OrderLineItem list | VERIFIED | 13 lines, `data class Order` with `items: List<OrderLineItem>` |
| `pubsec/android-firebase-pos/app/src/main/java/live/ditto/pubsec/pos/data/model/InventoryItem.kt` | InventoryItem data class | VERIFIED | 11 lines, `data class InventoryItem` with 7 fields |
| `pubsec/android-firebase-pos/app/src/main/java/live/ditto/pubsec/pos/data/model/OrderStatus.kt` | OrderStatus enum | VERIFIED | 6 lines, `enum class OrderStatus { OPEN, FULFILLED }` |
| `pubsec/android-firebase-pos/app/src/main/java/live/ditto/pubsec/pos/data/model/Collections.kt` | Collection and field name constants | VERIFIED | `object Collections` with PRODUCTS/ORDERS/INVENTORY + `object Fields` with 17 constants |
| `pubsec/android-firebase-pos/app/src/test/java/live/ditto/pubsec/pos/ProductModelTest.kt` | Product model unit tests | VERIFIED | 5 tests covering defaults, id type, price type, explicit values, copy round-trip |
| `pubsec/android-firebase-pos/app/src/test/java/live/ditto/pubsec/pos/OrderModelTest.kt` | Order model unit tests | VERIFIED | 7 tests covering defaults, line item embedding, total type, status, enum values |
| `pubsec/android-firebase-pos/app/src/test/java/live/ditto/pubsec/pos/InventoryModelTest.kt` | InventoryItem unit tests | VERIFIED | 5 tests covering defaults, quantity Int type, productId, lastModifiedBy |
| `pubsec/android-firebase-pos/app/src/test/java/live/ditto/pubsec/pos/SoftDeleteConventionTest.kt` | Soft delete convention tests | VERIFIED | 9 tests — deleted=false defaults + Boolean type + copy-mutation on all 3 models |
| `pubsec/android-firebase-pos/app/src/test/java/live/ditto/pubsec/pos/SyncSourceTagTest.kt` | SyncSource tagging tests | VERIFIED | 12 tests — syncSource defaults, lastSyncedAt defaults, type checks, copy mutations |

**Plan 02 Artifacts**

| Artifact | Expected | Status | Details |
|----------|----------|--------|---------|
| `pubsec/android-firebase-pos/app/src/main/java/live/ditto/pubsec/pos/data/seed/SeedData.kt` | Hardcoded coffee shop product catalog and inventory | VERIFIED | `object SeedData` with 9 products + 9 inventory items, all with hardcoded UUIDs and `syncSource="firestore"` |
| `pubsec/android-firebase-pos/app/src/main/java/live/ditto/pubsec/pos/data/seed/FirestoreSeeder.kt` | Firestore batch writer with empty-check guard | VERIFIED | `class FirestoreSeeder` with `suspend fun seedIfEmpty()` using `limit(1)` guard and `batch.set()` |
| `pubsec/android-firebase-pos/app/src/main/java/live/ditto/pubsec/pos/di/RepositoryModule.kt` | Koin bindings for FirestoreSeeder | VERIFIED | `single { FirestoreSeeder(get()) }` — stub replaced with real binding |
| `pubsec/android-firebase-pos/app/src/test/java/live/ditto/pubsec/pos/SeedDataTest.kt` | Seed data integrity tests | VERIFIED | `class SeedDataTest` with 12 tests covering count, IDs, prices, syncSource, deleted, categories, inventory references |
| `pubsec/android-firebase-pos/app/src/test/java/live/ditto/pubsec/pos/FirestoreSeederTest.kt` | Mock-based seeder tests | VERIFIED | `class FirestoreSeederTest` with 5 mockk tests: empty-check guard, batch write, skip when non-empty, canonical product ID, canonical inventory ID |

---

### Key Link Verification

| From | To | Via | Status | Details |
|------|----|-----|--------|---------|
| `Order.kt` | `OrderLineItem.kt` | `items: List<OrderLineItem>` | WIRED | Line 5 of Order.kt: `val items: List<OrderLineItem> = emptyList()` |
| `Order.kt` | `OrderStatus.kt` | Status stored as String, enum for type safety | WIRED | `OrderModelTest` line 51 confirms `OrderStatus.OPEN.name == order.status`; enumValueOf round-trip tested |
| `SeedData.kt` | `Product.kt` | `List<Product>` | WIRED | Line 8: `val products: List<Product>` with import of Product |
| `SeedData.kt` | `InventoryItem.kt` | `List<InventoryItem>` | WIRED | Line 101: `val inventoryItems: List<InventoryItem>` with import of InventoryItem |
| `FirestoreSeeder.kt` | `SeedData.kt` | reads SeedData.products and SeedData.inventoryItems | WIRED | Lines 13 and 17: `SeedData.products.forEach` / `SeedData.inventoryItems.forEach` |
| `FirestoreSeeder.kt` | `Collections.kt` | uses collection name constants | WIRED | Lines 10, 14, 18: `Collections.PRODUCTS` / `Collections.INVENTORY` — no hardcoded strings |
| `RepositoryModule.kt` | `FirestoreSeeder.kt` | Koin `single{}` binding | WIRED | `single { FirestoreSeeder(get()) }` with import verified |

---

### Requirements Coverage

| Requirement | Source Plan | Description | Status | Evidence |
|-------------|------------|-------------|--------|---------|
| DATA-01 | 02-01, 02-02 | Products collection with canonical UUID IDs (name, price, category, imageUrl) | SATISFIED | Product.kt has all fields; SeedData has hardcoded UUID `_id` values used as Firestore document IDs |
| DATA-02 | 02-01 | Orders collection with canonical UUID IDs (items, total, status, timestamp, terminalId) | SATISFIED | Order.kt has all fields including `_id: String`, `items: List<OrderLineItem>`, `total: Long` |
| DATA-03 | 02-01, 02-02 | Inventory collection with canonical UUID IDs (productId, quantity) | SATISFIED | InventoryItem.kt has `_id`, `productId`, `quantity: Int`; seed data confirms UUID pattern |
| DATA-04 | 02-01, 02-02 | Soft delete convention using `deleted` flag (not hard deletes) | SATISFIED | All 3 models have `deleted: Boolean = false`; SoftDeleteConventionTest has 9 passing tests |
| DATA-05 | 02-01, 02-02 | SyncSource tagging on all bridge-written documents to prevent sync loops | SATISFIED | All 3 models have `syncSource: String = "local"` and `lastSyncedAt: Long`; seed data sets `syncSource = "firestore"` to prevent phantom bridge writes |

**Orphaned requirements check:** REQUIREMENTS.md traceability table maps DATA-01 through DATA-05 to Phase 2 — all 5 are claimed by both plans. No orphaned requirements.

---

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|------|------|---------|----------|--------|
| None | - | - | - | - |

No TODOs, FIXMEs, placeholders, empty implementations, or `UUID.randomUUID()` calls found in any Phase 2 source or test file.

---

### Human Verification Required

#### 1. Firestore Seed Data Visible in Console

**Test:** Run the app on a device/emulator connected to the Firebase project. Open Firebase console and navigate to Firestore Database.
**Expected:** `products` collection has 9 documents (Espresso through Tote Bag), each with document ID matching a hardcoded UUID. `inventory` collection has 9 documents with matching IDs. All documents have `syncSource: "firestore"` and `deleted: false`.
**Why human:** Requires a live Firebase project with credentials configured, a running Android device or emulator, and Firebase console access. Cannot verify Firestore write success from static code analysis alone.

#### 2. Inventory Delta-Event Pattern Scope Confirmation

**Test:** Review whether Success Criterion 5 ("Inventory write operations use delta events rather than absolute quantity overwrites") is intentionally deferred to Phase 3.
**Expected:** The Phase 2 plans explicitly state "`InventoryItem.quantity is Int — CRDT counter mechanics are a Phase 3 repository concern, not model concern`". The `FieldValue.increment()` / Ditto CRDT counter wiring is not present in Phase 2 code — `FirestoreSeeder` uses `batch.set()` for initial writes (correct for seeding). Inventory delta writes will use `FieldValue.increment()` in Phase 3 repository code. The Phase 2 contribution to this criterion is: the `quantity: Int` field is present in the schema as the target of future delta operations, and the seed write uses `batch.set()` (not a delta, which is correct for initial population).
**Why human:** The ROADMAP phrases SC5 as a write-operation behavior, but Phase 2 scope ends before any write code (other than seeding). A human must confirm this staged delivery is intentional and that SC5 will be re-evaluated at Phase 3 verification.

---

### Commit Verification

All commits documented in SUMMARYs are present in git history:

| Commit | Description | Verified |
|--------|-------------|---------|
| `f0e2905` | feat(02-01): data model classes and collection constants | Present |
| `c72e84b` | test(02-01): unit tests for all data models | Present |
| `70d8dc7` | feat(02-02): SeedData, FirestoreSeeder, coroutines-play-services | Present |
| `0292772` | test(02-02): SeedDataTest and FirestoreSeederTest | Present |
| `4f3750e` | docs(02-02): plan metadata | Present |

---

### Build Dependency Verification

| Addition | Location | Status |
|----------|----------|--------|
| `coroutines = "1.10.2"` version | `gradle/libs.versions.toml` line 16 | VERIFIED |
| `kotlinx-coroutines-play-services` library entry | `gradle/libs.versions.toml` line 47 | VERIFIED |
| `implementation(libs.kotlinx.coroutines.play.services)` | `app/build.gradle.kts` line 115 | VERIFIED |

---

### Notable Implementation Detail: Beyond-Plan Addition

`Collections.Fields` nested object was added beyond plan spec (plan mentioned it as optional). This is a beneficial addition: 17 field name constants prevent hardcoded strings in Phase 3 bridge and repository code. No scope creep concern — harmless addition aligned with phase goal of locking the schema contract.

---

## Summary

Phase 2 goal is substantively achieved. All 14 automated must-haves pass:
- 6 model/constant source files created and substantive (no stubs)
- 7 test files created and substantive (no placeholder tests)
- All key links wired (Order → OrderLineItem, SeedData → models, FirestoreSeeder → SeedData/Collections, RepositoryModule → FirestoreSeeder)
- All 5 requirement IDs (DATA-01 through DATA-05) satisfied with evidence
- No anti-patterns detected
- All 4 documented commits verified in git history
- `kotlinx-coroutines-play-services` dependency added and wired

Two human confirmations are needed before declaring the phase fully closed: live Firestore seed verification (SC2 from ROADMAP) and explicit acceptance that inventory delta-event wiring (SC5 from ROADMAP) is correctly deferred to Phase 3.

---

_Verified: 2026-03-04_
_Verifier: Claude (gsd-verifier)_
