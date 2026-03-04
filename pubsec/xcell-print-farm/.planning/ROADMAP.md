# Roadmap: xCell Print Farm Monitor

## Overview

A Ditto-powered 3D print farm monitoring system built for DDIL environments. Six phases deliver progressively wider visibility: local foundation with offline-first patterns, realistic printer simulation, single-device dashboard, multi-device mesh sync at FOB level, theater-wide cloud sync, and fleet-wide aggregation. Each phase builds on proven capabilities from prior phases, with mesh networking as the key differentiator over commercial solutions.

## Phases

- [x] **Phase 1: Local Foundation** - Offline-first React app with Ditto SDK integration
- [x] **Phase 2: Printer Simulation** - C++ simulator generating realistic printer telemetry
- [x] **Phase 3: Local Dashboard** - Single-device web UI showing printer status and metrics
- [x] **Phase 4: Multi-Device Mesh** - P2P sync between xCells via BLE/LAN at FOB
- [ ] **Phase 5: Cloud Sync** - Theater-wide visibility via Ditto cloud bridge
- [ ] **Phase 6: Fleet Dashboard** - Aggregated analytics and production metrics

## Phase Details

### Phase 1: Local Foundation
**Goal**: Developer can run React app that initializes Ditto SDK and persists data locally
**Depends on**: Nothing (first phase)
**Requirements**: INFRA-01, INFRA-03, INFRA-04, DDIL-01
**Success Criteria** (what must be TRUE):
  1. Docker Compose brings up full stack (React app + Ditto backend)
  2. React app successfully initializes Ditto SDK and creates local store
  3. App writes test document to Ditto and observes it reactively
  4. App works fully offline without network connectivity
**Plans**: 4 plans

Plans:
- [x] 01-01-PLAN.md - Project scaffolding (React + Vite + Tailwind + Ditto SDK)
- [x] 01-02-PLAN.md - Ditto SDK initialization with Online Playground identity
- [x] 01-03-PLAN.md - Reactive document creation and observation
- [x] 01-04-PLAN.md - Docker containerization with docker-compose

### Phase 2: Printer Simulation
**Goal**: Simulator generates realistic printer telemetry that flows into Ditto store
**Depends on**: Phase 1
**Requirements**: SIM-01, SIM-02, SIM-03, SIM-04, SIM-05, INFRA-02
**Success Criteria** (what must be TRUE):
  1. C++ simulator runs as Docker container and connects to Ditto
  2. Simulator generates printer status transitions with realistic timing
  3. Simulator models print jobs with progress over time and material consumption
  4. Simulator injects occasional errors and failures
  5. Multiple simulator instances run independently with unique printer IDs
**Plans**: 3 plans

Plans:
- [x] 02-01-PLAN.md - C++ simulator core (state machine and types)
- [x] 02-02-PLAN.md - Ditto SDK integration and main entry point
- [x] 02-03-PLAN.md - Docker containerization and multi-instance deployment

### Phase 3: Local Dashboard
**Goal**: Operator sees real-time printer status on single-device web dashboard
**Depends on**: Phase 2
**Requirements**: MNTR-01, MNTR-02, MNTR-03, MNTR-04, MNTR-05, JOB-01, JOB-02, JOB-03, MAT-01, MAT-02, MAT-03, LIFE-01, LIFE-02, LIFE-03
**Success Criteria** (what must be TRUE):
  1. Dashboard displays all printer cards showing real-time status (idle/printing/error/offline)
  2. Dashboard shows print job progress with time remaining and Starcraft-style production queue
  3. Dashboard displays temperature readings and air quality data in real time
  4. Dashboard shows material levels with consumption rates and low material alerts
  5. Dashboard tracks finished goods lifecycle (printing -> QA -> staged -> assigned)
**Plans**: 4 plans

Plans:
- [x] 03-01-PLAN.md - Data layer and TypeScript types for printer schema
- [x] 03-02-PLAN.md - 3D globe visualization with location markers
- [x] 03-03-PLAN.md - Printer table with TanStack Table
- [x] 03-04-PLAN.md - Detail panel, job queue, and Dashboard orchestration

### Phase 4: Multi-Device Mesh
**Goal**: Multiple xCells at same FOB sync printer data via P2P mesh
**Depends on**: Phase 3
**Requirements**: DDIL-02, DDIL-03, FLEET-01, FLEET-03
**Success Criteria** (what must be TRUE):
  1. Two dashboard instances on separate devices see same printer data via mesh sync
  2. Dashboard shows mesh health indicator with peer count
  3. When device goes offline and returns, mesh auto-reconnects and syncs changes
  4. Dashboard groups printers by site (local FOB vs theater-wide)
**Plans**: 4 plans

Plans:
- [x] 04-01-PLAN.md - Mesh types and usePresence hook
- [x] 04-02-PLAN.md - Site filter support in usePrinters
- [x] 04-03-PLAN.md - MeshIndicator and SiteSelector components
- [x] 04-04-PLAN.md - Dashboard integration and verification

### Phase 5: Cloud Sync
**Goal**: Theater commander sees all distributed xCells when connectivity allows
**Depends on**: Phase 4
**Requirements**: DDIL-04
**Success Criteria** (what must be TRUE):
  1. Cloud dashboard instance syncs with distributed xCells via Ditto Big Peer
  2. When xCell comes online, new printer data flows to cloud dashboard
  3. Cloud dashboard shows last-seen timestamps for offline xCells
  4. Theater-wide data persists in cloud even when local xCells offline
**Plans**: TBD

Plans:
- [ ] 05-01: TBD during planning

### Phase 6: Fleet Dashboard
**Goal**: Commander sees fleet-wide production metrics and capacity forecasting
**Depends on**: Phase 5
**Requirements**: FLEET-02, FLEET-04
**Success Criteria** (what must be TRUE):
  1. Fleet dashboard shows aggregate status across all xCells (total capacity, active printers)
  2. Dashboard displays fleet-wide production metrics (units completed per period, success rate)
  3. Dashboard shows analytics and trends (utilization over time, failure patterns)
  4. Dashboard supports filtering by site/region for theater-level planning
**Plans**: TBD

Plans:
- [ ] 06-01: TBD during planning

## Progress

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 1. Local Foundation | 4/4 | Complete | 2026-02-05 |
| 2. Printer Simulation | 3/3 | Complete | 2026-02-05 |
| 3. Local Dashboard | 4/4 | Complete | 2026-02-06 |
| 4. Multi-Device Mesh | 4/4 | Complete | 2026-02-06 |
| 5. Cloud Sync | 0/TBD | Not started | - |
| 6. Fleet Dashboard | 0/TBD | Not started | - |

---
*Roadmap created: 2026-02-05*
*Depth: standard (6 phases)*
*Coverage: 24/24 v1 requirements mapped*
