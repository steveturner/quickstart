# Phase 1: Foundation - Research

**Researched:** 2026-03-03
**Domain:** Android project scaffolding — Ditto SDK 4.14.3, Firebase BoM 34.10.0, Koin 4.1.0 DI, ProGuard
**Confidence:** HIGH

---

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

- New app lives at `pubsec/android-firebase-pos/` (consistent with other pubsec sub-projects)
- Self-contained Gradle project (own `build.gradle.kts`, `settings.gradle.kts`, `libs.versions.toml`)
- Package name: `live.ditto.pubsec.pos`
- Namespace: `live.ditto.pubsec.pos`
- Follow existing quickstart pattern: Ditto singleton initialized in `Application.onCreate()` with `applicationContext`
- Use Koin `single { }` module to provide Ditto instance (not companion object like existing quickstart)
- `enableDittoCloudSync = false` with custom websocket URL from env (matches existing pattern)
- `disableSyncWithV3()` required for DQL
- Ditto SDK version 4.14.3 (critical upgrade from existing 4.13.1 for P2P deadlock fix)
- Firebase Android BoM 34.10.0
- `google-services.json` placed in `app/` directory (standard Firebase setup)
- Firestore offline persistence explicitly disabled: `firestoreSettings { setLocalCacheSettings(memoryCacheSettings {}) }`
- No `-ktx` artifacts (KTX merged into base modules since BoM 34.0.0)
- google-services Gradle plugin 4.4.4
- Koin BOM 4.1.0 (matches existing quickstart)
- Modules: `appModule` (Ditto, Firestore instances), `repositoryModule` (empty stubs for Phase 2), `viewModelModule` (empty stubs for Phase 3)
- Koin started in Application.onCreate() after Ditto init
- Copy `libs.versions.toml` from existing android-kotlin quickstart as baseline
- Add Firebase BoM 34.10.0, google-services plugin
- Update Ditto from 4.13.1 → 4.14.3
- minSdk = 23 (Ditto requirement), targetSdk = 35, compileSdk = 35
- ProGuard keep rule: `-keep class live.ditto.internal.swig.ffi.** { *; }`
- `loadEnvProperties()` function copied from existing quickstart for Ditto credentials
- `buildConfig = true` for BuildConfig access to env vars
- Ditto credentials from `.env` file: DITTO_APP_ID, DITTO_PLAYGROUND_TOKEN, DITTO_AUTH_URL, DITTO_WEBSOCKET_URL
- Firebase uses `google-services.json` (not env vars)
- Include a `.env.sample` and document Firebase setup in README

### Claude's Discretion

- Exact Compose theme colors and typography
- Application class name (suggest `PosApplication`)
- Whether to include a minimal "hello world" Compose screen or just a blank scaffold
- Gradle wrapper version

### Deferred Ideas (OUT OF SCOPE)

None — discussion stayed within phase scope
</user_constraints>

---

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|-----------------|
| FOUN-01 | Android app initializes Ditto SDK with Online Playground identity in Application.onCreate() | TasksApplication.kt pattern + Koin single {} adaptation documented |
| FOUN-02 | Android app initializes Firebase/Firestore with offline persistence disabled | Firebase BoM 34.10.0 setup + memoryCacheSettings{} pattern verified |
| FOUN-03 | App uses Koin DI for dependency injection consistent with repo patterns | Koin 4.1.0 startKoin pattern + module structure documented |
| FOUN-04 | Gradle build uses version catalogs (libs.versions.toml) consistent with repo conventions | Baseline catalog from android-kotlin quickstart documented with additions |
| FOUN-05 | ProGuard/R8 rules configured for Ditto SDK in release builds | Official keep rule verified via Ditto docs |
</phase_requirements>

---

## Summary

This phase creates `pubsec/android-firebase-pos/` as a self-contained Android project. No shared code exists with other quickstarts — everything must be created from scratch, though the android-kotlin QuickStartTasks project provides copy-ready patterns. The key difference from the existing quickstart is replacing the companion-object Ditto singleton (`DittoHandler`) with Koin-managed dependency injection, and adding Firebase/Firestore initialization alongside Ditto.

