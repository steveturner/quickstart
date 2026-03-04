# Feature Research

**Domain:** Firebase-Ditto POS Bridge — Android demo app for developer SDK evaluation
**Researched:** 2026-03-03
**Confidence:** HIGH (core Ditto/Firebase APIs), MEDIUM (POS demo conventions)

---

## Feature Landscape

### Table Stakes (Demo Fails Without These)

These features are the minimum needed to prove the core value proposition: Ditto extends
Firebase with offline-capable P2P sync. Missing any of these leaves the demo unconvincing.

| Feature | Why Expected | Complexity | Notes |
|---------|--------------|------------|-------|
| Product catalog (read-only browse) | Developers need something to sell; no products = no orders | LOW | Static-ish data synced from Firebase; Ditto REGISTER type for fields |
| Create order (add items, submit) | Core POS workflow; without order creation there is nothing to sync | MEDIUM | Order document written locally to Ditto first; bridge propagates to Firestore |
| Inventory decrement on order | Proves concurrent write conflict scenario — the most interesting sync demo | MEDIUM | Use Ditto REGISTER LWW for quantity; decrement must work offline |
| Firebase connectivity indicator | Developers need to see when cloud sync is live vs. offline | LOW | Firestore has no native connection listener; use NetworkCallback + manual ping or Realtime Database `.info/connected` as proxy |
| Ditto mesh connectivity indicator | Must show when P2P edge sync is active and how many peers | LOW | `ditto.presence.observe()` delivers peer graph updates; show peer count |
| Offline operation without network | The core proof — POS must keep working when Firebase unreachable | MEDIUM | Ditto local store provides reads/writes; bridge pauses and queues |
| Reconnect sync propagation | Changes made offline must sync to Firebase when reconnected | MEDIUM | Firebase SnapshotListeners auto-resume; bridge re-processes Ditto events |
| Bidirectional data flow (Firebase → Ditto) | Proves interoperability is two-way; Firebase changes must reach Ditto | HIGH | Firestore SnapshotListener writes into Ditto store; hardest part of bridge |
| Bidirectional data flow (Ditto → Firebase) | Proves interoperability is two-way; Ditto changes must reach Firestore | HIGH | Ditto `registerObserver` triggers Firestore writes; must handle offline queuing |
| 3 collections (products, orders, inventory) | Covers enough domain to be credible; matches PROJECT.md constraint | MEDIUM | One Firestore collection + one Ditto collection per domain, bridged |

### Differentiators (Wow Factor for Developer Audience)

These features are not required for the concept proof, but they elevate the demo from
"it technically works" to "I understand why I'd use this."

| Feature | Value Proposition | Complexity | Notes |
|---------|-------------------|------------|-------|
| Conflict resolution visualization | Shows CRDT merge in action — makes the abstract concrete | MEDIUM | Place two orders on same product while offline; show both orders survive, inventory converges |
| Live peer count badge | Developers immediately grasp "the mesh has 3 peers syncing" | LOW | Ditto `presence.observe()` with peer count badge in toolbar |
| Sync event log / activity feed | Shows bridge activity in real time — invaluable for developer trust | MEDIUM | In-memory log of "Firestore → Ditto" and "Ditto → Firestore" events with timestamps |
| Per-terminal offline mode toggle | Explicit UI button to simulate losing Firebase connectivity | LOW | Disable Firestore listeners programmatically; Ditto keeps running; re-enable to trigger sync |
| Visual diff on sync (highlight changed rows) | Makes sync visible, not invisible | LOW | Briefly highlight rows that changed due to incoming sync event |
| Order status progression (open → fulfilled) | Demonstrates state changes flowing across the mesh | LOW | Order fields update through bridge; status visible on all synced peers |

### Anti-Features (Explicitly Out of Scope)

These features are commonly requested or seem natural but would harm the demo's clarity
or violate stated project constraints.

