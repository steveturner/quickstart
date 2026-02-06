# Project State

## Project Reference

See: .planning/PROJECT.md (updated 2026-02-04)

**Core value:** Commanders see real-time manufacturing capacity across distributed xCell units even when connectivity is degraded
**Current focus:** Phase 3 - Local Dashboard (In Progress)

## Current Position

Phase: 3 of 6 (Local Dashboard) - IN PROGRESS
Plan: 2 of 4 complete
Status: In progress
Last activity: 2026-02-06 - Completed 03-02-PLAN.md (Globe Visualization)

Progress: [█████░░░░░] 9/11 plans (~82%)

## Performance Metrics

**Velocity:**
- Total plans completed: 8
- Average duration: 4 min
- Total execution time: 0.49 hours

**By Phase:**

| Phase | Plans | Total | Avg/Plan |
|-------|-------|-------|----------|
| 01-local-foundation | 4/4 | 8 min | 2 min |
| 02-printer-simulation | 3/3 | 20 min | 7 min |
| 03-local-dashboard | 2/4 | 2 min | 1 min |

**Recent Trend:**
- 03-01: 1 min (TypeScript types, usePrinters hook)
- 03-02: 1 min (Globe component with react-globe.gl)
- Trend: Clean execution, established patterns

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

### Pending Todos

None.

### Blockers/Concerns

- Auth warning in simulator logs (playground auth endpoint) - cosmetic, not blocking sync

## Session Continuity

Last session: 2026-02-06T04:41:48Z
Stopped at: Completed 03-02-PLAN.md (Globe Visualization)
Resume file: None

## Next Steps

Continue Phase 3: Local Dashboard
- [x] 03-01: Data layer & TypeScript types
- [x] 03-02: Globe visualization with react-globe.gl
- [ ] 03-03: Printer table with TanStack Table
- [ ] 03-04: Detail panel, job queue, Dashboard orchestration

## Deployed Components

| Service | Status | Description |
|---------|--------|-------------|
| xcell-dashboard | Running | React dashboard at localhost:8080 |
| printer-sim-1 | Running | Ramstein Air Base, Germany (FOB-ALPHA) |
| printer-sim-2 | Running | Camp Humphreys, South Korea (FOB-BRAVO) |
| printer-sim-3 | Running | Al Udeid Air Base, Qatar (FOB-CHARLIE) |