The technology choices are fully locked by CONTEXT.md. Ditto 4.14.3 is verified on Maven Central. Firebase BoM 34.10.0 is current (released February 26, 2026). Koin 4.1.0 is stable. The `loadEnvProperties()` function and `libs.versions.toml` can be copied from `android-kotlin/QuickStartTasks/` with targeted modifications. ProGuard rules are confirmed via official Ditto documentation.

The main risk is ordering: Koin modules must be defined before `startKoin` is called, Ditto init should run off the main thread (IO dispatcher), and Firestore settings must be applied before first use. The smoke-test Compose screen that shows "Ditto: initialized / Firestore: connected" is the right scope for Phase 1 completion.

**Primary recommendation:** Copy `android-kotlin/QuickStartTasks/` structure as the scaffold, add Firebase BoM + google-services plugin, replace DittoHandler companion object with Koin modules, disable Firestore persistence, add ProGuard rule.

---

## Standard Stack

### Core

| Library | Version | Purpose | Why Standard |
|---------|---------|---------|--------------|
| Ditto SDK | 4.14.3 | P2P edge sync | Required; 4.14.3 fixes P2P deadlock present in 4.13.1 |
| Firebase BoM | 34.10.0 | Firebase version management | Latest stable BoM (Feb 26, 2026) |
| firebase-firestore | managed by BoM (~26.1.1) | Cloud database | No KTX suffix needed since BoM 34.0.0 |
| Koin BoM | 4.1.0 | DI framework | Already used in repo; matches existing quickstart |
| koin-android | managed by Koin BoM | Android application/ViewModel DI | Provides `startKoin`, `androidContext`, KoinApplication |
| koin-androidx-compose | managed by Koin BoM | Compose integration | Provides `koinViewModel()` injection in composables |
| koin-androidx-compose-navigation | managed by Koin BoM | Nav-scoped ViewModels | Consistent with quickstart pattern |
| Compose BoM | 2025.07.00 | Compose version management | Already in libs.versions.toml baseline |
| Navigation Compose | 2.9.2 | In-app navigation | Already in libs.versions.toml baseline |
| AGP | 8.9.3 | Android Gradle Plugin | Already in libs.versions.toml baseline |
| Kotlin | 2.1.0 | Language | Already in libs.versions.toml baseline |
| google-services plugin | 4.4.4 | Processes google-services.json | Latest; required by Firebase |

### Supporting

| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| androidx.core-ktx | 1.16.0 | Android core extensions | Standard Android; in baseline |
| lifecycle-runtime-ktx | 2.9.2 | Lifecycle-aware coroutines | ViewModels; in baseline |
| activity-compose | 1.10.1 | setContent / rememberLauncherForActivityResult | Entry point for Compose |
| kotlinx-coroutines-android | via lifecycle | Coroutine dispatchers | Ditto init on IO dispatcher |

### Alternatives Considered

| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| Koin | Hilt | Hilt requires KAPT/KSP setup; Koin is already in repo; no reason to change |
| Firebase BoM 34.10.0 | Earlier BoM | 34.0.0+ is where KTX modules were retired; stay on latest |
| memoryCacheSettings{} | isPersistenceEnabled = false | Both work; `memoryCacheSettings{}` is the current recommended API |

**Installation:**
```bash
# No npm/cargo — Gradle handles everything via libs.versions.toml + build.gradle.kts
# Ensure google-services.json is placed at pubsec/android-firebase-pos/app/google-services.json
```

---

## Architecture Patterns

### Recommended Project Structure

```
pubsec/android-firebase-pos/
├── app/
│   ├── src/
│   │   └── main/
│   │       ├── java/live/ditto/pubsec/pos/
│   │       │   ├── PosApplication.kt      # Application class, Ditto + Koin init
│   │       │   ├── MainActivity.kt        # ComponentActivity, setContent, permissions
│   │       │   ├── Root.kt                # Compose NavHost entry point
│   │       │   ├── di/
│   │       │   │   ├── AppModule.kt       # single { ditto }, single { firestore }
│   │       │   │   ├── RepositoryModule.kt  # empty stubs (Phase 2)
│   │       │   │   └── ViewModelModule.kt   # empty stubs (Phase 3)
│   │       │   └── ui/
│   │       │       ├── StatusScreen.kt    # Smoke-test: Ditto + Firestore status
│   │       │       └── theme/
│   │       │           ├── Color.kt
│   │       │           ├── Theme.kt
│   │       │           └── Type.kt
│   │       ├── res/
│   │       └── AndroidManifest.xml
│   ├── google-services.json               # Firebase config — NOT committed to git
│   ├── proguard-rules.pro                 # Ditto keep rule
│   └── build.gradle.kts
├── gradle/
│   └── libs.versions.toml
├── build.gradle.kts
├── settings.gradle.kts
├── .env.sample                            # Documents required env vars
└── README.md
```

