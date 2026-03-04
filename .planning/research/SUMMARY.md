# Project Research Summary

**Project:** Firebase-Ditto POS Bridge — Android demo app
**Domain:** Bidirectional sync bridge between Firebase Firestore (cloud) and Ditto SDK (edge P2P)
**Researched:** 2026-03-03
**Confidence:** MEDIUM-HIGH (stack HIGH, architecture MEDIUM, features HIGH, pitfalls MEDIUM)

## Executive Summary

This project is a novel integration demo: an Android POS application that bridges Firebase Firestore (cloud source of truth) with Ditto's P2P mesh SDK (edge sync). No prior art exists for this exact combination — the Ditto POS demo app (`demoapp-pos-kds`) uses Ditto only, and Firebase POS examples use local queues without mesh sync. The recommended approach is to treat Ditto as the edge database that the UI reads exclusively, with a `SyncBridgeService` handling bidirectional propagation between Firestore and Ditto. This offline-first design means the UI code is identical regardless of connectivity state, which is the core demo value: a POS terminal that keeps working when Firebase is unreachable.

The primary technical risk is the sync loop — a bidirectional bridge where writes from each system trigger the other system's listener, producing infinite feedback. This must be addressed from the first commit with a `ChangeGuard` component and `hasPendingWrites` checking on Firestore snapshots. A secondary risk is data model correctness: inventory quantities modeled as mutable integers will lose data under concurrent offline edits due to Ditto's last-write-wins register semantics. Both of these are architectural decisions that cannot be safely retrofitted — they must be correct before feature development begins.

The stack is well-aligned with the existing `ditto-quickstart` repository conventions: Kotlin 2.1.0, AGP 8.9.3, Jetpack Compose, Koin for DI, and Kotlin Coroutines. The Ditto SDK should be updated from the existing quickstart's 4.13.1 to 4.14.3, which fixes a P2P connection management deadlock that would undermine the mesh demo. Firebase integration uses BoM 34.10.0 — the KTX module consolidation in BoM 34.0.0 means no `-ktx` artifacts should be used.

## Key Findings

### Recommended Stack

The stack closely follows the existing `android-kotlin` quickstart in this repo, with two additions (Firebase) and one update (Ditto). All dependency management uses BOMs/version catalogs per repo convention. Koin is the right DI choice for a demo app — simpler than Hilt without compile-time annotation processing overhead. StateFlow throughout (no LiveData). Firestore's `.snapshots()` Flow extension is available from the standard SDK — no manual `callbackFlow` wrapping needed.

**Core technologies:**
- Kotlin 2.1.0 + AGP 8.9.3: matches existing repo, avoids AGP 9.0 migration friction
- Ditto SDK 4.14.3: critical update from 4.13.1 — P2P deadlock fix in 4.14.2
- Firebase Android BoM 34.10.0: latest stable; KTX modules merged into base modules — no `-ktx` artifacts
- Jetpack Compose BOM 2026.02.01: current stable, backward-compatible upgrade from repo's 2025.07.00
- Koin BOM 4.1.0: already used in repo; correct choice for demo DI
- Kotlin Coroutines 1.10.2: StateFlow + callbackFlow for reactive bridge wiring

**Critical version notes:**
- Ditto 4.14.3 minimum (4.14.2 fixed the P2P deadlock)
- ProGuard keep rule required: `-keep class live.ditto.internal.swig.ffi.** { *; }`
- minSdk = 23 (Ditto requirement)

### Expected Features

The MVP must prove a single thesis: a POS terminal running Ditto can keep operating when Firebase is unavailable, and sync bidirectionally when connectivity restores. All v1 features serve that thesis. Anything that doesn't serve it is deferred.

**Must have (v1 — demo fails without these):**
- Firebase project setup + Firestore seeded with products and inventory
- Ditto SDK initialized with Online Playground identity
- Product catalog screen (browse products from Ditto local store)
- Create order screen (select products, submit order written to Ditto first)
- Inventory decrement on order (concurrent write scenario)
- Firebase → Ditto bridge (Firestore SnapshotListener → Ditto store)
- Ditto → Firebase bridge (Ditto registerObserver → Firestore write)
- Firebase connectivity indicator (connected / disconnected)
- Ditto peer count indicator (mesh peer count badge)
- Offline operation: place orders when Firebase unreachable, sync on reconnect

