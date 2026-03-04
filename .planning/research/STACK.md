# Stack Research

**Domain:** Android POS demo app — Firebase/Firestore (cloud) + Ditto SDK (edge P2P) bidirectional sync bridge
**Researched:** 2026-03-03
**Confidence:** HIGH (core SDKs verified against official docs; versions confirmed from live sources)

---

## Recommended Stack

### Core Technologies

| Technology | Version | Purpose | Why Recommended |
|------------|---------|---------|-----------------|
| Kotlin | 2.1.0 | Primary language | Version used in repo's android-kotlin quickstart; stable, AGP 8.x compatible. Kotlin 2.3.x is latest stable but requires AGP 9.x + Gradle 9.1+, which is a large migration step not warranted for a demo. |
| Android Gradle Plugin (AGP) | 8.9.3 | Build system | Version used in repo's android-kotlin quickstart; mature, well-understood, avoids AGP 9.0 migration friction. |
| Ditto Kotlin SDK | 4.14.3 | Edge P2P sync | Latest stable as of Feb 2026. Includes intelligent write batching (3.2x sync improvement), new COUNTER CRDT type, and 16KB alignment for Google Play. Critical: v4.14.2 fixed a deadlock in P2P connection management — use 4.14.x, not 4.13.x. |
| Firebase Android BoM | 34.10.0 | Firebase dependency management | Latest stable as of Feb 26, 2026. Manages all Firebase library versions together. KTX modules were folded into main modules starting with BoM 34.0.0 — no separate `-ktx` artifacts needed. |
| Jetpack Compose BOM | 2026.02.01 | Compose dependency management | Latest stable. Maps to Compose UI 1.10.4 and Material3 1.4.0. Repo's android-kotlin quickstart uses 2025.07.00; 2026.02.01 is current and backward-compatible. |
| Material3 | 1.4.0 (via Compose BOM) | UI component system | Standard for new Android apps. Included via Compose BOM — do not pin separately. |
| Kotlin Coroutines | 1.10.2 | Async/reactive primitives | Used throughout repo. StateFlow for ViewModel state, callbackFlow for bridging Ditto/Firestore listener callbacks into reactive streams. |

### Supporting Libraries

| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| `firebase-firestore` | via BoM 34.10.0 | Cloud data store | Primary collection for products, orders, inventory. Provides real-time snapshot listeners wrapped in Kotlin Flow via `.snapshots()` extension (available since firestore 24.3.0 — no manual callbackFlow needed). |
| `firebase-common` | via BoM 34.10.0 | Firebase core runtime | Required transitive dependency; managed by BoM. |
| `google-services` plugin | 4.4.4 | Injects `google-services.json` into build | Required Gradle plugin — applies in app module only. |
| `androidx.lifecycle:lifecycle-viewmodel-compose` | 2.9.2 | ViewModel in Compose | Exposes `viewModel()` composable factory. Required for MVVM wiring in Compose screens. |
| `androidx.navigation:navigation-compose` | 2.9.2 | Screen navigation | Standard Compose navigation; used in existing repo quickstart. Handles back stack and nav graph. |
| `androidx.lifecycle:lifecycle-runtime-ktx` | 2.9.2 | Lifecycle-aware coroutine scopes | `lifecycleScope`, `repeatOnLifecycle` — needed for safe flow collection in Activity/Fragment. |
| `androidx.datastore:datastore-preferences` | 1.1.7 | Lightweight persistent config | Optional: store connectivity state flags or user preferences. Replaces SharedPreferences. |
| Koin BOM | 4.1.0 | Dependency injection | Used in repo's android-kotlin quickstart. Simpler than Hilt for demo apps; Kotlin DSL, no annotation processing. For a single-developer demo, Koin's faster setup is correct call. |
| `koin-androidx-compose` | via Koin BOM 4.1.0 | Koin integration with Compose | `koinViewModel()` composable — binds DI and ViewModel lifecycle correctly in Compose. |
| `kotlinx-coroutines-android` | 1.10.2 | Android main dispatcher | Ensures coroutines run on Android main thread correctly. Required alongside coroutines-core. |

### Development Tools