### Pattern 1: Ditto Initialization via Koin (adapted from TasksApplication.kt)

**What:** Initialize Ditto in the Application class on the IO dispatcher, provide it via a Koin `single {}` module.
**When to use:** Any Android project using Ditto + Koin. Replaces the companion-object `DittoHandler` pattern.

```kotlin
// PosApplication.kt
class PosApplication : Application() {

    private val ioScope = CoroutineScope(SupervisorJob() + Dispatchers.IO)

    override fun onCreate() {
        super.onCreate()
        // 1. Start Koin with the module list
        startKoin {
            androidLogger()
            androidContext(this@PosApplication)
            modules(appModule, repositoryModule, viewModelModule)
        }
        // 2. Trigger Ditto init (Koin resolves it lazily on first get(), or eager init here)
        ioScope.launch {
            // Eager resolution ensures Ditto is ready before first screen renders
            get<Ditto>()
        }
    }
}
```

```kotlin
// di/AppModule.kt
// Source: Koin official docs — https://insert-koin.io/docs/reference/koin-android/start/
val appModule = module {

    single {
        val androidDependencies = DefaultAndroidDittoDependencies(androidContext())
        val ditto = Ditto(
            androidDependencies,
            DittoIdentity.OnlinePlayground(
                dependencies = androidDependencies,
                appId = BuildConfig.DITTO_APP_ID,
                token = BuildConfig.DITTO_PLAYGROUND_TOKEN,
                customAuthUrl = BuildConfig.DITTO_AUTH_URL,
                enableDittoCloudSync = false
            )
        )
        ditto.updateTransportConfig { config ->
            config.connect.websocketUrls.add(BuildConfig.DITTO_WEBSOCKET_URL)
        }
        // Required for DQL
        ditto.disableSyncWithV3()
        ditto
    }

    single {
        val db = Firebase.firestore
        db.firestoreSettings = firestoreSettings {
            // Ditto is the offline store — disable Firestore's built-in cache
            setLocalCacheSettings(memoryCacheSettings {})
        }
        db
    }
}
```

**Note:** `ditto.store.execute("ALTER SYSTEM SET DQL_STRICT_MODE = false")` is a suspend call — if needed it must run inside a coroutine, not inside the `single {}` block directly. Skip for Phase 1; add in Phase 2/3 when DQL is used.

### Pattern 2: Koin BoM Declaration in libs.versions.toml

```toml
# gradle/libs.versions.toml additions on top of quickstart baseline
[versions]
# ... existing versions ...
koin-bom = "4.1.0"           # already present in baseline
ditto = "4.14.3"             # CHANGE from 4.13.1
firebase-bom = "34.10.0"     # ADD
google-services = "4.4.4"    # ADD

[libraries]
# ... existing libraries ...
firebase-bom = { group = "com.google.firebase", name = "firebase-bom", version.ref = "firebase-bom" }
firebase-firestore = { group = "com.google.firebase", name = "firebase-firestore" }

[plugins]
# ... existing plugins ...
google-services = { id = "com.google.gms.google-services", version.ref = "google-services" }
```

### Pattern 3: Firestore Persistence Disabled (Kotlin DSL)

```kotlin
// Source: Firebase Firestore docs — firebase.google.com/docs/firestore/manage-data/enable-offline
val db = Firebase.firestore
db.firestoreSettings = firestoreSettings {
    setLocalCacheSettings(memoryCacheSettings {})
}
```

This uses the current API (BoM 34.x) rather than the deprecated `isPersistenceEnabled = false`. Both work, but `memoryCacheSettings {}` is the recommended approach for new code.

### Pattern 4: google-services Plugin (Kotlin DSL)

