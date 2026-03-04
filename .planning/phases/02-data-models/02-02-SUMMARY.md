---
phase: 02-data-models
plan: 02
subsystem: database
tags: [firestore, seed-data, coroutines, koin, mockk, kotlin]

# Dependency graph
requires:
  - phase: 02-data-models-01
    provides: Product, InventoryItem, Collections data classes from Plan 01
provides:
  - SeedData object with 9 hardcoded coffee shop products and matching inventory items
  - FirestoreSeeder with empty-check guard and batch write using canonical UUID pattern
  - FirestoreSeeder wired into Koin DI via RepositoryModule
  - kotlinx-coroutines-play-services dependency for Task<T>.await() support
affects: [03-sync-bridge, phase-03, bridge, firestore-reads, seeder-call]

# Tech tracking
tech-stack:
  added: [kotlinx-coroutines-play-services:1.10.2]
  patterns:
    - "Canonical UUID pattern: product._id IS the Firestore document ID"
    - "Empty-check guard: check collection.limit(1) before batch write"
    - "Seed data with syncSource='firestore' prevents phantom bridge writes on first launch"
    - "Hardcoded UUIDs in SeedData (not UUID.randomUUID()) — deterministic re-runs"
    - "mockkStatic('kotlinx.coroutines.tasks.TasksKt') for mocking Task<T>.await()"

key-files:
  created:
    - pubsec/android-firebase-pos/app/src/main/java/live/ditto/pubsec/pos/data/seed/SeedData.kt
    - pubsec/android-firebase-pos/app/src/main/java/live/ditto/pubsec/pos/data/seed/FirestoreSeeder.kt
    - pubsec/android-firebase-pos/app/src/test/java/live/ditto/pubsec/pos/SeedDataTest.kt
    - pubsec/android-firebase-pos/app/src/test/java/live/ditto/pubsec/pos/FirestoreSeederTest.kt
  modified:
    - pubsec/android-firebase-pos/gradle/libs.versions.toml
    - pubsec/android-firebase-pos/app/build.gradle.kts
    - pubsec/android-firebase-pos/app/src/main/java/live/ditto/pubsec/pos/di/RepositoryModule.kt

key-decisions:
  - "FirestoreSeeder uses batch.set() with product._id as Firestore document ID — NOT .add() with auto-generated IDs"
  - "SeedData UUIDs are hardcoded string literals — deterministic IDs survive app restart without re-seeding"
  - "coJustRun used for Task<Void>.await() mocking — coEvery returns null fails Kotlin non-null check on Void"

patterns-established:
  - "SeedData pattern: hardcoded UUIDs, syncSource='firestore', deleted=false — canonical initial state"
  - "FirestoreSeeder pattern: limit(1) empty-check, then batch set, then batch commit"

requirements-completed: [DATA-01, DATA-03, DATA-04, DATA-05]

# Metrics
duration: 3min
completed: 2026-03-04
---

# Phase 2 Plan 02: Seed Data and Firestore Seeder Summary

**Coffee shop SeedData catalog (9 items across Beverage/Food/Merchandise) with FirestoreSeeder batch writer using canonical UUID-as-document-ID pattern and empty-check guard**

## Performance

- **Duration:** 3 min
- **Started:** 2026-03-04T18:16:30Z
- **Completed:** 2026-03-04T18:19:56Z
- **Tasks:** 2
- **Files modified:** 7

## Accomplishments

- SeedData object provides 9 hardcoded coffee shop products with matching inventory items; all UUIDs are string literals (not runtime-generated), all have syncSource="firestore" to prevent phantom bridge writes
- FirestoreSeeder.seedIfEmpty() checks products collection via limit(1) before batch writing products and inventory items, using product._id as the Firestore document ID
- RepositoryModule updated from empty stub to provide FirestoreSeeder via Koin single{}
- kotlinx-coroutines-play-services 1.10.2 added so Task<T>.await() works in suspend functions

## Task Commits

Each task was committed atomically:

1. **Task 1: SeedData, FirestoreSeeder, dependency, RepositoryModule** - `70d8dc7` (feat)
2. **Task 2: SeedDataTest and FirestoreSeederTest** - `0292772` (test)

**Plan metadata:** (docs commit follows)

_Note: TDD tasks produced two commits — implementation then tests_

## Files Created/Modified

- `data/seed/SeedData.kt` - Object with 9 products (Espresso, Latte, Cappuccino, Cold Brew, Croissant, Turkey Sandwich, Blueberry Muffin, Coffee Mug, Tote Bag) and 9 matching InventoryItems; all hardcoded UUIDs; syncSource="firestore"
- `data/seed/FirestoreSeeder.kt` - Suspend fun seedIfEmpty() with limit(1) guard and batch write
- `di/RepositoryModule.kt` - Updated from empty stub: single { FirestoreSeeder(get()) }
- `gradle/libs.versions.toml` - Added coroutines = "1.10.2" version and kotlinx-coroutines-play-services library
- `app/build.gradle.kts` - Added implementation(libs.kotlinx.coroutines.play.services)
- `test/.../SeedDataTest.kt` - 12 data integrity tests: count, uniqueness, prices, syncSource, deleted flag, categories, inventory consistency
- `test/.../FirestoreSeederTest.kt` - 5 mockk tests: empty-check guard, batch write, skip when non-empty, canonical ID for products, canonical ID for inventory

## Decisions Made

- `coJustRun` used instead of `coEvery { }.returns(null)` for `Task<Void>.await()` mocking — Kotlin's non-null type system rejects null for `Void`, so `coJustRun` is the correct mockk idiom
- `mockkStatic("kotlinx.coroutines.tasks.TasksKt")` established as the pattern for mocking Firestore Task<T>.await() extension functions in JVM unit tests

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Fixed Task<Void>.await() mockk null return type error**
- **Found during:** Task 2 (FirestoreSeederTest compilation)
- **Issue:** `coEvery { mockCommitTask.await() } returns null` fails because Task<Void>.await() in kotlinx-coroutines returns Unit not Void, and Kotlin rejects null for non-null Void
- **Fix:** Replaced with `coJustRun { mockCommitTask.await() }` which correctly stubs suspend functions returning Unit
- **Files modified:** FirestoreSeederTest.kt
- **Verification:** Test compilation succeeded; all 5 FirestoreSeederTest tests passed
- **Committed in:** 0292772 (Task 2 commit)

---

**Total deviations:** 1 auto-fixed (Rule 1 - bug in test setup)
**Impact on plan:** Necessary correction for correct test behavior. No scope creep.

## Issues Encountered

None beyond the mockk Task<Void> type issue documented above.

## Next Phase Readiness

- SeedData and FirestoreSeeder are ready; Phase 3 bridge code can call seeder.seedIfEmpty() from MainActivity or Application.onCreate()
- Canonical UUID pattern established: product._id must be frozen before writing — confirmed working
- RepositoryModule is no longer empty; Phase 3 can add repository classes alongside FirestoreSeeder
- All 17 unit tests pass (10 from Plan 01 + 5 FirestoreSeederTest + 12 SeedDataTest = correct count per test result XMLs)

---
*Phase: 02-data-models*
*Completed: 2026-03-04*