**Should have (v1.x — makes demo clearly better):**
- Per-terminal offline mode toggle (repeatable demo without airplane mode)
- Sync event log / activity feed (shows bridge activity in real time)
- Order status progression (open → fulfilled)
- Visual diff on sync (highlight changed rows)

**Defer to v2+:**
- Conflict resolution visualization (high value but requires scripted multi-device scenario)
- Live peer metadata display (device names per peer)
- Kitchen display (KDS) view
- Any payment, authentication, barcode, or receipt features

### Architecture Approach

The architecture has four layers: Cloud (Firestore), Bridge (`SyncBridgeService` with `ChangeGuard`), Local/Edge (Ditto store + P2P mesh), and UI (Jetpack Compose + ViewModels). The UI reads exclusively from the Ditto local store — never from Firestore directly. This is the key architectural decision: it guarantees offline-first behavior and keeps UI code connectivity-agnostic. Firebase data flows into the UI only after passing through the bridge into Ditto.

**Major components:**
1. `SyncBridgeService` — orchestrates bidirectional sync; owns `ChangeGuard`; runs for app lifetime
2. `ChangeGuard` — prevents sync loops by tracking in-flight document IDs; must exist before any bridge code
3. `ProductsRepository` / `OrdersRepository` / `InventoryRepository` — Ditto-side read paths; expose `StateFlow` to ViewModels
4. `ProductSyncBridge` / `OrderSyncBridge` / `InventorySyncBridge` — per-collection Firebase ↔ Ditto bridge logic
5. `ConnectivityMonitor` — tracks Firebase connection state and Ditto mesh status; exposes `StateFlow<ConnectivityState>`
6. `PosApplication` — Ditto singleton initialization in `Application.onCreate()`; Koin module registration

**Build order from architecture research:**
Data models → Ditto init → Repositories → ViewModels + UI → Firebase integration → ChangeGuard → SyncBridgeService → ConnectivityMonitor

### Critical Pitfalls

1. **Sync loop (infinite write feedback)** — Implement `ChangeGuard` and check `snapshot.metadata.hasPendingWrites` on Firestore listeners before writing to Ditto. This is the most dangerous failure mode: it causes quota exhaustion and CPU saturation within seconds. Must be in from the first bridge commit.

2. **Ditto initialized in Activity context** — Always initialize Ditto in `Application.onCreate()` with `applicationContext`. Activity-scoped Ditto instances get garbage collected on rotation, producing split sync graphs and duplicate instances. Also add ProGuard keep rules before the first release build.

3. **Inventory quantities as last-write-wins integers** — Concurrent offline decrements on two terminals produce data loss via CRDT register semantics. Model inventory adjustments as delta events (`{ productId, delta: -2, terminalId }`) in a separate append-only collection. This must be in the data model before any write code exists.

4. **Firestore and Ditto document IDs diverging** — Use a single canonical UUID as both the Ditto `_id` and the Firestore document ID from first write. Auto-IDs on either side create duplicate documents and break upsert logic. ID strategy must be frozen in Phase 2 data modeling.

5. **Observer lifecycle leaks** — Every `DittoCancelable`, `DittoSyncSubscription`, and Firestore `ListenerRegistration` must be stored and cancelled in `ViewModel.onCleared()`. Leaking observers cause duplicate writes, battery drain, and memory growth.

6. **Soft deletes not honored across the bridge** — Use `deleted: true` fields in both Firestore and Ditto; never physically delete documents. Physical Firestore deletes cause Ditto tombstoned records to resurrect when the bridge re-syncs. This pattern already exists in the repo's quickstart task documents.

## Implications for Roadmap

Based on the component dependency graph from architecture research and the pitfall phase mappings, four phases emerge naturally.

### Phase 1: Foundation and Core Infrastructure

**Rationale:** All bridge code depends on Ditto singleton, Koin DI graph, data models, ChangeGuard, and observer lifecycle patterns. Every pitfall that is "HIGH recovery cost" must be prevented here. Getting this wrong means retrofitting across the entire codebase.

**Delivers:** Running Android app with Ditto initialized, Firebase connected, DI wired, data models defined with correct ID strategy, ChangeGuard implemented, ProGuard rules in place.

**Addresses:** Product catalog (read-only, Ditto-sourced) as a smoke test; Ditto peer count indicator as proof of SDK init.

