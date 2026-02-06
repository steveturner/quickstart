# Project State

## Project Reference

See: .planning/PROJECT.md (updated 2026-02-04)

**Core value:** Commanders see real-time manufacturing capacity across distributed xCell units even when connectivity is degraded
**Current focus:** Phase 4 - Multi-Device Mesh (Plan 3 of ? complete)

## Current Position

Phase: 4 of 6 (Multi-Device Mesh)
Plan: 3 of ? complete
Status: In progress
Last activity: 2026-02-06 - Completed 04-03-PLAN.md (Mesh UI Components)

Progress: [████████░░] 14 plans complete

## Performance Metrics

**Velocity:**
- Total plans completed: 14
- Average duration: 3 min
- Total execution time: 0.67 hours

**By Phase:**

| Phase | Plans | Total | Avg/Plan |
|-------|-------|-------|----------|
| 01-local-foundation | 4/4 | 8 min | 2 min |
| 02-printer-simulation | 3/3 | 20 min | 7 min |
| 03-local-dashboard | 4/4 | 7 min | 2 min |
| 04-multi-device-mesh | 3/? | 5 min | 2 min |

**Recent Trend:**
- 03-04: 3 min (Detail panel, job queue, Dashboard orchestration)
- 04-01: 1 min (Presence types and usePresence hook)
- 04-02: 3 min (Site filtering in usePrinters hook)
- 04-03: 1 min (MeshIndicator and SiteSelector components)
- Trend: Clean execution continues

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
| 04 | 01 | Typed observer as Observer (SDK type) | Use stop() not cancel() for presence observer |
| 04 | 01 | Extract connectionTypes via Set | Unique values from peer connections |
| 04 | 02 | Backward compatible siteCode parameter | No options = theater-wide view (all printers) |
| 04 | 02 | DQL WHERE with :site parameter | Efficient site filtering via location.site_code |
| 04 | 02 | Separate useEffect for site changes | Avoid Ditto reinit on filter change |
| 04 | 03 | Green/yellow status dot colors | Standard connected/connecting visual pattern |
| 04 | 03 | Null = All Sites theater-wide view | SiteSelector maps dropdown to filter value |
| 04 | 03 | Via cloud annotation for WebSocket | Expected for web browsers connecting via relay |

### Pending Todos

None.

### Blockers/Concerns

- Auth warning in simulator logs (playground auth endpoint) - cosmetic, not blocking sync

## Session Continuity

Last session: 2026-02-06T05:37:47Z
Stopped at: Completed 04-03-PLAN.md (Mesh UI Components)
Resume file: None

## Next Steps

Phase 4: Multi-Device Mesh - IN PROGRESS
- [x] 04-01: Presence types and usePresence hook
- [x] 04-02: Site filtering in usePrinters hook
- [x] 04-03: MeshIndicator and SiteSelector components
- [ ] 04-04+: Dashboard integration with mesh components (if defined)

## Deployed Components

| Service | Status | Description |
|---------|--------|-------------|
| xcell-dashboard | Running | React dashboard at localhost:8080 |
| printer-sim-1 | Running | Ramstein Air Base, Germany (FOB-ALPHA) |
| printer-sim-2 | Running | Camp Humphreys, South Korea (FOB-BRAVO) |
| printer-sim-3 | Running | Al Udeid Air Base, Qatar (FOB-CHARLIE) |