| Tool | Purpose | Notes |
|------|---------|-------|
| Gradle Version Catalogs (`libs.versions.toml`) | Centralized dependency version management | Required by repo conventions — all existing Android quickstarts use this. Place in `gradle/libs.versions.toml`. |
| Android Studio Hedgehog+ | IDE with Compose preview | Compose layout inspector and live preview work best in Hedgehog+. |
| ProGuard rules | Prevent Ditto class stripping on release builds | Add `-keep class live.ditto.internal.swig.ffi.** { *; }` — official Ditto requirement or release builds will crash. |
| `google-services.json` | Firebase project configuration | Must be placed in `app/` directory. Generated from Firebase Console. Do NOT commit to version control if it contains sensitive project identifiers. |

---

## Installation

```toml
# gradle/libs.versions.toml

[versions]
agp = "8.9.3"
kotlin = "2.1.0"
coreKtx = "1.16.0"
lifecycleRuntimeKtx = "2.9.2"
activityCompose = "1.10.1"
composeBom = "2026.02.01"
navigationCompose = "2.9.2"
koin-bom = "4.1.0"
ditto = "4.14.3"
firebaseBom = "34.10.0"
googleServices = "4.4.4"
coroutines = "1.10.2"

[libraries]
# Android core
androidx-core-ktx = { group = "androidx.core", name = "core-ktx", version.ref = "coreKtx" }
androidx-lifecycle-runtime-ktx = { group = "androidx.lifecycle", name = "lifecycle-runtime-ktx", version.ref = "lifecycleRuntimeKtx" }
androidx-lifecycle-viewmodel-compose = { group = "androidx.lifecycle", name = "lifecycle-viewmodel-compose", version.ref = "lifecycleRuntimeKtx" }
androidx-activity-compose = { group = "androidx.activity", name = "activity-compose", version.ref = "activityCompose" }

# Compose
androidx-compose-bom = { group = "androidx.compose", name = "compose-bom", version.ref = "composeBom" }
androidx-ui = { group = "androidx.compose.ui", name = "ui" }
androidx-ui-graphics = { group = "androidx.compose.ui", name = "ui-graphics" }
androidx-ui-tooling-preview = { group = "androidx.compose.ui", name = "ui-tooling-preview" }
androidx-material3 = { group = "androidx.compose.material3", name = "material3" }
androidx-navigation-compose = { group = "androidx.navigation", name = "navigation-compose", version.ref = "navigationCompose" }

# Ditto
live-ditto = { group = "live.ditto", name = "ditto", version.ref = "ditto" }

# Firebase (versions managed by BoM)
firebase-bom = { group = "com.google.firebase", name = "firebase-bom", version.ref = "firebaseBom" }
firebase-firestore = { group = "com.google.firebase", name = "firebase-firestore" }
firebase-common = { group = "com.google.firebase", name = "firebase-common" }

# Koin DI
koin-bom = { group = "io.insert-koin", name = "koin-bom", version.ref = "koin-bom" }
koin-android = { group = "io.insert-koin", name = "koin-android" }
koin-androidx-compose = { group = "io.insert-koin", name = "koin-androidx-compose" }

# Coroutines
kotlinx-coroutines-android = { group = "org.jetbrains.kotlinx", name = "kotlinx-coroutines-android", version.ref = "coroutines" }

[plugins]
android-application = { id = "com.android.application", version.ref = "agp" }
jetbrains-kotlin-android = { id = "org.jetbrains.kotlin.android", version.ref = "kotlin" }
compose-compiler = { id = "org.jetbrains.kotlin.plugin.compose", version.ref = "kotlin" }
google-services = { id = "com.google.gms.google-services", version.ref = "googleServices" }
```

```kotlin
// app/build.gradle.kts

plugins {
    alias(libs.plugins.android.application)
    alias(libs.plugins.jetbrains.kotlin.android)
    alias(libs.plugins.compose.compiler)
    alias(libs.plugins.google.services)  // Firebase requires this
}

android {
    compileSdk = 35
    defaultConfig {
        minSdk = 23  // Ditto requirement; also matches repo convention
        targetSdk = 35
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }
    kotlinOptions { jvmTarget = "11" }
    buildFeatures { compose = true; buildConfig = true }
}

dependencies {
    implementation(libs.androidx.core.ktx)
    implementation(libs.androidx.lifecycle.runtime.ktx)
    implementation(libs.androidx.lifecycle.viewmodel.compose)
    implementation(libs.androidx.activity.compose)
    implementation(libs.androidx.navigation.compose)

    // Compose BOM — no version on individual artifacts
    implementation(platform(libs.androidx.compose.bom))
    implementation(libs.androidx.ui)
    implementation(libs.androidx.ui.graphics)
    implementation(libs.androidx.ui.tooling.preview)
    implementation(libs.androidx.material3)

    // Ditto
    implementation(libs.live.ditto)

    // Firebase BOM — no version on individual artifacts
    implementation(platform(libs.firebase.bom))
    implementation(libs.firebase.firestore)

    // Koin BOM — no version on individual artifacts
    implementation(platform(libs.koin.bom))
    implementation(libs.koin.android)
    implementation(libs.koin.androidx.compose)

    // Coroutines
    implementation(libs.kotlinx.coroutines.android)
}
```