**Avoids:**
- Pitfall 2 (Ditto Activity context): Ditto singleton in Application
- Pitfall 5 (Observer lifecycle leaks): ViewModel pattern with onCleared()
- Pitfall 2 (ProGuard): keep rules added before any release build

**Research flag:** Standard patterns — Ditto init, Koin, Compose scaffold are well-documented in the existing repo. No additional research needed for this phase.

---

### Phase 2: Data Models and Schema Contracts

**Rationale:** The three data pitfalls (inventory LWW, ID divergence, soft deletes) are schema decisions. They must be finalized before any write code exists. Changing the schema after writes are implemented requires data migration.

**Delivers:** Finalized Kotlin data models (`Product`, `Order`, `InventoryItem`), canonical UUID ID strategy documented and enforced, inventory delta-event schema defined, soft delete `deleted` field convention established, Firestore collections seeded with test data.

**Addresses:** 3 Firestore collections (products, orders, inventory); Firestore seeding for demo.

**Avoids:**
- Pitfall 3 (Inventory LWW data loss): delta events, not absolute integers
- Pitfall 4 (ID divergence): canonical UUID strategy locked before any writes
- Pitfall 6 (Soft delete resurrection): `deleted` flag in schema, never physical deletes

**Research flag:** Standard patterns — UUID generation, Kotlin data classes, Firestore seeding are well-understood. No additional research needed.

---

### Phase 3: Bidirectional Sync Bridge

**Rationale:** This is the most novel component. The SyncBridgeService, per-collection bridge classes, and ChangeGuard must all work correctly together before the demo value can be demonstrated. Firebase → Ditto direction enables offline reads; Ditto → Firebase direction enables offline writes to persist to cloud.

**Delivers:** Fully functional bidirectional bridge for all 3 collections; offline order creation that syncs to Firestore on reconnect; Firebase product/inventory changes visible in the app within seconds.

**Addresses:** Firebase → Ditto bridge, Ditto → Firebase bridge, offline operation, reconnect sync propagation.

**Avoids:**
- Pitfall 1 (Sync loop): ChangeGuard + hasPendingWrites check wired from first bridge commit
- Pitfall 6 (Soft delete resurrection): bridge propagates `deleted` flag, never deletes documents

**Research flag:** Needs research-phase planning. The Firebase-Ditto bridge has no prior art — patterns are synthesized from first principles in ARCHITECTURE.md. Specific DQL upsert syntax (`INSERT INTO ... ON ID CONFLICT DO UPDATE SET`) needs validation against current Ditto 4.14.x docs. Firestore `SetOptions.merge()` behavior on empty maps needs verification.

---

### Phase 4: UI Polish and Demo Layer

**Rationale:** Connectivity indicators, sync event log, offline mode toggle, and visual sync highlights are the demo presentation layer. They depend on a working bridge (Phase 3) and make the bridge's behavior legible to a developer audience.

**Delivers:** Firebase connectivity indicator, Ditto peer count badge, per-terminal offline mode toggle, sync event log (in-memory), order status progression, visual row highlights on sync changes.

**Addresses:** All v1.x "Should have" features; ConnectivityMonitor integration.

**Avoids:**
- UX pitfall (no connectivity state indicator): explicit Firebase + Ditto status UI
- UX pitfall (connectivity demo hard to trigger): offline mode toggle built in

**Research flag:** Standard patterns — Compose UI, StateFlow observation, Android NetworkCallback are well-documented. No additional research needed.

---

### Phase Ordering Rationale

- Phase 1 before Phase 2: DI graph and Ditto singleton must exist before data model classes can be injected
- Phase 2 before Phase 3: Schema decisions (IDs, soft deletes, inventory delta events) cannot be changed after bridge write code exists
- Phase 3 before Phase 4: Connectivity indicators require a working bridge to observe; the sync event log has nothing to log without a running bridge
- Features within each phase are grouped by the component they belong to, not by feature type — this avoids partially-built components spanning phases

### Research Flags

Phases needing deeper research during planning:
- **Phase 3 (Bridge):** DQL upsert syntax for `ON ID CONFLICT DO UPDATE SET` in Ditto 4.14.x; exact behavior of `SetOptions.merge()` for partial Firestore writes; ChangeGuard thread-safety model with coroutine dispatchers; Firestore `MetadataChanges.EXCLUDE_METADATA_CHANGES` vs `hasPendingWrites` — which is more reliable for loop prevention.

