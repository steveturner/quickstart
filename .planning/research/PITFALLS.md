# Pitfalls Research

**Domain:** Bidirectional Firebase-Ditto sync bridge, Android POS demo app
**Researched:** 2026-03-03
**Confidence:** MEDIUM — Core pitfalls are well-evidenced from official Ditto docs, Firebase docs, and MongoDB connector patterns. Some claims are inferred from analogous bridge architectures.

---

## Critical Pitfalls

### Pitfall 1: Sync Loop — Bridge Writes Trigger Its Own Listener

**What goes wrong:**
The bridge observes Ditto changes and writes to Firestore. Firestore's real-time listener then fires for that write, which writes back to Ditto, which triggers the Ditto observer again. This infinite feedback loop saturates the device with write operations, exhausts Firebase quota, and locks Ditto write transactions.

**Why it happens:**
Both Ditto's `registerObserver` and Firestore's `addSnapshotListener` fire on all document changes — including changes made by the local device. Without an explicit "did I cause this?" check, every bridge write re-enters the cycle.

**How to avoid:**
On the Firestore side: use `SnapshotMetadata.hasPendingWrites` — when `true`, the snapshot reflects a local write that hasn't confirmed from the server yet. Only process snapshots where `hasPendingWrites == false` (i.e., changes that originated remotely from the server). Alternatively filter on `MetadataChanges.EXCLUDE_METADATA_CHANGES` to suppress local-echo events entirely.

On the Ditto side: tag every document written by the bridge with a `syncSource` field (e.g., `"firebase"` or `"ditto"`). In each observer callback, skip documents whose `syncSource` matches the direction the observer serves.

**Warning signs:**
- Firebase writes per minute spike orders of magnitude above user activity
- Ditto logs show "transaction blocked over 30 seconds"
- CPU usage climbs steadily with zero user interaction
- Firestore cost anomalies visible in Firebase console

**Phase to address:** Phase 1 (Bridge Foundation) — this must be designed in from the first commit, not retrofitted.

---

### Pitfall 2: Ditto Initialized in Activity Context Instead of Application Context

**What goes wrong:**
If `Ditto` is instantiated inside an Activity (or inside a ViewModel scoped to an Activity), it gets garbage collected when the Activity rotates or goes to the background. A new Ditto instance is created on the next launch with no connection to previous sync state. Worse, duplicate instances can form, causing split sync graphs.

**Why it happens:**
The Ditto Kotlin SDK requires `DefaultAndroidDittoDependencies(applicationContext)`. Developers familiar with Firebase (which handles its own singleton) assume Ditto does the same. It does not — the app is responsible for singleton lifecycle.

**How to avoid:**
Initialize Ditto exactly once in `Application.onCreate()`, store the singleton there, and inject it via dependency injection or a global accessor. Never pass Activity context to Ditto initialization. ProGuard release builds also require explicit keep rules (`-keep class live.ditto.**`) or you'll get `NoSuchMethodError` crashes in production that don't reproduce in debug builds.

**Warning signs:**
- Sync stops working after screen rotation
- Multiple Ditto instances visible in logs
- "authentication request succeeded" appears multiple times per session
- Release build crashes with `NoSuchMethodError` that don't appear in debug

**Phase to address:** Phase 1 (Project Setup / Ditto Integration) — initialization structure must be correct before any feature work begins.

---

### Pitfall 3: Using Last-Write-Wins for Inventory Quantities

**What goes wrong:**
Inventory stock levels modified concurrently on two offline POS terminals (e.g., terminal A sells 2 units of item X, terminal B sells 3 units of the same item) are merged using Ditto's register (last-write-wins) semantics. The merge discards one write. The final inventory count is wrong — typically over-reporting available stock. Items that are actually sold out get sold again.

**Why it happens:**
Ditto registers use timestamp-based last-write-wins. When both terminals are offline and independently decrement a stock integer, merging picks only one value. The other decrement is silently dropped. This is the correct CRDT behavior for a register, but wrong for a quantity that represents accumulated operations.

**How to avoid:**
Model inventory adjustments as **deltas, not absolute values**. Instead of writing `quantity = 47`, write an adjustment event: `{ productId, delta: -2, terminalId, timestamp }` into a separate `inventoryAdjustments` collection. The current quantity is computed by summing all adjustment deltas. This is append-only and CRDT-safe — no data is lost during merge. The `orders` collection already naturally provides this pattern (each order is an immutable event).

**Warning signs:**
- Inventory count after reconciliation doesn't match sum of sales
- Stock shows positive but product was sold out locally
- Discrepancies only appear after the app has been used offline across two devices

**Phase to address:** Phase 2 (Data Model Design) — the inventory schema must be decided before any write code exists. Retrofitting from absolute values to delta events requires a data migration.