```kotlin
// root build.gradle.kts
plugins {
    alias(libs.plugins.android.application) apply false
    alias(libs.plugins.jetbrains.kotlin.android) apply false
    alias(libs.plugins.compose.compiler) apply false
    alias(libs.plugins.google.services) apply false   // ADD
}

// app/build.gradle.kts
plugins {
    alias(libs.plugins.android.application)
    alias(libs.plugins.jetbrains.kotlin.android)
    alias(libs.plugins.compose.compiler)
    alias(libs.plugins.google.services)               // ADD
}
```

### Pattern 5: loadEnvProperties() (copy from existing quickstart)

The function reads the `.env` file relative to the repo root. For the new project at `pubsec/android-firebase-pos/`, the path must be:

```kotlin
// app/build.gradle.kts — adjust rootProject traversal depth
val envFile = rootProject.file("../../.env")   // same depth as android-kotlin/QuickStartTasks
```

This matches exactly because `pubsec/android-firebase-pos/` and `android-kotlin/QuickStartTasks/` are both two levels below the repo root.

### Anti-Patterns to Avoid

- **Calling Firestore before settings applied:** `Firebase.firestore` must have `firestoreSettings` applied before any reads/writes. Do it immediately after obtaining the instance in the Koin module.
- **Initializing Ditto on main thread:** The `Ditto()` constructor and `disableSyncWithV3()` should run off the main thread. The Koin `single {}` block runs on whichever thread first calls `get<Ditto>()` — ensure the first eager resolution is from an IO-scoped coroutine.
- **Using DittoHandler companion object pattern:** This phase replaces that pattern with Koin. Do not create a `DittoHandler.kt` file.
- **Adding KTX Firebase suffixes:** `firebase-firestore-ktx` does not exist as a separate artifact since BoM 34.0.0. Use `firebase-firestore` only.
- **Committing google-services.json:** This file contains Firebase project credentials and must be in `.gitignore`.

---

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Dependency injection | Custom ServiceLocator / companion objects | Koin `single {}` modules | Lifecycle correctness, testability, established repo pattern |
| Firebase version management | Manual version strings per artifact | Firebase BoM `platform()` | BoM guarantees inter-library compatibility |
| Env var loading | Custom Properties reader | `loadEnvProperties()` (copy from quickstart) | Already handles CI fallback to `System.getenv()` |
| Bluetooth/WiFi permission requests | Custom permission dialog | `DittoSyncPermissions(this).missingPermissions()` | Handles all Ditto-required permissions across API levels |
| Firestore offline cache | Custom caching logic | Disable Firestore cache entirely; Ditto is the offline store | Prevents two competing caches; simplifies sync story |

**Key insight:** Everything in this phase is wiring infrastructure together — zero custom algorithms. Every problem has a library-level solution.

---

## Common Pitfalls

### Pitfall 1: Ditto Store Execute on Main Thread

**What goes wrong:** `ditto.store.execute("ALTER SYSTEM SET ...")` is a suspend function. Calling it from a non-coroutine context or on the main thread causes ANR or compile error.
**Why it happens:** Developers copy-paste init code without checking dispatch context.
**How to avoid:** All `ditto.store.execute()` calls must be inside `withContext(Dispatchers.IO)` or a coroutine launched on the IO scope.
**Warning signs:** StrictMode DiskRead violations in Logcat; `Suspend function called from non-coroutine` compile error.

### Pitfall 2: Firestore Settings Applied After First Use

**What goes wrong:** `FirebaseFirestore.getInstance().firestoreSettings = ...` throws `IllegalStateException: FirebaseFirestore has already been started` if any Firestore call happened first.
**Why it happens:** Firebase lazily initializes; anything that touches the instance before settings are set locks the config.
**How to avoid:** Apply settings inside the Koin `single { }` block, immediately after `Firebase.firestore`. Because Koin resolves lazily on first `get<FirebaseFirestore>()`, settings will always be applied before any repository code runs.
**Warning signs:** `IllegalStateException` crash at app startup.

### Pitfall 3: google-services.json Missing or Wrong Location

