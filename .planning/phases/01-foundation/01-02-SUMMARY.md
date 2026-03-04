---
phase: 01-foundation
plan: 02
subsystem: infra
tags: [koin, ditto, firebase, firestore, android, dependency-injection]

requires:
  - phase: 01-foundation/01-01
    provides: Gradle dependencies (Koin, Ditto, Firebase BoM), BuildConfig fields, AndroidManifest with PosApplication

provides:
  - Koin DI container started in Application.onCreate()
  - Ditto singleton initialized with OnlinePlayground identity, websocket URL, disableSyncWithV3()
  - FirebaseFirestore singleton with memoryCacheSettings{} (offline cache disabled)
  - Empty stub modules (repositoryModule, viewModelModule) ready for Phase 2/3

affects: [01-foundation/01-03, 02-repositories, 03-viewmodels]

tech-stack:
  added: []
  patterns:
    - "Koin single{} for lazy singletons — Ditto eagerly resolved on IO dispatcher at app start"
    - "memoryCacheSettings{} instead of deprecated isPersistenceEnabled=false for Firestore offline cache control"
    - "Firebase KTX imports via com.google.firebase.Firebase (bundled in firebase-firestore since BoM 34.x, not separate -ktx artifact)"

key-files:
  created:
    - pubsec/android-firebase-pos/app/src/main/java/live/ditto/pubsec/pos/di/AppModule.kt
    - pubsec/android-firebase-pos/app/src/main/java/live/ditto/pubsec/pos/di/RepositoryModule.kt
    - pubsec/android-firebase-pos/app/src/main/java/live/ditto/pubsec/pos/di/ViewModelModule.kt
  modified:
    - pubsec/android-firebase-pos/app/src/main/java/live/ditto/pubsec/pos/PosApplication.kt

key-decisions:
  - "Firebase import path is com.google.firebase.Firebase (not com.google.firebase.ktx.Firebase) — KTX bundled in main artifact since BoM 34.0.0"
  - "ditto.startSync() deferred to Phase 3 — no subscriptions exist yet, calling it here would be premature"
  - "Firebase.firestore resolved lazily (not eagerly) — firestoreSettings applied inside Koin single{} before first use"

patterns-established:
  - "DI pattern: all SDK singletons declared in AppModule; feature modules (repository, viewmodel) in separate stub modules"
  - "IO init pattern: Ditto eagerly resolved on ioScope to avoid StrictMode violations on main thread"

requirements-completed: [FOUN-01, FOUN-02, FOUN-03]

duration: 2min
completed: 2026-03-04
---

# Phase 1 Plan 02: DI Modules and Application Init Summary

**Koin DI wired in PosApplication with Ditto (OnlinePlayground + websocket + disableSyncWithV3) and Firestore (memoryCacheSettings) singletons**

## Performance

- **Duration:** 2 min
- **Started:** 2026-03-04T08:43:32Z
- **Completed:** 2026-03-04T08:46:03Z
- **Tasks:** 2
- **Files modified:** 4

## Accomplishments
- AppModule provides Ditto and FirebaseFirestore singletons under Koin
- PosApplication starts Koin and eagerly resolves Ditto on IO dispatcher
- RepositoryModule and ViewModelModule are valid empty stubs Koin can load

## Task Commits

Each task was committed atomically:

1. **Task 1: Create Koin DI modules** - `57b70c9` (feat)
2. **Task 2: Implement PosApplication** - `8d30188` (feat)

**Plan metadata:** (docs commit follows)

## Files Created/Modified
- `pubsec/android-firebase-pos/app/src/main/java/live/ditto/pubsec/pos/di/AppModule.kt` - Koin appModule with Ditto and FirebaseFirestore singletons
- `pubsec/android-firebase-pos/app/src/main/java/live/ditto/pubsec/pos/di/RepositoryModule.kt` - Empty stub for Phase 2
- `pubsec/android-firebase-pos/app/src/main/java/live/ditto/pubsec/pos/di/ViewModelModule.kt` - Empty stub for Phase 3
- `pubsec/android-firebase-pos/app/src/main/java/live/ditto/pubsec/pos/PosApplication.kt` - Full Application class with Koin startup and eager Ditto init

## Decisions Made
- Firebase KTX extension functions are bundled in `firebase-firestore` since BoM 34.0.0 — correct import is `com.google.firebase.Firebase` and `com.google.firebase.firestore.firestore`, NOT the deprecated `com.google.firebase.ktx.Firebase` / `com.google.firebase.firestore.ktx.firestore` paths

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Fixed Firebase import paths for BoM 34.x**
- **Found during:** Task 1 (Create Koin DI modules)
- **Issue:** Plan specified `com.google.firebase.ktx.Firebase` and `com.google.firebase.firestore.ktx.firestore` imports — these are legacy KTX paths that don't exist as separate packages in BoM 34.x where KTX is bundled into the main artifact
- **Fix:** Changed to `com.google.firebase.Firebase` and `com.google.firebase.firestore.firestore`
- **Files modified:** di/AppModule.kt
- **Verification:** `./gradlew assembleDebug` exits 0
- **Committed in:** `57b70c9` (Task 1 commit)

---

**Total deviations:** 1 auto-fixed (Rule 1 - import path bug)
**Impact on plan:** Required fix for compilation. No scope creep.

## Issues Encountered
- Firebase KTX import paths in plan were for the legacy split-artifact pattern. BoM 34.x bundles KTX into the main artifact with different import paths.

## Next Phase Readiness
- DI container is fully wired and buildable
- Plan 01-03 can now test/smoke-screen this initialization
- Phase 2 can add repository bindings to repositoryModule
- Phase 3 can add ViewModel bindings to viewModelModule

---
*Phase: 01-foundation*
*Completed: 2026-03-04*