---

### Pitfall 4: Firestore Document IDs and Ditto Document IDs Diverge

**What goes wrong:**
Firestore auto-generates document IDs (e.g., `KjHd83kLm`). Ditto uses its own internal ID system. If these are not explicitly reconciled at bridge design time, documents end up with different identities in each system. Upsert operations create duplicates instead of updating existing documents. Querying "the same order" from each system returns different objects.

**Why it happens:**
Developers write to Ditto with a Ditto-generated `_id`, then write the "same" document to Firestore using Firestore's auto-ID. The bridge has no join key, so every sync cycle creates a new Firestore document.

**How to avoid:**
Use a single canonical ID strategy from the start. The recommended approach: generate a UUID on first write (in Ditto or the app), use that UUID as both the Ditto `_id` and the Firestore document ID. The Firestore SDK accepts explicit document IDs (`collection.document(uuid).set(data)`). The bridge always upserts by this canonical ID in both directions. Fields used as ID mapping must be immutable and always present — never null, never changed after creation.

**Warning signs:**
- Firestore collection document count grows faster than order/product count
- Querying by business key (e.g., order number) returns multiple documents
- Ditto and Firestore show different counts for the same collection

**Phase to address:** Phase 2 (Data Model Design) — the ID strategy must be frozen before any writes happen. ID schema changes require full data migration.

---

### Pitfall 5: Observer Lifecycle Not Managed — Memory Leaks and Stale Subscriptions

**What goes wrong:**
Ditto `registerObserver` and `registerSubscription` calls accumulate without ever calling `stop()` or `cancel()`. In a ViewModel-based architecture, creating observers in `init {}` without cleanup in `onCleared()` means every ViewModel recreation (rotation, navigation back-stack) adds another observer. Battery drain increases, duplicate writes to Firestore occur, and memory grows unbounded.

**Why it happens:**
Ditto's reactive SDK requires explicit resource cleanup. Kotlin coroutine Flows from Firestore similarly must be cancelled on the appropriate lifecycle scope. Firebase's `addSnapshotListener` returns a `ListenerRegistration` that must be explicitly removed. When using `ViewModel`, developers often forget to implement `onCleared()`.

**How to avoid:**
Store every `DittoSyncSubscription`, `DittoCancelable`, and Firestore `ListenerRegistration` as ViewModel properties. Cancel all of them in `ViewModel.onCleared()`. For Ditto observers used in Composables, wrap in `DisposableEffect`. Launch Firestore listeners in `viewModelScope` using `repeatOnLifecycle` so cancellation is automatic. Keep Ditto itself as a process-scoped singleton — only the observers are scoped to the ViewModel.

**Warning signs:**
- Firestore snapshot listener callback fires more than once per document change
- Ditto logs show duplicate write transactions for the same document
- Memory usage climbs steadily over a session with no activity
- Crash on `DittoQueryResultItem` accessed after parent result closed

**Phase to address:** Phase 1 (Architecture Setup) — the lifecycle pattern must be established in the first ViewModel before features are added.

---

### Pitfall 6: Soft Deletes Not Honored Across the Bridge

**What goes wrong:**
A product is deleted on one POS terminal (offline) using Ditto's soft delete (`deleted = true`). When the bridge syncs to Firestore, it hard-deletes the Firestore document. Later, when connectivity restores and Ditto syncs with other peers, the deleted document is resurrected from Firestore because the bridge re-writes any Firestore document that doesn't exist in Ditto.

**Why it happens:**
CRDTs use "add-wins" semantics for maps — a deletion in Ditto is represented as a tombstone, not a physical removal. If the Firebase side performs a physical delete and the bridge naively treats "missing in Firestore" as "write it back to Ditto," deleted records come back to life.

**How to avoid:**
Use soft deletes consistently in both systems. A `deleted: true` field in Firestore and a `deleted: true` field in Ditto documents. The bridge propagates the `deleted` flag, never physically removes documents. UI filters out `deleted == true`. This matches the pattern already used in the existing quickstart repo (`deleted` field in task documents).

**Warning signs:**
- Deleted products reappear after reconnecting to cloud
- Order history shows cancelled orders as active
- Document counts in Ditto and Firestore diverge after delete operations

**Phase to address:** Phase 2 (Data Model) and Phase 3 (Bridge Implementation) — the delete strategy must be in the schema and enforced in the bridge write logic.

---

## Technical Debt Patterns

