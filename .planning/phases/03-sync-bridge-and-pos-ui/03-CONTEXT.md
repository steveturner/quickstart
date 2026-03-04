# Phase 3: Sync Bridge and POS UI - Context

**Gathered:** 2026-03-04
**Status:** Ready for planning

<domain>
## Phase Boundary

Bidirectional sync bridge live for all three collections (products, orders, inventory). POS UI screens reading exclusively from Ditto local store. Core demo thesis is provable: create order on one terminal, see it appear on another; decrement inventory offline, quantities merge correctly on reconnect. Requirements: SYNC-01 through SYNC-05, POSU-01 through POSU-05.

</domain>

<decisions>
## Implementation Decisions

### POS screen layout
- Product catalog displays as a 2-column grid of cards — product name, price, category badge on each card
- Category tabs at top for filtering: All | Beverages | Food | Merchandise
- Each product card shows current inventory count (e.g., "12 in stock") — key for demonstrating delta-event sync across terminals
- Orders list screen shows all orders from all terminals (not filtered by device) — demonstrates cross-device Ditto mesh sync

### Order creation flow
- Tap product card to add 1 unit to current order; tap again to increment quantity
- Sticky bottom bar shows running total: "3 items — $14.97 [Submit Order]" — collapses when cart is empty
- After submit: brief snackbar "Order #1234 placed", cart clears, stay on catalog screen for fast demo turnaround
- Orders are immutable after submission — items can't be changed, only status progresses (OPEN → FULFILLED)
- Inventory decrements immediately on order submission (Ditto CRDT counter + Firestore FieldValue.increment())

### Navigation structure
- Bottom navigation bar with 3 tabs: Catalog (home/order creation), Orders (list of all orders), Status
- Existing StatusScreen from Phase 1 becomes the Status tab — Phase 4 will enhance with connectivity indicators
- Material 3 NavigationBar component with Navigation Compose for routing
- Catalog tab is the start destination

### Bridge error visibility
- Phase 3 logs sync errors to Logcat only — no UI error indicators (that's Phase 4's scope)
- Bridge auto-retries Firestore write failures with exponential backoff (3 retries) — silent recovery when Firebase reconnects
- Bridge pauses gracefully when Firebase is unreachable, resumes on reconnect (SYNC-04)

### Sync bridge behavior (carried from prior decisions)
- UI reads exclusively from Ditto local store — never from Firestore directly
- SyncSource tagging on all bridge-written docs prevents infinite sync loops
- Bridge checks `syncSource` field before writing: if doc was written by this bridge direction, skip it
- `ditto.startSync()` called in this phase after subscriptions are registered
- Ditto `registerSubscription` active for all 3 collections to pull data from P2P peers (SYNC-05)

### Claude's Discretion
- Exact bridge class architecture (single BridgeManager vs per-collection bridges)
- Coroutine scope/dispatcher strategy for bridge observers
- ChangeGuard implementation details (how to handle `hasPendingWrites` on Firestore side)
- DQL upsert syntax validation (flagged in STATE.md as needing live validation)
- Firestore connection state detection approach (`.info/connected` workaround)
- ViewModel architecture for each screen
- Compose navigation route naming
- Product card visual design details (spacing, colors, typography)

</decisions>

<specifics>
## Specific Ideas

- The demo story is: "Two terminals decrement stock offline, quantities merge correctly when reconnected" — inventory count on product cards makes this visible
- Orders appearing on the other terminal's orders list is the most tangible proof of mesh sync
- Keep the POS UI functional but simple — this is a developer demo, not a production POS app
- Bottom bar cart UX should feel responsive — tap and see count/total update instantly (from Ditto local store, no network latency)

</specifics>

<code_context>
## Existing Code Insights

### Reusable Assets
- `StatusScreen.kt`: Existing Compose screen with koinInject pattern — becomes the Status tab
- `MainActivity.kt`: Already handles DittoSyncPermissions and sets PosTheme — add NavigationBar here
- `viewModelModule` in `di/ViewModelModule.kt`: Empty Koin module stub ready for Phase 3 ViewModels
- `repositoryModule` in `di/RepositoryModule.kt`: Has FirestoreSeeder, add repositories/bridge here
- `Collections.kt` + `Collections.Fields`: Constants for all collection and field names — use in bridge DQL/Firestore queries
- `SeedData.kt` + `FirestoreSeeder.kt`: Seeder wired via Koin — call on app start before bridge

### Established Patterns
- Koin DI: `single { }` for singletons, `viewModel { }` for ViewModels
- MVVM: ViewModels + Compose screens with koinInject
- Data classes in `data/model/` package with dual-purpose Ditto/Firestore shape
- Package structure: `live.ditto.pubsec.pos` with `di/`, `ui/`, `data/model/`, `data/seed/`
- kotlinx-coroutines-play-services for Firestore `.await()`

### Integration Points
- `PosApplication.kt`: Koin modules registered — new repositories and ViewModels auto-register
- `appModule`: Provides Ditto and FirebaseFirestore instances via Koin
- `ditto.startSync()` needs to be called after subscriptions are set up
- Ditto instance from `get<Ditto>()` for DQL operations and subscriptions
- FirebaseFirestore from `get<FirebaseFirestore>()` for listeners and writes

</code_context>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope

</deferred>

---

*Phase: 03-sync-bridge-and-pos-ui*
*Context gathered: 2026-03-04*
