# Requirements: Ditto-Firebase POS Bridge

**Defined:** 2026-03-03
**Core Value:** POS terminals continue processing transactions and stay in sync via Ditto P2P mesh even when Firebase/internet is unavailable

## v1 Requirements

### Foundation

- [x] **FOUN-01**: Android app initializes Ditto SDK with Online Playground identity in Application.onCreate()
- [x] **FOUN-02**: Android app initializes Firebase/Firestore with offline persistence disabled
- [x] **FOUN-03**: App uses Koin DI for dependency injection consistent with repo patterns
- [x] **FOUN-04**: Gradle build uses version catalogs (libs.versions.toml) consistent with repo conventions
- [x] **FOUN-05**: ProGuard/R8 rules configured for Ditto SDK in release builds

### Data Model

- [x] **DATA-01**: Products collection with canonical UUID IDs shared between Firestore and Ditto (name, price, category, imageUrl)
- [x] **DATA-02**: Orders collection with canonical UUID IDs shared between Firestore and Ditto (items, total, status, timestamp, terminalId)
- [x] **DATA-03**: Inventory collection with canonical UUID IDs shared between Firestore and Ditto (productId, quantity)
- [x] **DATA-04**: Soft delete convention using `deleted` flag (not hard deletes) in both systems
- [x] **DATA-05**: SyncSource tagging on all bridge-written documents to prevent sync loops

### Sync Bridge

- [x] **SYNC-01**: Firebase-to-Ditto bridge — Firestore SnapshotListener writes changes into Ditto local store for all 3 collections
- [x] **SYNC-02**: Ditto-to-Firebase bridge — Ditto registerObserver triggers Firestore writes for all 3 collections
- [x] **SYNC-03**: ChangeGuard preventing infinite sync loops (hasPendingWrites on Firestore side, syncSource skip on Ditto side)
- [x] **SYNC-04**: Bridge pauses gracefully when Firebase is unreachable and resumes on reconnect
- [x] **SYNC-05**: Ditto registerSubscription active for all 3 collections to pull data from P2P peers

### POS UI

- [ ] **POSU-01**: Product catalog screen displaying products from Ditto local store (sourced via bridge from Firestore)
- [ ] **POSU-02**: Create order screen — user can select products, see running total, and submit order
- [ ] **POSU-03**: Inventory decrements when order is placed
- [ ] **POSU-04**: Orders list screen showing all orders with status
- [ ] **POSU-05**: Jetpack Compose UI consistent with existing quickstart patterns (MVVM + ViewModels)

### Connectivity & Demo

- [ ] **DEMO-01**: Firebase connectivity indicator showing connected/disconnected status
- [ ] **DEMO-02**: Ditto mesh peer count indicator showing number of connected P2P peers
- [ ] **DEMO-03**: POS operations work fully offline — user can browse products, create orders, decrement inventory without internet
- [ ] **DEMO-04**: Changes made offline sync to Firebase when connectivity is restored

## v2 Requirements

### Demo Polish

- **POLH-01**: Per-terminal offline mode toggle (simulate Firebase loss without airplane mode)
- **POLH-02**: Sync event log / activity feed showing bridge operations in real time
- **POLH-03**: Order status progression (open → fulfilled) flowing across mesh
- **POLH-04**: Visual diff on sync — briefly highlight rows changed by incoming sync

### Advanced Demo

- **ADVD-01**: Conflict resolution visualization showing CRDT merge in action
- **ADVD-02**: Live peer metadata display (device names per peer)

## Out of Scope

| Feature | Reason |
|---------|--------|
| Payment processing | Sync demo, not payment system — PCI scope and SDK complexity distract from the story |
| User authentication / login | Single-store demo; Online Playground identity sufficient for evaluation |
| Receipt generation / printing | Orthogonal to sync architecture; requires printer SDK |
| Tax and discount calculation | Business logic noise obscuring the sync signal |
| Multi-store / multi-tenant | Data isolation complexity replaces sync as the story |
| Barcode scanner integration | Hardware dependency breaks demo portability |
| iOS / cross-platform | Android only per project constraints |
| Room database | Ditto is the edge data store; Room creates redundant third sync layer |
| Real-time chat between terminals | Scope creep; peer count communicates mesh presence adequately |
| Complex inventory replenishment | Reorder workflows far out of scope; inventory count is sufficient |

## Traceability

| Requirement | Phase | Status |
|-------------|-------|--------|
| FOUN-01 | Phase 1 | Complete |
| FOUN-02 | Phase 1 | Complete |
| FOUN-03 | Phase 1 | Complete |
| FOUN-04 | Phase 1 | Complete |
| FOUN-05 | Phase 1 | Complete |
| DATA-01 | Phase 2 | Complete |
| DATA-02 | Phase 2 | Complete |
| DATA-03 | Phase 2 | Complete |
| DATA-04 | Phase 2 | Complete |
| DATA-05 | Phase 2 | Complete |
| SYNC-01 | Phase 3 | Complete |
| SYNC-02 | Phase 3 | Complete |
| SYNC-03 | Phase 3 | Complete |
| SYNC-04 | Phase 3 | Complete |
| SYNC-05 | Phase 3 | Complete |
| POSU-01 | Phase 3 | Pending |
| POSU-02 | Phase 3 | Pending |
| POSU-03 | Phase 3 | Pending |
| POSU-04 | Phase 3 | Pending |
| POSU-05 | Phase 3 | Pending |
| DEMO-01 | Phase 4 | Pending |
| DEMO-02 | Phase 4 | Pending |
| DEMO-03 | Phase 4 | Pending |
| DEMO-04 | Phase 4 | Pending |

**Coverage:**
- v1 requirements: 24 total
- Mapped to phases: 24
- Unmapped: 0

---
*Requirements defined: 2026-03-03*
*Last updated: 2026-03-04 after Phase 3 Plan 1 completion*