**What goes wrong:** Build succeeds but Firebase throws `FirebaseApp is not initialized` at runtime.
**Why it happens:** `google-services.json` must be at `app/google-services.json`, not project root.
**How to avoid:** Verify file exists at `pubsec/android-firebase-pos/app/google-services.json` before first build.
**Warning signs:** `com.google.firebase.FirebaseApp is not initialized` in Logcat.

### Pitfall 4: DQL_STRICT_MODE ALTER SYSTEM in Koin Single Block

**What goes wrong:** `ditto.store.execute(...)` is a `suspend` function but Koin's `single {}` block is not a coroutine scope. Calling it directly causes a compile error.
**Why it happens:** Koin module blocks are synchronous; they cannot call suspend functions.
**How to avoid:** Either (a) skip `ALTER SYSTEM SET DQL_STRICT_MODE = false` for Phase 1 since no DQL queries run yet, or (b) use `runBlocking(Dispatchers.IO)` inside the single block with awareness it blocks the calling thread briefly. Defer to Phase 3 when DQL is actually used.
**Warning signs:** Kotlin compile error: "Suspend function 'execute' should be called only from a coroutine or another suspend function."

### Pitfall 5: Version Catalog Entry Without `version.ref` for BoM-Managed Libraries

**What goes wrong:** Adding `version.ref = "firebase-bom"` to `firebase-firestore` entry in `libs.versions.toml` pins a specific version, defeating the BoM.
**Why it happens:** Misunderstanding how BoM dependency management works with version catalogs.
**How to avoid:** BoM-managed libraries in `libs.versions.toml` must have NO version, just group + name:
  ```toml
  firebase-firestore = { group = "com.google.firebase", name = "firebase-firestore" }
  ```
  The BoM `platform()` dependency provides the version at runtime.

### Pitfall 6: Ditto Bluetooth/WiFi Permissions Not Requested

**What goes wrong:** Ditto sync never starts because Android withholds runtime permissions. P2P peer count stays at 0.
**Why it happens:** Starting API 31, Bluetooth permissions changed; API 33+ requires `NEARBY_WIFI_DEVICES`. Ditto's manifest merger handles declaration but runtime request must be in the app.
**How to avoid:** Copy `requestMissingPermissions()` from `MainActivity.kt` (existing quickstart), which uses `DittoSyncPermissions(this).missingPermissions()`.

---

## Code Examples

Verified patterns from official sources and existing repo code:

### Koin startKoin in Application

```kotlin
// Source: https://insert-koin.io/docs/reference/koin-android/start/
class PosApplication : Application() {
    override fun onCreate() {
        super.onCreate()
        startKoin {
            androidLogger()
            androidContext(this@PosApplication)
            modules(appModule, repositoryModule, viewModelModule)
        }
    }
}
```

### Ditto Init (adapted from android-kotlin/QuickStartTasks/TasksApplication.kt)

```kotlin
// Source: Existing repo — android-kotlin/QuickStartTasks/app/src/main/java/.../TasksApplication.kt
val androidDependencies = DefaultAndroidDittoDependencies(androidContext())
val ditto = Ditto(
    androidDependencies,
    DittoIdentity.OnlinePlayground(
        dependencies = androidDependencies,
        appId = BuildConfig.DITTO_APP_ID,
        token = BuildConfig.DITTO_PLAYGROUND_TOKEN,
        customAuthUrl = BuildConfig.DITTO_AUTH_URL,
        enableDittoCloudSync = false
    )
)
ditto.updateTransportConfig { config ->
    config.connect.websocketUrls.add(BuildConfig.DITTO_WEBSOCKET_URL)
}
ditto.disableSyncWithV3()
```

### Firebase Firestore — Disable Persistence (current API)

```kotlin
// Source: Firebase docs — firebase.google.com/docs/firestore/manage-data/enable-offline
// BoM 34.x recommended approach (not the deprecated isPersistenceEnabled = false)
val db = Firebase.firestore
db.firestoreSettings = firestoreSettings {
    setLocalCacheSettings(memoryCacheSettings {})
}
```

### ProGuard Rule for Ditto FFI

```
# Source: https://docs.ditto.live/support/proguard-rules-for-android
# Required for all Ditto SDK use
-keep class live.ditto.internal.swig.ffi.** { *; }
```

### BuildConfig Fields via androidComponents (copy from existing app/build.gradle.kts)