| Shortcut | Immediate Benefit | Long-term Cost | When Acceptable |
|----------|-------------------|----------------|-----------------|
| Firestore auto-IDs without bridge mapping | Faster initial writes | Duplicate document explosion; bridge cannot upsert | Never — fix ID strategy in Phase 2 |
| Hard delete instead of soft delete | Simpler code | Deleted data resurrects; CRDT tombstones break | Never for synced data |
| Absolute inventory integers instead of delta events | Simpler reads | Silent data loss on concurrent offline edits | Never for mutable quantities |
| Single Coroutine scope for all bridge operations | Less wiring | Bridge cannot be paused independently; resource leaks on configuration change | Acceptable in Phase 1 prototype only |
| No syncSource field tagging | Fewer fields to manage | Sync loops in any connectivity scenario | Never — must be in from Phase 1 |
| Verbose Ditto logging left enabled | Easier debugging | "Significantly slows replication" per Ditto docs; demo looks slow | OK in debug builds; remove for demo build |

---

## Integration Gotchas

| Integration | Common Mistake | Correct Approach |
|-------------|----------------|------------------|
| Ditto init | `DefaultAndroidDittoDependencies(this)` inside Activity | `DefaultAndroidDittoDependencies(applicationContext)` in `Application.onCreate()` |
| Ditto permissions | Declare in manifest, forget runtime request | Call `DittoSyncPermissions.missingPermissions()` and then `ditto.refreshPermissions()` after grant |
| Firestore snapshot listener | Process all snapshot events equally | Check `snapshot.metadata.hasPendingWrites` — skip local-echo writes to prevent sync loops |
| Ditto observer | Leave `DittoCancelable` unreferenced (GC'd immediately) | Store reference, call `stop()` in `ViewModel.onCleared()` |
| Ditto DQL subscriptions | Use `"SELECT * FROM COLLECTION"` wildcard | Use collection-specific subscriptions per the 3 POS collections to limit sync scope |
| Ditto ProGuard | Release build works in debug | Add `-keep class live.ditto.**` to ProGuard rules; without it, release crashes |
| Firebase + Ditto timestamps | Mix Firebase `Timestamp` with Unix millis | Normalize to Unix millis (Long) in all Ditto documents; convert at bridge boundary |

---

## Performance Traps

| Trap | Symptoms | Prevention | When It Breaks |
|------|----------|------------|----------------|
| Subscribing to all collections at unlimited scope | Slow initial sync, high memory on first launch | Scope subscriptions to only active session data (e.g., today's orders) | Noticeable with >500 order documents |
| Writing full document on any field change | Excessive Firestore writes and Ditto write transactions | Write only changed fields; use Firestore `update()` not `set()` for patches | 10+ terminals all syncing simultaneously |
| Firestore `set()` instead of `update()` | Overwrites fields the bridge didn't intend to touch | Use `set(data, SetOptions.merge())` or `update()` for partial writes | Every write |
| Verbose Ditto logging in production/demo | Demo shows sluggish sync, battery drains fast | Disable verbose logging for demo builds; Ditto docs explicitly warn this "significantly slows replication" | Always in demo context |
| `ORDER BY` mutable field in Ditto live query | Query re-evaluates and re-notifies on every write | Sort in ViewModel/UI layer, not in DQL subscription query | Any live query with mutable sort key |

---

## Security Mistakes

| Mistake | Risk | Prevention |
|---------|------|------------|
| Embedding Firestore service account credentials in APK | Credential exposure; unauthorized cloud access | Use Playground token for Ditto (acceptable per project scope); for Firestore use anonymous auth or environment-injected config — never hardcode production service account in app code |
| Open Firestore security rules during development | Any device can read/write all POS data | Lock rules to authenticated UID or use emulator; even for a demo, don't leave `allow read, write: if true` on a live Firebase project |
| Ditto Playground identity in demo vs. production | Demo shows it works; customer uses same code in prod | Add visible warning in the UI and README that Playground is not for production; point to Online with Authentication docs |

---

## UX Pitfalls

| Pitfall | User Impact | Better Approach |
|---------|-------------|-----------------|
| No connectivity state indicator | User doesn't know if sync is active; operates on stale data without realizing | Show explicit Firebase status (cloud icon with color) and Ditto mesh status (peer count or icon) — this is a demo requirement |
| Optimistic UI without rollback | POS shows "order created" then silently fails if Ditto write throws | Wrap writes in try-catch; show error state; Ditto write transactions are synchronous so success/failure is known before UI updates |
| Connectivity demo hard to trigger | Developer can't easily show offline → online sync during a presentation | Build explicit "Demo Mode" toggle that simulates disconnecting Firebase (but keeps Ditto P2P active) — this is a stated project requirement |
| Generic "Sync failed" error | User or demo audience can't tell what failed | Distinguish Firebase connectivity errors from Ditto sync errors in status indicators |

---

## "Looks Done But Isn't" Checklist

- [ ] **Sync loop prevention:** Verify that writing from Firebase to Ditto does NOT trigger a write back to Firebase. Test by watching Firestore write count when one document changes — it should be exactly 1, not growing.
- [ ] **Offline inventory accuracy:** Create an order on Device A (offline), create a conflicting order on Device B (offline), reconnect — verify inventory decrements from both orders are both applied correctly.
- [ ] **Ditto singleton:** Rotate the screen 5 times and check logs — "Ditto SDK initialized" should appear exactly once per app process launch.
- [ ] **Observer cleanup:** Navigate away from the main screen and back 10 times — Firestore write count from sync observers should not increase with each navigation.
- [ ] **Soft delete round-trip:** Delete a product on one device. Reconnect. Verify the product is deleted on all peers and in Firestore, and does NOT reappear after a Firestore → Ditto sync cycle.
- [ ] **ProGuard release build:** Build a signed release APK and run it — verify sync works and no `NoSuchMethodError` crashes appear.
- [ ] **Firestore offline persistence conflicts with bridge:** Firestore's built-in offline persistence cache can replay writes on reconnect that the bridge has already processed. Verify no duplicate Ditto writes result from Firestore's internal offline replay.

---

## Recovery Strategies

| Pitfall | Recovery Cost | Recovery Steps |
|---------|---------------|----------------|
| Sync loop discovered in production | HIGH | Hotfix: add `hasPendingWrites` guard to Firestore listener and `syncSource` tag to Ditto observer; deploy; purge duplicate Firestore documents manually |
| Wrong ID strategy (duplicates in Firestore) | HIGH | Data migration: query all Firestore collections, deduplicate by canonical business key, delete orphans; update bridge ID logic; requires coordinated downtime |
| Inventory data loss from LWW on integers | HIGH | Audit: compute expected inventory from order history; manually correct; add delta event model; requires schema migration |
| Activity-context Ditto instance (GC crashes) | MEDIUM | Refactor: move Ditto singleton to Application class; requires removing all Activity-scoped Ditto references; testable by rotating device |
| Observer leaks | LOW-MEDIUM | Audit all ViewModel classes for missing `onCleared()`; add cancellation; memory profiler confirms leak elimination |
| Verbose logging in demo build | LOW | Add debug-only logging guard or separate build variant; no data impact |

---

## Pitfall-to-Phase Mapping

| Pitfall | Prevention Phase | Verification |
|---------|------------------|--------------|
| Sync loop | Phase 1 (Bridge Foundation) | Write one doc to Ditto, count Firestore write operations — should be exactly 1 |
| Ditto Activity context | Phase 1 (Project Setup) | Rotate screen during sync; verify no re-initialization in logs |
| Inventory LWW data loss | Phase 2 (Data Model) | Offline concurrent inventory test: both decrements must survive merge |
| Document ID divergence | Phase 2 (Data Model) | Write one product to Ditto; verify same UUID appears in Firestore |
| Observer lifecycle leaks | Phase 1 (Architecture) | Navigate home/back 10x; no duplicate observer callbacks |
| Soft delete resurrection | Phase 2 (Data Model) + Phase 3 (Bridge) | Delete a product; trigger full sync cycle; verify it stays deleted |
| ProGuard release crash | Phase 1 (Setup) | Build signed release APK and run before any demo |
| Missing runtime permissions | Phase 1 (Setup) | Fresh install on Android 12+ device; grant permissions; verify sync starts |

---

## Sources

- Ditto Kotlin SDK Troubleshooting: https://docs.ditto.live/sdk/latest/troubleshooting
- Ditto Kotlin Installation Guide: https://docs.ditto.live/install-guides/kotlin
- Ditto Kotlin SDK Release Notes (bug fix history): https://docs.ditto.live/sdk/latest/release-notes/kotlin
- Ditto Consistency Models (CRDT/LWW behavior): https://docs.ditto.live/sdk/v4-7/sync/consistency-models
- Ditto MongoDB Connector (bridge pattern reference): https://docs.ditto.live/cloud/mongodb-connector
- Firebase Firestore SnapshotMetadata (hasPendingWrites): https://firebase.google.com/docs/reference/kotlin/com/google/firebase/firestore/SnapshotMetadata
- Firebase Firestore Best Practices: https://firebase.google.com/docs/firestore/best-practices
- Offline Sync & Conflict Resolution Patterns (2026): https://www.sachith.co.uk/offline-sync-conflict-resolution-patterns-architecture-trade%E2%80%91offs-practical-guide-feb-19-2026/
- Ditto jepsen-ditto bridge implementation (in-repo reference): pubsec/jepsen-ditto/bridge/src/main/kotlin/live/ditto/jepsen/bridge/DittoManager.kt

---
*Pitfalls research for: Firebase-Ditto POS Bridge Android app*
*Researched: 2026-03-03*
