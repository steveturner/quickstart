---
phase: 01-foundation
plan: 01
subsystem: infra
tags: [android, gradle, ditto, firebase, kotlin, koin]

requires: []
provides:
  - Compilable Android Gradle project scaffold at pubsec/android-firebase-pos/
  - Version catalog with Ditto 4.14.3, Firebase BoM 34.10.0, Koin BoM 4.1.0, google-services 4.4.4
  - loadEnvProperties() reading from ../../.env for Ditto credentials as BuildConfig fields
  - ProGuard rule for Ditto FFI native library
  - PosApplication stub satisfying AndroidManifest requirement
affects:
  - 01-02 (Kotlin source depends on this Gradle scaffold)
  - 01-03 (DittoManager depends on BuildConfig fields created here)
  - all subsequent plans in phase 01

tech-stack:
  added:
    - ditto 4.14.3 (live.ditto:ditto)
    - firebase-bom 34.10.0 (com.google.firebase:firebase-bom)
    - firebase-firestore (BoM-managed, no explicit version)
    - koin-bom 4.1.0 (io.insert-koin:koin-bom)
    - google-services plugin 4.4.4
    - AGP 8.9.3, Kotlin 2.1.0
  patterns:
    - Version catalog (gradle/libs.versions.toml) as single source of truth for all dependency versions
    - loadEnvProperties() reading from rootProject.file("../../.env") with System.getenv() fallback for CI
    - trim('"') stripping bash-style quoted values from Properties before embedding in BuildConfig

key-files:
  created:
    - pubsec/android-firebase-pos/gradle/libs.versions.toml
    - pubsec/android-firebase-pos/settings.gradle.kts
    - pubsec/android-firebase-pos/build.gradle.kts
    - pubsec/android-firebase-pos/gradle.properties
    - pubsec/android-firebase-pos/app/build.gradle.kts
    - pubsec/android-firebase-pos/app/proguard-rules.pro
    - pubsec/android-firebase-pos/app/src/main/AndroidManifest.xml
    - pubsec/android-firebase-pos/app/src/main/java/live/ditto/pubsec/pos/PosApplication.kt
    - pubsec/android-firebase-pos/.env.sample
    - pubsec/android-firebase-pos/app/google-services.json
  modified: []

key-decisions:
  - "firebase-firestore library declared without version.ref — BoM manages version at runtime (firebase-firestore-ktx is retired in BoM 34.0.0)"
  - "stub google-services.json committed to repo so CI builds pass without requiring developer Firebase project"
  - "trim('\"') applied to Properties values because bash .env files quote values (DITTO_APP_ID=\"value\") and Java Properties.load() preserves the quotes"

patterns-established:
  - "Pattern 1: All Android library versions live in gradle/libs.versions.toml — never inline versions in build.gradle.kts"
  - "Pattern 2: BuildConfig fields sourced from .env via loadEnvProperties() with CI fallback via System.getenv()"

requirements-completed: [FOUN-04, FOUN-05]

duration: 25min
completed: 2026-03-03
---

# Phase 1 Plan 1: Gradle Scaffold Summary

**Compilable Android project scaffold for pubsec/android-firebase-pos with Ditto 4.14.3, Firebase BoM 34.10.0, and Koin BoM 4.1.0 wired to loadEnvProperties()**

## Performance

- **Duration:** ~25 min
- **Started:** 2026-03-03T08:15:00Z
- **Completed:** 2026-03-03T08:40:00Z
- **Tasks:** 2
- **Files modified:** 13

## Accomplishments
- Version catalog established with all required dependency versions (Ditto 4.14.3, Firebase BoM 34.10.0, google-services 4.4.4)
- Full Gradle project structure created: settings, root build, app build, gradle.properties
- loadEnvProperties() reads from `../../.env` (repo root) with CI System.getenv() fallback
- ProGuard rule for Ditto FFI native library in place
- `./gradlew assembleDebug` passes cleanly: BUILD SUCCESSFUL

## Task Commits

Each task was committed atomically:

1. **Task 1: Create gradle/libs.versions.toml** - `48e30ce` (chore)
2. **Task 2: Create Gradle build files, AndroidManifest, ProGuard, env sample** - `54b39e4` (feat)