---

## Alternatives Considered

| Recommended | Alternative | When to Use Alternative |
|-------------|-------------|-------------------------|
| Koin 4.1.0 | Hilt 2.x | Use Hilt if this grows into a production app with many contributors. Hilt has compile-time verification and Google's backing. For a demo app with one engineer, Koin's simpler setup is correct. |
| Firebase BoM (no KTX) | firebase-firestore-ktx (legacy) | Never — KTX modules were removed from BoM 34.0.0. Using `-ktx` artifacts alongside BoM 34.x causes version conflicts. |
| Jetpack Navigation Compose | Voyager / Decompose | Use only if you need true multiplatform navigation. For Android-only, Navigation Compose is the Google-supported standard. |
| StateFlow (ViewModel) | LiveData | LiveData is legacy. StateFlow + Kotlin coroutines is the current standard for new projects. Do not use LiveData for state in new code. |
| Compose BOM (latest) | Pinning individual Compose versions | Never pin individual Compose library versions alongside a BOM — it causes version conflicts and breaks BoM guarantees. |
| Ditto 4.14.3 | Ditto 5.0.0 | The acoustic-edge pubsec project has `ditto:5.0.0` with a `// TODO: Update` comment, suggesting it was a test pin. Official install guide targets 4.14.x. Use 4.14.3 until 5.x has stable documentation. |

---

## What NOT to Use

| Avoid | Why | Use Instead |
|-------|-----|-------------|
| `firebase-firestore-ktx` artifact | Removed from Firebase BoM 34.0.0+. All KTX APIs were merged into the main `firebase-firestore` module. Using `-ktx` will create dependency resolution conflicts with BoM 34.x. | `firebase-firestore` (via BoM) |
| Manual `callbackFlow` for Firestore listeners | Firestore's Kotlin SDK has included `.snapshots()` extension since v24.3.0 which already returns `Flow<QuerySnapshot>`. Writing your own callbackFlow is unnecessary boilerplate. | `collection.snapshots()` / `document.snapshots()` extension functions |
| `LiveData` for ViewModel state | Deprecated pattern. StateFlow integrates directly with coroutines, is null-safe, and Compose's `collectAsState()` handles it correctly without `observeAsState()` workarounds. | `StateFlow<UiState>` in ViewModel |
| AGP 9.0+ / Kotlin 2.3+ for this project | AGP 9.0 requires Gradle 9.1+ and introduces breaking changes. Kotlin 2.3 paired with AGP 9.0 is a significant migration. Repo's android-kotlin quickstart is on AGP 8.9.3 / Kotlin 2.1 — stay consistent to avoid blocking the demo. | AGP 8.9.3 + Kotlin 2.1.0 |
| `Room` for local caching | Ditto IS the local database at the edge. Adding Room alongside Ditto creates a redundant data layer and a third sync problem (Ditto ↔ Room ↔ Firebase). Keep it: Firebase = cloud store, Ditto = edge store, no Room. | Ditto's embedded document store |
| Ditto 4.13.x | v4.14.2 fixed a deadlock that prevents new P2P connections. Any version before 4.14.2 is unsafe for a mesh networking demo where P2P connectivity is the feature being demonstrated. | Ditto 4.14.3 |

---

## Stack Patterns by Variant

**For the Firebase-to-Ditto sync bridge (cloud → edge):**
- Use a Firestore `collection.snapshots()` Flow collected in a Repository class
- On each snapshot, upsert changed documents into Ditto using `ditto.store.execute()`
- Run this bridge in a coroutine tied to a Service or Application-scoped ViewModel
- Because this bridge can open from the internet side, implement a connectivity check before starting the Firestore listener

**For the Ditto-to-Firebase sync bridge (edge → cloud):**
- Register a Ditto `registerObserver` or `store.execute()` result observer
- On each Ditto change event, write to Firestore using `set()` with merge semantics
- Use Firestore's server timestamp for cloud-side ordering; use Ditto's CRDT semantics for edge-side conflict resolution
- Apply a "last-writer-wins" or "Firebase wins on reconnect" strategy for the bidirectional conflict case — document this choice explicitly in code