| Feature | Why Requested | Why Problematic | Alternative |
|---------|---------------|-----------------|-------------|
| Payment processing | "Real POS needs payment" | Adds PCI scope, SDK complexity, fake card flows that distract from sync story | Show order status transition to "paid" as a simple field update — sync is the story |
| User authentication / login screen | "Production apps need auth" | Adds Firebase Auth dependency, credential management, and onboarding friction for evaluators | Use Ditto Online Playground identity; document the auth gap explicitly in README |
| Receipt generation / printing | "POS produces receipts" | Requires printer SDK, PDF generation — totally orthogonal to sync demo | Not needed; out of scope per PROJECT.md |
| Tax and discount calculation | "Real POS has pricing rules" | Business logic noise that obscures the sync architecture signal | Fixed prices; no discounts |
| Multi-store / multi-tenant | "Real retail has many locations" | Multiplies data model complexity; auth and data isolation become the story instead of sync | Single store, single Firebase project |
| Barcode scanner integration | "Real POS scans barcodes" | Hardware dependency breaks demo portability | Tap-to-add from product list |
| Complex inventory replenishment workflow | "Stock needs to be managed" | Reorder workflows, supplier integration — far out of scope | Inventory count is the only inventory feature needed |
| Persistent sync event log (database) | "Need to audit sync history" | Adds schema complexity; log fills storage over time | In-memory ring buffer cleared on restart |
| Real-time chat between terminals | "Mesh could carry messages" | Scope creep; separate demo category | Peer count indicator communicates mesh presence adequately |
| iOS support | "Demo should be cross-platform" | Doubles development effort; Android-only per PROJECT.md constraint | Android only |

---

## Feature Dependencies

```
[Firebase project + Firestore collections]
    └──required by──> [Firebase → Ditto bridge]
    └──required by──> [Ditto → Firebase bridge]
    └──required by──> [Firebase connectivity indicator]

[Ditto SDK initialization]
    └──required by──> [Firebase → Ditto bridge]
    └──required by──> [Ditto → Firebase bridge]
    └──required by──> [Ditto mesh connectivity indicator]
    └──required by──> [Live peer count badge]
    └──required by──> [Per-terminal offline mode toggle]

[Product catalog]
    └──required by──> [Create order]
                          └──required by──> [Inventory decrement on order]

[Bidirectional bridge (both directions)]
    └──required by──> [Offline operation without network]
    └──required by──> [Reconnect sync propagation]
    └──required by──> [Conflict resolution visualization] (differentiator)

[Sync event log] ──enhances──> [Bidirectional bridge visibility]
[Visual diff on sync] ──enhances──> [Reconnect sync propagation]
[Per-terminal offline mode toggle] ──enables──> [Conflict resolution visualization]
```

### Dependency Notes

- **Firebase project required before bridge**: Both bridge directions need a live Firestore instance. Firebase setup is phase zero — no bridge code works without it.
- **Ditto SDK init required before all Ditto features**: Initialization (App ID, Playground Token) gates every Ditto capability.
- **Product catalog before orders**: Orders reference products; catalog data must exist (can be seeded from Firebase or hardcoded initially).
- **Both bridge directions required for offline demo**: One-way sync is not enough to prove the value proposition. Firebase → Ditto brings catalog/inventory updates; Ditto → Firebase persists orders.
- **Offline mode toggle enables conflict demo**: The conflict resolution differentiator requires deliberately isolating a device, making orders, then reconnecting. The toggle makes this repeatable without fiddling with device settings.

---

## MVP Definition

### Launch With (v1) — Proves the Core Concept

- [ ] Firebase project setup + Firestore seeded with products and inventory — without this, nothing runs
- [ ] Ditto SDK initialized with Online Playground identity — without this, no edge sync
- [ ] Product catalog screen (list products from Ditto, sourced from Firebase via bridge) — gives something to interact with
- [ ] Create order screen (select products, submit order) — the core POS action
- [ ] Inventory decrement when order placed — proves concurrent write scenario
- [ ] Firebase → Ditto bridge (Firestore SnapshotListener writes into Ditto) — one direction of the bridge
- [ ] Ditto → Firebase bridge (Ditto observer writes to Firestore) — other direction of the bridge
- [ ] Firebase connectivity indicator (connected / disconnected) — makes the demo legible
- [ ] Ditto peer count indicator (mesh size) — makes the P2P story visible
- [ ] Offline operation: place orders when Firebase unreachable, sync on reconnect — the headline demo moment

### Add After Core Is Validated (v1.x)

- [ ] Per-terminal offline mode toggle — makes demo repeatable without airplane mode; add when giving live demos to customers
- [ ] Sync event log / activity feed — add when evaluators ask "how do I know what the bridge is doing?"
- [ ] Order status progression (open → fulfilled) — add to make orders feel more complete
- [ ] Visual diff on sync (highlight changed rows) — low effort, high legibility payoff

### Future Consideration (v2+)

- [ ] Conflict resolution visualization — high demo value but requires scripted multi-device scenario; defer until live demo polish phase
- [ ] Live peer metadata display (device name per peer) — nice for multi-device demos with named terminals; not needed for solo evaluation

---

