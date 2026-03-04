# Phase 2: Data Models - Context

**Gathered:** 2026-03-04
**Status:** Ready for planning

<domain>
## Phase Boundary

Schema contracts for all three collections (Products, Orders, Inventory) finalized and enforced before any write code exists. Canonical UUID IDs, soft delete convention, SyncSource tagging, and inventory delta-event pattern locked in. Requirements: DATA-01 through DATA-05.

</domain>

<decisions>
## Implementation Decisions

### Field design & types
- Monetary values stored as Long cents (e.g., 999 = $9.99) — avoids floating-point rounding, standard POS practice
- Order.items is an embedded `List<OrderLineItem>` with productId, productName, quantity, unitPrice — denormalized for offline reads without product lookups
- Product.category is a plain String field (e.g., "Beverage", "Food", "Merchandise") — no separate category collection
- Order status: simple OPEN/FULFILLED enum — minimal for demo, v2 requirement POLH-03 adds progression later
- All models carry `_id: String` (canonical UUID), `deleted: Boolean`, `syncSource: String`

### Inventory delta events
- Use Ditto's built-in CRDT counter type for inventory quantity — concurrent offline decrements merge correctly
- Firestore side uses `FieldValue.increment()` for equivalent atomic decrement
- No separate event log collection needed — CRDT counter handles conflict resolution natively
- InventoryItem tracks `lastModifiedBy: String` (terminalId) for demo visibility

### Seed data catalog
- Coffee shop theme: ~8-10 items across Beverages, Food, Merchandise categories
- Example: espresso, latte, cappuccino, croissant, sandwich, mug, etc. with realistic prices
- Seed data lives in a Kotlin `SeedData` object with hardcoded `List<Product>` — no external JSON files
- Seeder class writes to Firestore on first run (checks if collection is empty)
- Each product seeded with initial inventory quantity

### SyncSource tagging
- Every model has a `syncSource: String` field with values: "firestore", "ditto", "local"
- String (not enum) for Ditto DQL compatibility — DQL stores/queries strings natively
- Bridge checks syncSource before writing to prevent infinite sync loops
- `lastSyncedAt: Long` (epoch millis) included alongside syncSource for demo debugging

### Claude's Discretion
- Exact product names, prices, and quantities in seed data
- Kotlin data class naming conventions (e.g., `OrderLineItem` vs `LineItem`)
- Whether to use Kotlin value classes for IDs
- UUID generation strategy (UUID.randomUUID() vs other)
- Repository interface design within Koin modules

</decisions>

<specifics>
## Specific Ideas

- Models should be dual-purpose: work as both Ditto document shapes and Firestore document shapes with no field name drift
- The schema is a contract — Phase 3 bridge code depends on field names being identical in both systems
- Inventory delta pattern is the key demo differentiator: "Two terminals decrement stock offline, quantities merge correctly when reconnected"

</specifics>

<code_context>
## Existing Code Insights

### Reusable Assets
- `repositoryModule` in `di/RepositoryModule.kt`: Empty Koin module stub ready for Phase 2 repository classes
- `appModule` in `di/AppModule.kt`: Provides Ditto and FirebaseFirestore instances via Koin — repositories will inject these
- `PosApplication.kt`: Koin startup with `modules(appModule, repositoryModule, viewModelModule)` — new modules auto-register

### Established Patterns
- Koin DI: `single { }` for singletons, modules registered in PosApplication.onCreate()
- MVVM: ViewModels + Compose screens (Phase 1 established with StatusScreen)
- Package structure: `live.ditto.pubsec.pos` with `di/`, `ui/`, `ui/theme/` sub-packages
- BuildConfig for environment variables via `loadEnvProperties()`

### Integration Points
- `repositoryModule` is the hook — add repository classes here, they'll be available app-wide via Koin
- Ditto instance from `get<Ditto>()` — repositories use this for DQL operations
- FirebaseFirestore instance from `get<FirebaseFirestore>()` — repositories use this for Firestore reads/writes
- `viewModelModule` stub ready for Phase 3 ViewModels that consume repositories

</code_context>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope

</deferred>

---

*Phase: 02-data-models*
*Context gathered: 2026-03-04*