```kotlin
// Source: android-kotlin/QuickStartTasks/app/build.gradle.kts
androidComponents {
    onVariants {
        val prop = loadEnvProperties()
        it.buildConfigFields.put("DITTO_APP_ID",
            BuildConfigField("String", "\"${prop["DITTO_APP_ID"]}\"", "Ditto app ID"))
        it.buildConfigFields.put("DITTO_PLAYGROUND_TOKEN",
            BuildConfigField("String", "\"${prop["DITTO_PLAYGROUND_TOKEN"]}\"", "Ditto token"))
        it.buildConfigFields.put("DITTO_AUTH_URL",
            BuildConfigField("String", "\"${prop["DITTO_AUTH_URL"]}\"", "Ditto auth URL"))
        it.buildConfigFields.put("DITTO_WEBSOCKET_URL",
            BuildConfigField("String", "\"${prop["DITTO_WEBSOCKET_URL"]}\"", "Ditto WS URL"))
    }
}
```

### Ditto Runtime Permissions Request (copy from existing MainActivity.kt)

```kotlin
// Source: android-kotlin/QuickStartTasks/app/src/main/java/.../MainActivity.kt
private fun requestMissingPermissions() {
    val missingPermissions = DittoSyncPermissions(this).missingPermissions()
    if (missingPermissions.isNotEmpty()) {
        this.requestPermissions(missingPermissions, 0)
    }
}
```

### Smoke-Test Status Screen (Phase 1 completion check)

```kotlin
// At discretion — minimal Compose screen to validate both SDKs initialized
@Composable
fun StatusScreen(ditto: Ditto = koinGet(), db: FirebaseFirestore = koinGet()) {
    Column(modifier = Modifier.padding(16.dp)) {
        Text("Ditto: initialized (peers: ${ditto.presence.graph.localPeer.deviceName})")
        Text("Firestore: connected")
    }
}
```

---

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| `firebase-firestore-ktx` | `firebase-firestore` (KTX merged) | BoM 34.0.0 (July 2025) | Remove `-ktx` suffix from all Firebase artifacts |
| `isPersistenceEnabled = false` | `memoryCacheSettings {}` via `setLocalCacheSettings` | Firebase SDK v24+ | New API is cleaner; old API still works but deprecated |
| Ditto companion object singleton | Koin `single {}` | This project introduces Koin | Lifecycle-managed, testable, injectable |
| `ditto.startSync()` in Application | Deferred to ViewModel/Repository layer | Best practice | Application.onCreate() should only initialize, not start sync |

**Deprecated/outdated:**
- `firebase-firestore-ktx`: Retired from Firebase BoM as of 34.0.0; use `firebase-firestore` directly
- `DittoHandler.companion object`: Only pattern in existing quickstart — not to be used in this project; Koin is the injection mechanism
- `composeOptions { kotlinCompilerExtensionVersion }`: Deprecated in favor of the `compose-compiler` Kotlin plugin (already applied via `alias(libs.plugins.compose.compiler)`)

---

## Open Questions

1. **DQL_STRICT_MODE setting in Koin single block**
   - What we know: `ditto.store.execute()` is a suspend function; Koin `single {}` is synchronous
   - What's unclear: Whether this setting is needed for Phase 1 (no DQL queries run in this phase)
   - Recommendation: Skip for Phase 1. Add it in Phase 2 within a coroutine when DQL is first used

2. **Ditto startSync() timing**
   - What we know: The existing quickstart calls `startSync()` from the ViewModel; CONTEXT.md doesn't specify
   - What's unclear: Should Phase 1 include `ditto.startSync()` at all, or leave it to Phase 2/3?
   - Recommendation: Phase 1 should NOT call `startSync()` — just initialize. Start sync in Phase 3 when Subscriptions are configured. Peer count will show 0 in smoke test, which is expected

3. **Koin module lazy vs. eager initialization**
   - What we know: Koin `single {}` resolves lazily; first `get<Ditto>()` triggers Ditto constructor
   - What's unclear: Whether the smoke-test screen is fast enough to show "initialized" state or shows a loading state briefly
   - Recommendation: Add `androidContext().get<Ditto>()` eager call from Application.onCreate() IO scope to pre-warm the singleton

---

## Validation Architecture