**For connectivity state indicators:**
- Expose two `StateFlow<Boolean>` from a `ConnectivityRepository`: `isFirebaseConnected` and `isDittoMeshActive`
- Firebase: observe `FirebaseApp` connectivity via a no-op Firestore write/read with timeout
- Ditto: observe `ditto.presence` or `ditto.sync.isEnabled` for mesh status

**For POS operations while offline:**
- All writes go to Ditto first (edge source of truth)
- A bridge coroutine attempts Firestore write in background; if offline, queues for retry
- Never block the POS UI on Firebase connectivity — this is the core demo value

---

## Version Compatibility

| Package | Compatible With | Notes |
|---------|-----------------|-------|
| Ditto 4.14.3 | minSdk 23, compileSdk 34-35 | Official requirement is API 23 (Android 6). Repo uses compileSdk 35. |
| Ditto 4.14.3 | Kotlin 1.7.20+ | Ditto dropped the Kotlin 2.0+ requirement in 4.14.x. Kotlin 2.1.0 is fully supported. |
| Firebase BoM 34.10.0 | AGP 7.3.0+ | BoM 34.x works with AGP 8.x without issues. |
| Compose BOM 2026.02.01 | AGP 8.x, Kotlin 2.1+ | No known conflicts with AGP 8.9.3 / Kotlin 2.1.0. |
| Koin BOM 4.1.0 | Kotlin 2.1+ | Koin 4.x uses Kotlin 2.0+ — no issue with Kotlin 2.1.0. |
| AGP 8.9.3 | Gradle 8.11.1+ | AGP 8.9.x requires Gradle 8.x. Do NOT use Gradle 9.x without migrating to AGP 9.x. |
| google-services plugin 4.4.4 | AGP 7.3.0–8.x | Current; processes `google-services.json` correctly with AGP 8.x. |

---

## Sources

- [Ditto Kotlin Install Guide (v4.9+)](https://docs.ditto.live/install-guides/kotlin) — confirmed version 4.14.1 as install guide reference; release notes confirm 4.14.3 latest
- [Ditto Kotlin Release Notes](https://docs.ditto.live/sdk/latest/release-notes/kotlin) — confirmed 4.14.3 (Feb 19, 2026), P2P deadlock fix in 4.14.2, performance gains in 4.14.0 — HIGH confidence
- [Firebase Android Release Notes](https://firebase.google.com/support/release-notes/android) — confirmed BoM 34.10.0 (Feb 26, 2026), KTX removal from BoM 34.0.0 — HIGH confidence
- [Firebase Android Setup Docs](https://firebase.google.com/docs/android/setup) — confirmed google-services plugin 4.4.4, BoM usage pattern — HIGH confidence
- [Compose BOM to Library Version Mapping](https://developer.android.com/develop/ui/compose/bom/bom-mapping) — confirmed BOM 2026.02.01 → Compose 1.10.4, Material3 1.4.0 — HIGH confidence
- [Kotlin Releases](https://kotlinlang.org/docs/releases.html) — confirmed Kotlin 2.3.10 is latest stable; 2.1.0 is what repo uses and is compatible — HIGH confidence
- [AGP 9.0 Release Notes](https://developer.android.com/build/releases/agp-9-0-0-release-notes) — confirmed AGP 9.0 requires Gradle 9.1+; rationale for staying on 8.9.3 — HIGH confidence
- Repo's `android-kotlin/QuickStartTasks/gradle/libs.versions.toml` — confirmed existing stack choices (AGP 8.9.3, Kotlin 2.1.0, Ditto 4.13.1, Koin 4.1.0, Compose BOM 2025.07.00) — HIGH confidence (direct inspection)
- [Firestore Kotlin Flow extensions](https://medium.com/firebase-tips-tricks/how-to-use-kotlin-flows-with-firestore-6c7ee9ae12f3) — confirmed `.snapshots()` extension available since firestore-ktx 24.3.0 — MEDIUM confidence (community source, cross-referenced with Firebase blog)
- [Koin vs Hilt 2025 comparison](https://jamshidbekboynazarov.medium.com/hilt-vs-koin-a-2025-perspective-on-android-di-139b27d684e1) — rationale for Koin in demo context — MEDIUM confidence (community analysis)

---

*Stack research for: Android Firebase-Ditto POS Bridge*
*Researched: 2026-03-03*
