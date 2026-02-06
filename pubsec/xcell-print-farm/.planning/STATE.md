# Project State

## Project Reference

See: .planning/PROJECT.md (updated 2026-02-04)

**Core value:** Commanders see real-time manufacturing capacity across distributed xCell units even when connectivity is degraded
**Current focus:** Phase 3 - Local Dashboard (COMPLETE)

## Current Position

Phase: 3 of 6 (Local Dashboard) - COMPLETE
Plan: 4 of 4 complete
Status: Phase complete
Last activity: 2026-02-06 - Completed 03-04-PLAN.md (Detail Panel & Job Queue)

Progress: [███████░░░] 11/11 plans for Phase 3 (~100%)

## Performance Metrics

**Velocity:**
- Total plans completed: 10
- Average duration: 4 min
- Total execution time: 0.55 hours

**By Phase:**

| Phase | Plans | Total | Avg/Plan |
|-------|-------|-------|----------|
| 01-local-foundation | 4/4 | 8 min | 2 min |
| 02-printer-simulation | 3/3 | 20 min | 7 min |
| 03-local-dashboard | 4/4 | 7 min | 2 min |

**Recent Trend:**
- 03-01: 1 min (TypeScript types, usePrinters hook)
- 03-02: 1 min (Globe component with react-globe.gl)
- 03-03: 2 min (TanStack Table with columns and status badges)
- 03-04: 3 min (Detail panel, job queue, Dashboard orchestration)
- Trend: Clean execution, phase complete

*Updated after each plan completion*

## Accumulated Context

### Decisions

Decisions are logged in PROJECT.md Key Decisions table.
Recent decisions affecting current work:

| Phase | Plan | Decision | Rationale |
|-------|------|----------|-----------|
| 01 | 01 | envDir: '.' instead of '../' | Standalone project structure, not nested like javascript-web reference |
| 01 | 02 | Two-stage Ditto init pattern | await init() loads WASM, then new Ditto() creates instance |
| 01 | 03 | Subscription + observer pairing | Both required for reactive sync pattern |
| 01 | 04 | Nginx Alpine | Minimal production image (~20MB) |
| 02 | 01 | std::variant for state machine | Type-safe states without inheritance |
| 02 | 01 | Location struct in PrinterState.h | Carries lat/lon/name for globe visualization |
| 02 | 02 | OnlinePlayground identity | Matches dashboard auth pattern |
| 02 | 02 | 1Hz tick rate | Reasonable telemetry frequency |
| 02 | 03 | Ubuntu 22.04 base | Ditto SDK requires glibc (not Alpine musl) |
| 02 | 03 | platform=linux/amd64 | SDK only available for x86_64 |
| 02 | 03 | nlohmann::json args for DQL | Proper argument binding for INSERT DOCUMENTS |
| 03 | 01 | Types match C++ schema exactly | printer_id, timestamp from getTelemetry() |
| 03 | 01 | Separate usePrinters hook | Printer-specific, useDitto was test scaffolding |
| 03 | 01 | Pure aggregation function | No side effects, testable |
| 03 | 02 | earth-night.jpg texture | Command center aesthetic |
| 03 | 02 | Raw DOM for markers | react-globe.gl requires HTMLElement, not React |
| 03 | 03 | TanStack Table headless | Flexible column definitions with custom cells |
| 03 | 03 | Material alert via text color | Amber <20%, red <10% - visual only, no toasts |
| 03 | 04 | Selected printer syncs with live data | useEffect watches printers array to update selection |
| 03 | 04 | Panel auto-closes on printer disappear | Handle deleted/offline printers gracefully |
| 03 | 04 | JobQueue future-proofed | Accepts array for future job_queue collection |

### Pending Todos

None.

### Blockers/Concerns

- Auth warning in simulator logs (playground auth endpoint) - cosmetic, not blocking sync

## Session Continuity

Last session: 2026-02-06T04:51:00Z
Stopped at: Completed 03-04-PLAN.md (Detail Panel & Job Queue) - Phase 3 complete
Resume file: None

## Next Steps

Phase 3: Local Dashboard - COMPLETE
- [x] 03-01: Data layer & TypeScript types
- [x] 03-02: Globe visualization with react-globe.gl
- [x] 03-03: Printer table with TanStack Table
- [x] 03-04: Detail panel, job queue, Dashboard orchestration

Ready for Phase 4: Cloud Hub (if defined)

## Deployed Components

| Service | Status | Description |
|---------|--------|-------------|
| xcell-dashboard | Running | React dashboard at localhost:8080 |
| printer-sim-1 | Running | Ramstein Air Base, Germany (FOB-ALPHA) |
| printer-sim-2 | Running | Camp Humphreys, South Korea (FOB-BRAVO) |
| printer-sim-3 | Running | Al Udeid Air Base, Qatar (FOB-CHARLIE) |
