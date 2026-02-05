# xCell Print Farm Monitor

## What This Is

A Ditto-powered monitoring system for distributed Firestorm Labs xCell portable 3D print farms. Provides DDIL-resilient visibility into expeditionary manufacturing capacity across a theater of operations, with a web dashboard showing fleet status, production queues, printer health, and finished goods lifecycle tracking.

## Core Value

Commanders see real-time manufacturing capacity across distributed xCell units even when connectivity is degraded — local mesh keeps operators informed, theater-wide sync happens when backhaul allows.

## Requirements

### Validated

(None yet — ship to validate)

### Active

- [ ] Web dashboard showing all xCell units and their status
- [ ] Simulated printer data modeled on real 3D printer farm APIs
- [ ] Print queue visualization (Starcraft-style "units under construction")
- [ ] Printer health monitoring (temps, errors, utilization)
- [ ] Material tracking (powder levels, consumption rates)
- [ ] Finished goods lifecycle (printing → QA → staged → assigned)
- [ ] Local mesh sync (multiple xCells at FOB sync P2P)
- [ ] Theater-wide sync (distributed xCells reconnect when connectivity allows)
- [ ] Fleet-wide aggregate view (total capacity, production rates)

### Out of Scope

- Deep TAK/ATAK integration — notional/mentioned only for this demo
- Real xCell hardware connection — simulated data only
- User authentication — demo assumes trusted environment
- Production deployment hardening — architecture demonstration

## Context

**Hardware context:** xCell is Firestorm Labs' expeditionary manufacturing system — two expandable ISO 20-foot containers with HP MJF 3D printing, capable of producing 50 Group 2 UAS airframes per month. Designed for edge deployment, operational within 24 hours, runs on generator or battery.

**Software context:** This is an architecture demonstration showing how Ditto enables DDIL-resilient monitoring. Part of the ditto-quickstart/pubsec examples alongside cUAS, asset tracking, and other military/government use cases.

**Data model inspiration:** Research OctoPrint, Repetier Server, Prusa Connect, and other 3D printer farm management APIs to inform realistic simulated data structures.

**Mesh topology:**
- Local: Multiple xCells at a FOB mesh directly via Bluetooth/LAN
- Theater: Distributed xCells sync via Ditto's cloud bridge when connectivity allows
- Both patterns demonstrated

**UX inspiration:** Starcraft unit production queue — visual representation of "3 Tempests building, first ready in 12 min, material consumption rate X kg/hr"

## Constraints

- **Stack**: Must use Ditto SDK for sync (this is a Ditto quickstart example)
- **Platform**: Web dashboard as primary interface
- **Data**: Simulated, not real hardware integration
- **Scope**: Architecture demonstration, not production system

## Key Decisions

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| Web-first, TAK notional | Focus demo effort on core value prop | — Pending |
| Simulated printer data | Enables demo without xCell hardware | — Pending |
| Full lifecycle tracking | Shows complete picture from print to assignment | — Pending |
| Ditto C++ SDK for simulators | Consistent with pubsec examples, native performance | — Pending |
| Keep architecture simple/clear | Demo should be easy to understand and extend | — Pending |
| Docker-compose deployment | Easy to spin up full demo environment | — Pending |

---
*Last updated: 2026-02-05 after roadmap approval*
