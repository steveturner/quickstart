# Ditto-Firebase POS Bridge

## What This Is

An Android demonstration app using the Ditto Kotlin SDK that showcases bidirectional synchronization between Firebase (cloud) and Ditto (edge P2P). Built around a simple Point-of-Sale use case, the app proves that Ditto can seamlessly extend a Firebase-based architecture with offline-first, peer-to-peer sync — keeping POS terminals operational even when internet/Firebase connectivity is lost.

## Core Value

POS terminals continue processing transactions and stay in sync with each other via Ditto P2P mesh networking even when Firebase/internet connectivity is completely unavailable.

## Requirements

### Validated

(None yet — ship to validate)

### Active

- [ ] Firebase-first architecture with Firestore as the primary cloud data store
- [ ] Ditto SDK integration for edge P2P synchronization between Android devices
- [ ] Bidirectional sync bridge: Firebase ↔ Ditto (changes flow both directions)
- [ ] 3 Firestore/Ditto collections: products, orders, inventory
- [ ] Offline-resilient POS operations — transactions work without internet
- [ ] Conflict resolution strategy for concurrent edits across Firebase and Ditto
- [ ] Simple POS UI: browse products, create orders, track inventory
- [ ] Visual connectivity indicators showing Firebase and Ditto mesh status
- [ ] Demonstration mode showing sync behavior during connectivity changes

### Out of Scope

- Payment processing integration — this is a sync demo, not a payment system
- Multi-tenant / authentication — single-store demo scenario
- Production security hardening — demo/playground credentials acceptable
- iOS or cross-platform — Android only
- Complex POS features (receipts, discounts, tax calculations) — keep it simple

## Context

- Part of the ditto-quickstart repository which contains multi-platform Ditto SDK samples
- Existing Android/Kotlin quickstart in the repo uses MVVM + Jetpack Compose patterns
- Ditto SDK uses "Online Playground" identity for development
- Firebase/Firestore is the customer's existing cloud infrastructure; Ditto extends it with edge sync
- Target audience: developers evaluating Ditto as a complement to their Firebase stack
- The app should clearly demonstrate the value proposition: Firebase handles cloud sync, Ditto handles edge P2P sync, and they work together seamlessly

## Constraints

- **SDK**: Ditto Kotlin SDK + Firebase Android SDK (Firestore)
- **Architecture**: Firebase/Firestore is the source of truth for cloud; Ditto is the source of truth at the edge
- **UI**: Jetpack Compose (consistent with existing quickstart patterns)
- **Collections**: Exactly 3 — products, orders, inventory
- **Build**: Gradle with version catalogs (consistent with repo conventions)

## Key Decisions

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| Firebase as primary, Ditto as edge extension | Matches real-world customer architecture where Firebase is already deployed | — Pending |
| 3 collections (products, orders, inventory) | Covers core POS domain while keeping scope manageable for a demo | — Pending |
| Bidirectional sync bridge pattern | Demonstrates full interop, not just one-way replication | — Pending |

---
*Last updated: 2026-03-03 after initialization*
