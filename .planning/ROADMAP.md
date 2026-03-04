# Roadmap: Ditto-Firebase POS Bridge

## Overview

An Android demo app proving that POS terminals keep operating via Ditto P2P mesh when Firebase/internet is unavailable. Four phases build the foundation, lock the schema, wire the bidirectional sync bridge, then add the demo presentation layer that makes the bridge's behavior visible to a developer audience.

## Phases

**Phase Numbering:**
- Integer phases (1, 2, 3): Planned milestone work
- Decimal phases (2.1, 2.2): Urgent insertions (marked with INSERTED)

Decimal phases appear between their surrounding integers in numeric order.

- [x] **Phase 1: Foundation** - Android app boots with Ditto and Firebase initialized, DI wired, ProGuard configured (completed 2026-03-04)
- [ ] **Phase 2: Data Models** - Schema contracts locked — canonical IDs, soft deletes, inventory delta events
- [ ] **Phase 3: Sync Bridge and POS UI** - Bidirectional bridge live for all 3 collections; POS screens reading from Ditto local store
- [ ] **Phase 4: Connectivity and Demo Layer** - Connectivity indicators and offline demo capability wired and visible

## Phase Details

### Phase 1: Foundation
**Goal**: A running Android app with Ditto and Firebase initialized, Koin DI wired, and all structural safeguards in place before any feature code is written
**Depends on**: Nothing (first phase)
**Requirements**: FOUN-01, FOUN-02, FOUN-03, FOUN-04, FOUN-05
**Success Criteria** (what must be TRUE):
  1. App launches without crash on a physical Android device (minSdk 23)
  2. Ditto SDK initializes in Application.onCreate() and Ditto peer count is readable (even if zero peers)
  3. Firebase/Firestore connects to the project (confirmed via Logcat or Firebase console)
  4. Koin DI graph assembles without errors on startup
  5. ProGuard keep rule for Ditto FFI classes is present in the build configuration
**Plans**: 3 plans

Plans:
- [x] 01-01-PLAN.md — Gradle scaffold: libs.versions.toml, build files, ProGuard, AndroidManifest
- [x] 01-02-PLAN.md — Application class, Koin DI modules, Ditto init, Firestore init
- [x] 01-03-PLAN.md — MainActivity, StatusScreen, Compose theme, KoinModuleTest + device verification

### Phase 2: Data Models
**Goal**: Schema contracts for all three collections are finalized and enforced before any write code exists — canonical UUID IDs, soft delete convention, and inventory delta-event pattern locked in
**Depends on**: Phase 1
**Requirements**: DATA-01, DATA-02, DATA-03, DATA-04, DATA-05
**Success Criteria** (what must be TRUE):
  1. Kotlin data classes exist for Product, Order, and InventoryItem with fields matching the spec (no field name drift between Firestore and Ditto representations)
  2. Firestore collections for products, orders, and inventory are seeded with test data readable in Firebase console
  3. Every document in Firestore and Ditto uses a single canonical UUID as its ID (no auto-generated ID divergence)
  4. All documents carry a `deleted` boolean field; no code performs physical deletes
  5. Inventory write operations use delta events rather than absolute quantity overwrites
**Plans**: 2 plans

Plans:
- [ ] 02-01-PLAN.md — Data model classes (Product, Order, InventoryItem, OrderStatus, Collections) and unit tests
- [ ] 02-02-PLAN.md — SeedData catalog, FirestoreSeeder, Koin wiring, seed/seeder tests

### Phase 3: Sync Bridge and POS UI
**Goal**: Bidirectional sync bridge is live for all three collections and the POS UI reads exclusively from Ditto local store — the core demo thesis is provable
**Depends on**: Phase 2
**Requirements**: SYNC-01, SYNC-02, SYNC-03, SYNC-04, SYNC-05, POSU-01, POSU-02, POSU-03, POSU-04, POSU-05
**Success Criteria** (what must be TRUE):
  1. Products added in Firebase console appear in the POS product catalog screen within a few seconds (Firebase → Ditto direction works)
  2. An order created on-device appears in Firestore within a few seconds when Firebase is connected (Ditto → Firebase direction works)
  3. Writing a document to either system does not produce an infinite cascade of writes (ChangeGuard prevents sync loops)
  4. With Firebase connectivity disabled, a user can browse products, create an order, and see inventory decrement — all without internet
  5. Orders created offline appear in Firestore after connectivity is restored
**Plans**: TBD

### Phase 4: Connectivity and Demo Layer
**Goal**: The bridge's behavior is legible to a developer audience — connectivity state is visible on-screen and the offline scenario is demonstrable without airplane mode
**Depends on**: Phase 3
**Requirements**: DEMO-01, DEMO-02, DEMO-03, DEMO-04
**Success Criteria** (what must be TRUE):
  1. A visible Firebase indicator on screen changes state when Firebase connectivity is lost and restored
  2. A visible Ditto mesh peer count indicator updates when another device joins or leaves the P2P mesh
  3. A developer can demonstrate the offline POS scenario by toggling Firebase connectivity from within the app (no airplane mode required)
  4. Changes made offline sync to Firebase and the Firebase indicator returns to connected state after reconnection
**Plans**: TBD

## Progress

**Execution Order:**
Phases execute in numeric order: 1 → 2 → 3 → 4

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 1. Foundation | 3/3 | Complete   | 2026-03-04 |
| 2. Data Models | 0/2 | Planning complete | - |
| 3. Sync Bridge and POS UI | 0/TBD | Not started | - |
| 4. Connectivity and Demo Layer | 0/TBD | Not started | - |