## Files Created/Modified
- `pubsec/android-firebase-pos/gradle/libs.versions.toml` - Version catalog: Ditto 4.14.3, Firebase BoM 34.10.0, Koin BoM 4.1.0, google-services plugin
- `pubsec/android-firebase-pos/settings.gradle.kts` - Project settings with Ditto maven repo
- `pubsec/android-firebase-pos/build.gradle.kts` - Root build file, all plugins applied false
- `pubsec/android-firebase-pos/gradle.properties` - android.useAndroidX=true and JVM args
- `pubsec/android-firebase-pos/app/build.gradle.kts` - App build with loadEnvProperties() and 4 Ditto BuildConfig fields
- `pubsec/android-firebase-pos/app/proguard-rules.pro` - Ditto FFI keep rule
- `pubsec/android-firebase-pos/app/src/main/AndroidManifest.xml` - Permissions + PosApplication + MainActivity
- `pubsec/android-firebase-pos/app/src/main/java/live/ditto/pubsec/pos/PosApplication.kt` - Stub Application class
- `pubsec/android-firebase-pos/.env.sample` - Documents credentials and google-services.json requirement
- `pubsec/android-firebase-pos/app/google-services.json` - Stub for CI builds (placeholder project ID)
- `pubsec/android-firebase-pos/gradle/wrapper/` - Copied from android-kotlin/QuickStartTasks

## Decisions Made
- `firebase-firestore` declared without `version.ref` — BoM manages the version; `firebase-firestore-ktx` was not used because that artifact was retired in BoM 34.0.0
- Stub `google-services.json` committed to allow CI builds to pass without a real Firebase project configured; real project uses their own from Firebase Console
- `propVal()` helper added to `trim('"')` from Properties values because the `.env` file uses bash-style `KEY="value"` quoting which Java Properties.load() preserves as part of the value

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Added gradle.properties with android.useAndroidX=true**
- **Found during:** Task 2 (assembleDebug verification)
- **Issue:** Gradle build failed: "android.useAndroidX property is not enabled" — AndroidX dependencies detected but flag not set
- **Fix:** Created `gradle.properties` with `android.useAndroidX=true` and standard JVM/Kotlin settings
- **Files modified:** pubsec/android-firebase-pos/gradle.properties
- **Verification:** Build proceeded past checkDebugAarMetadata after fix
- **Committed in:** 54b39e4 (Task 2 commit)

**2. [Rule 3 - Blocking] Created stub google-services.json**
- **Found during:** Task 2 (assembleDebug verification)
- **Issue:** Build failed: "File google-services.json is missing. The Google Services Plugin cannot function without it."
- **Fix:** Created placeholder google-services.json with dummy project ID / API key so the plugin can proceed
- **Files modified:** pubsec/android-firebase-pos/app/google-services.json
- **Verification:** processDebugGoogleServices task passed
- **Committed in:** 54b39e4 (Task 2 commit)

**3. [Rule 1 - Bug] Fixed double-quoted BuildConfig strings**
- **Found during:** Task 2 (assembleDebug verification)
- **Issue:** CompileDebugJavaWithJavac failed with `""value""` — the .env file uses bash quoting (`KEY="value"`), Java Properties.load() preserves the quotes, and the build script added a second layer of quotes
- **Fix:** Added `fun propVal(key: String) = prop[key].toString().trim('"')` to strip surrounding quotes before embedding in BuildConfig string literal
- **Files modified:** pubsec/android-firebase-pos/app/build.gradle.kts
- **Verification:** BUILD SUCCESSFUL after fix
- **Committed in:** 54b39e4 (Task 2 commit)

---

**Total deviations:** 3 auto-fixed (2 blocking, 1 bug)
**Impact on plan:** All three fixes required for the build to compile. No scope creep.

## Issues Encountered
- Three Gradle build failures addressed sequentially: missing gradle.properties, missing google-services.json, double-quoted BuildConfig values. All resolved within Task 2.

## User Setup Required
None — no external service configuration required beyond what is documented in `.env.sample`.

## Next Phase Readiness
- Gradle scaffold is complete and compiles clean
- Plan 02 can add Kotlin source files (data models, ViewModels, UI) without any Gradle changes
- Developers need to replace `app/google-services.json` with their real Firebase project file before targeting a real Firebase backend

---
*Phase: 01-foundation*
*Completed: 2026-03-03*