## Feature Prioritization Matrix

| Feature | Demo Value | Implementation Cost | Priority |
|---------|------------|---------------------|----------|
| Firebase setup + Firestore seeding | HIGH | LOW | P1 |
| Ditto SDK initialization | HIGH | LOW | P1 |
| Product catalog (browse) | HIGH | LOW | P1 |
| Create order | HIGH | MEDIUM | P1 |
| Inventory decrement | HIGH | MEDIUM | P1 |
| Firebase → Ditto bridge | HIGH | HIGH | P1 |
| Ditto → Firebase bridge | HIGH | HIGH | P1 |
| Firebase connectivity indicator | HIGH | LOW | P1 |
| Ditto peer count indicator | HIGH | LOW | P1 |
| Offline operation (Firebase down) | HIGH | MEDIUM | P1 |
| Reconnect sync propagation | HIGH | MEDIUM | P1 |
| Per-terminal offline mode toggle | MEDIUM | LOW | P2 |
| Sync event log | MEDIUM | MEDIUM | P2 |
| Order status progression | MEDIUM | LOW | P2 |
| Visual diff on sync | MEDIUM | LOW | P2 |
| Conflict resolution visualization | HIGH | HIGH | P3 |
| Live peer metadata display | LOW | LOW | P3 |

**Priority key:**
- P1: Must have for launch — demo fails without it
- P2: Should have — makes demo clearly better
- P3: Nice to have — polish and advanced scenarios

---

## Competitor / Reference App Analysis

The primary reference is Ditto's own `demoapp-pos-kds` (GitHub: getditto/demoapp-pos-kds).
That demo is Ditto-only (no Firebase). This project's differentiator is the Firebase bridge.

| Feature | Ditto demoapp-pos-kds | Firebase-only POS (flutter_pos) | This Project |
|---------|-----------------------|----------------------------------|--------------|
| Order creation | Yes | Yes | Yes |
| Kitchen display (KDS) | Yes (separate view) | No | No — out of scope |
| Inventory tracking | No | Yes (SQLite) | Yes (3 collections) |
| Firebase sync | No | Yes | Yes (primary cloud store) |
| Edge P2P sync | Yes (Ditto only) | No | Yes (Ditto extends Firebase) |
| Offline operation | Yes | Yes (local queue) | Yes (Ditto local store) |
| Conflict resolution | Implicit (CRDTs) | Manual queue replay | Implicit (Ditto CRDTs) |
| Connectivity indicators | No | No | Yes (both Firebase + Ditto) |
| Bidirectional bridge | N/A | N/A | Yes — the core differentiator |

---

## Sources

- [Ditto Demo Apps](https://www.ditto.com/demo-apps) — inventory, POS, and chat demo descriptions
- [GitHub: getditto/demoapp-pos-kds](https://github.com/getditto/demoapp-pos-kds) — Ditto POS/KDS reference app; order status model, UI patterns
- [Ditto: Using Mesh Presence](https://docs.ditto.live/sdk/latest/sync/using-mesh-presence) — `presence.observe()` API, peer graph, peer metadata
- [Ditto: Consistency Models](https://docs.ditto.live/sdk/v4-7/sync/consistency-models) — CRDT types (Registers LWW, Maps add-wins) for POS data
- [Ditto: An Inside Look at Delta State CRDTs](https://www.ditto.com/blog/dittos-delta-state-crdts) — Counter, Register, Map types; conflict handling
- [Firebase: Access Data Offline (Firestore)](https://firebase.google.com/docs/firestore/manage-data/enable-offline) — offline persistence behavior
- [Firebase: Build Presence in Cloud Firestore](https://firebase.google.com/docs/firestore/solutions/presence) — no native connection state listener; workaround via Realtime Database
- [GitHub Issue: Firestore connection state listener #947](https://github.com/firebase/firebase-android-sdk/issues/947) — confirms Firestore has no native `onConnected()` callback (MEDIUM confidence — issue may be resolved; verify against current SDK)
- [Ditto on LinkedIn: Chick-fil-A POS case study](https://www.linkedin.com/posts/dittolive_chick-fil-a-point-of-sale-system-goes-cloud-optional-activity-7093364988093861888-GWnp) — real-world POS + Ditto deployment validation
- [GitHub: elrizwiraswara/flutter_pos](https://github.com/elrizwiraswara/flutter_pos) — offline-first POS reference (Firebase + SQLite queue pattern)

---

*Feature research for: Firebase-Ditto POS Bridge Android demo*
*Researched: 2026-03-03*