### Test Framework

| Property | Value |
|----------|-------|
| Framework | JUnit 4 (existing baseline) + Espresso for instrumented |
| Config file | None — uses Gradle test runner conventions |
| Quick run command | `./gradlew test` (unit tests) |
| Full suite command | `./gradlew test connectedAndroidTest` |

### Phase Requirements → Test Map

| Req ID | Behavior | Test Type | Automated Command | File Exists? |
|--------|----------|-----------|-------------------|-------------|
| FOUN-01 | Ditto initializes without crash in Application.onCreate() | Instrumented smoke | `./gradlew connectedAndroidTest` | Wave 0 |
| FOUN-02 | Firestore offline persistence disabled before first use | Unit (settings check) | `./gradlew test` | Wave 0 |
| FOUN-03 | Koin DI graph assembles without errors | Unit (koinTest verify) | `./gradlew test` | Wave 0 |
| FOUN-04 | libs.versions.toml present and build compiles | Build verification | `./gradlew assembleDebug` | Wave 0 |
| FOUN-05 | proguard-rules.pro contains Ditto keep rule | Unit (file content check) | `./gradlew test` | Wave 0 |

**Practical note:** FOUN-01 (no crash on physical device) is a manual verification step. The automated test suite covers DI graph assembly (FOUN-03 via `checkModules()`) and build compilation (FOUN-04). FOUN-01 requires running on device.

### Sampling Rate

- **Per task commit:** `./gradlew assembleDebug` (build must not fail)
- **Per wave merge:** `./gradlew test` (unit tests green)
- **Phase gate:** `./gradlew assembleDebug` green + manual smoke test on device before `/gsd:verify-work`

### Wave 0 Gaps

- [ ] `app/src/test/java/live/ditto/pubsec/pos/KoinModuleTest.kt` — covers FOUN-03 (Koin module verification)
- [ ] `app/src/test/java/live/ditto/pubsec/pos/FirestoreSettingsTest.kt` — covers FOUN-02 (persistence disabled)
- [ ] `app/src/test/java/live/ditto/pubsec/pos/ExampleUnitTest.kt` — placeholder, covers FOUN-04 indirectly via build

For FOUN-03, the Koin test module check pattern:
```kotlin
// Source: Koin docs — https://insert-koin.io/docs/reference/koin-test/testing/
class KoinModuleTest : KoinTest {
    @Test
    fun verifyKoinApp() {
        koinApplication {
            androidContext(mockk())
            modules(appModule, repositoryModule, viewModelModule)
        }.checkModules()
    }
}
```

---

## Sources

### Primary (HIGH confidence)
- Existing repo: `android-kotlin/QuickStartTasks/` — TasksApplication.kt, app/build.gradle.kts, gradle/libs.versions.toml — direct inspection
- `https://docs.ditto.live/support/proguard-rules-for-android` — Ditto ProGuard rules
- `https://docs.ditto.live/sdk/latest/install-guides/kotlin` — Ditto Android install guide
- `https://insert-koin.io/docs/reference/koin-android/start/` — Koin Android startKoin pattern
- `https://firebase.google.com/docs/android/setup` — Firebase Android BoM setup, google-services 4.4.4 confirmed
- `https://central.sonatype.com/artifact/live.ditto/ditto` — Ditto 4.14.3 confirmed on Maven Central

### Secondary (MEDIUM confidence)
- `https://firebase.google.com/support/release-notes/android` — Firebase BoM 34.10.0 confirmed (Feb 26, 2026), KTX retirement confirmed at BoM 34.0.0
- `https://firebase.google.com/docs/firestore/manage-data/enable-offline` — memoryCacheSettings pattern for disabling persistence
- WebSearch result confirming google-services plugin 4.4.4 is latest

### Tertiary (LOW confidence)
- None — all critical claims verified with primary/secondary sources

---

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH — all library versions verified on Maven Central or official docs
- Architecture: HIGH — patterns copied directly from existing repo code with documented modifications
- Pitfalls: HIGH — derived from official SDK docs, existing code inspection, and Firebase API change history

**Research date:** 2026-03-03
**Valid until:** 2026-04-03 (stable ecosystem; Firebase BoM may release minor updates but 34.10.0 is safe for 30 days)
