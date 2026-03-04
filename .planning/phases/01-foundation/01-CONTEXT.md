# Phase 1: Foundation - Context

**Gathered:** 2026-03-03
**Status:** Ready for planning

<domain>
## Phase Boundary

A running Android app with Ditto and Firebase/Firestore initialized, Koin DI wired, and ProGuard configured. No feature code — just the structural foundation that all subsequent phases build on. Requirements: FOUN-01 through FOUN-05.

</domain>

<decisions>
## Implementation Decisions

### Project location and structure
- New app lives at `pubsec/android-firebase-pos/` (consistent with other pubsec sub-projects)
- Self-contained Gradle project (own `build.gradle.kts`, `settings.gradle.kts`, `libs.versions.toml`)
- Package name: `live.ditto.pubsec.pos`
- Namespace: `live.ditto.pubsec.pos`

### Ditto initialization pattern
- Follow existing quickstart pattern: Ditto singleton initialized in `Application.onCreate()` with `applicationContext`
- Use Koin `single { }` module to provide Ditto instance (not companion object like existing quickstart — Koin manages the lifecycle)
- `enableDittoCloudSync = false` with custom websocket URL from env (matches existing pattern)
- `disableSyncWithV3()` required for DQL
- Ditto SDK version 4.14.3 (critical upgrade from existing 4.13.1 for P2P deadlock fix)

### Firebase/Firestore initialization
- Firebase Android BoM 34.10.0
- `google-services.json` placed in `app/` directory (standard Firebase setup)
- Firestore offline persistence explicitly disabled: `FirebaseFirestore.getInstance().firestoreSettings = firestoreSettings { isPersistenceEnabled = false }`
- No `-ktx` artifacts (KTX merged into base modules since BoM 34.0.0)
- google-services Gradle plugin 4.4.4

### Dependency injection
- Koin BOM 4.1.0 (matches existing quickstart)
- Modules: `appModule` (Ditto, Firestore instances), `repositoryModule` (empty stubs for Phase 2), `viewModelModule` (empty stubs for Phase 3)
- Koin started in Application.onCreate() after Ditto init

### Build configuration
- Copy `libs.versions.toml` from existing android-kotlin quickstart as baseline
- Add Firebase BoM 34.10.0, google-services plugin
- Update Ditto from 4.13.1 → 4.14.3
- minSdk = 23 (Ditto requirement), targetSdk = 35, compileSdk = 35
- ProGuard keep rule: `-keep class live.ditto.internal.swig.ffi.** { *; }`
- `loadEnvProperties()` function copied from existing quickstart for Ditto credentials
- `buildConfig = true` for BuildConfig access to env vars

### Environment variables
- Ditto credentials from `.env` file (same as existing quickstart): DITTO_APP_ID, DITTO_PLAYGROUND_TOKEN, DITTO_AUTH_URL, DITTO_WEBSOCKET_URL
- Firebase uses `google-services.json` (not env vars)
- Include a `.env.sample` and document Firebase setup in README

### Claude's Discretion
- Exact Compose theme colors and typography
- Application class name (suggest `PosApplication`)
- Whether to include a minimal "hello world" Compose screen or just a blank scaffold
- Gradle wrapper version

</decisions>

<specifics>
## Specific Ideas

- The app should feel like a natural sibling to the existing `android-kotlin/QuickStartTasks` — same structural patterns, familiar to anyone who's seen the quickstart
- Firestore offline persistence MUST be disabled from the start — Ditto is the offline store, not Firestore's built-in cache
- Include a simple scaffold screen that shows "Ditto: initialized / Firestore: connected" as a smoke test for Phase 1 completion

</specifics>

<code_context>
## Existing Code Insights

### Reusable Assets
- `loadEnvProperties()` in `build.gradle.kts`: Reads `.env` file for Ditto credentials — copy directly
- `libs.versions.toml`: Version catalog with AGP 8.9.3, Kotlin 2.1.0, Koin 4.1.0, Compose BOM — use as baseline
- `TasksApplication.kt`: Ditto init pattern with `DefaultAndroidDittoDependencies`, `OnlinePlayground` identity, coroutine scope — adapt for Koin
- `DittoHandler.kt`: Singleton pattern — replace with Koin `single { }` for cleaner DI

### Established Patterns
- MVVM with ViewModels + Compose screens (e.g., `TasksListScreenViewModel` + `TasksListScreen`)
- Koin for DI with compose integration (`koin-androidx-compose`, `koin-androidx-compose-navigation`)
- Navigation Compose for routing
- BuildConfig fields for environment variables
- Ditto init in Application.onCreate() with IO dispatcher coroutine scope

### Integration Points
- `.env` file at repository root (shared across all quickstarts)
- `google-services.json` in app/ directory (Firebase-specific, not shared)
- No shared code between quickstarts — each is self-contained

</code_context>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope

</deferred>

---

*Phase: 01-foundation*
*Context gathered: 2026-03-03*