Phases with standard patterns (can skip research-phase):
- **Phase 1:** Ditto init, Koin DI, Compose scaffold — documented in existing repo quickstarts
- **Phase 2:** Data classes, UUID strategy, Firestore seeding — no novel integration
- **Phase 4:** Compose UI, ConnectivityMonitor, StateFlow — well-documented Android patterns

## Confidence Assessment

| Area | Confidence | Notes |
|------|------------|-------|
| Stack | HIGH | All versions verified against official docs and live sources. Existing repo provides ground truth for compatibility. |
| Features | HIGH | Core Ditto and Firebase APIs are documented. POS demo conventions are MEDIUM — inferred from Ditto's own demo app and flutter_pos reference. |
| Architecture | MEDIUM | Four-layer pattern and ChangeGuard are well-reasoned from first principles. No existing Firebase-Ditto bridge reference app exists — patterns are novel synthesis, not documented patterns. |
| Pitfalls | MEDIUM | Core Ditto pitfalls from official troubleshooting docs. Bridge-specific pitfalls (sync loops, ID divergence) inferred from analogous MongoDB Connector and offline-sync architecture literature. |

**Overall confidence:** MEDIUM-HIGH

The stack and features are solid. The architecture is sound but novel — the bridge implementation will encounter edge cases not covered by research. Plan for discovery work in Phase 3.

### Gaps to Address

- **Ditto DQL upsert syntax:** `INSERT INTO ... ON ID CONFLICT DO UPDATE SET` syntax needs live validation against Ditto 4.14.3 docs during Phase 3 planning. The architecture research uses the expected pattern but DQL evolves between major versions.
- **Firestore connection state detection:** No native `onConnected()` callback exists in Firestore SDK. Firebase Realtime Database `.info/connected` or a `NetworkCallback` workaround is required. Verify current best practice against Firebase Android SDK 34.10.0 before building ConnectivityMonitor.
- **ChangeGuard with coroutine dispatchers:** The ChangeGuard uses `runBlocking` in the architecture research example — this blocks the calling thread and may cause ANR if called from the wrong dispatcher. Validate the correct suspend/coroutine implementation during Phase 3.
- **Ditto 5.0.0:** The `acoustic-edge` pubsec project has a `ditto:5.0.0` pin with a `// TODO: Update` comment. If Ditto 5.x stabilizes before Phase 3 begins, evaluate whether to migrate — it may include breaking DQL changes.

## Sources

### Primary (HIGH confidence)
- Ditto Kotlin SDK Release Notes — confirmed 4.14.3, P2P deadlock fix in 4.14.2, performance gains in 4.14.0
- Ditto Kotlin Install Guide — init pattern, ProGuard requirements, minSdk 23
- Firebase Android Release Notes — BoM 34.10.0, KTX module consolidation in BoM 34.0.0
- Firebase Android Setup Docs — google-services plugin 4.4.4, BoM usage pattern
- Compose BOM to Library Version Mapping — BOM 2026.02.01 → Compose 1.10.4, Material3 1.4.0
- Firebase Firestore SnapshotMetadata docs — `hasPendingWrites` field for sync loop prevention
- Ditto SDK observer API — `registerObserver`, `registerSubscription`, lifecycle management
- Repo `android-kotlin/QuickStartTasks/` — direct inspection of existing AGP/Kotlin/Koin/Compose versions

### Secondary (MEDIUM confidence)
- Ditto Consistency Models (CRDT/LWW behavior) — delta events for inventory, register semantics
- GitHub: getditto/demoapp-pos-kds — POS feature set reference (Ditto-only, not Firebase bridge)
- Ditto MongoDB Connector docs — bridge pattern analogies for sync loop prevention
- Firebase: Build Presence in Cloud Firestore — no native connection state listener
- Offline Sync and Conflict Resolution Patterns (2026) — delta event model for quantities
- Android offline-first architecture patterns — unidirectional data flow through local store
- Firestore Kotlin Flow extensions — `.snapshots()` available since firestore-ktx 24.3.0

### Tertiary (LOW confidence, needs validation)
- GitHub Issue: Firestore connection state listener #947 — may be resolved in current SDK; verify
- Community DI comparison (Koin vs Hilt 2025) — rationale for Koin in demo context

---
*Research completed: 2026-03-03*
*Ready for roadmap: yes*
