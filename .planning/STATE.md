# Project State

## Project Reference

See: .planning/PROJECT.md (updated 2026-03-03)

**Core value:** POS terminals continue processing transactions and stay in sync via Ditto P2P mesh even when Firebase/internet is unavailable
**Current focus:** Phase 1 — Foundation

## Current Position

Phase: 1 of 4 (Foundation)
Plan: 0 of TBD in current phase
Status: Ready to plan
Last activity: 2026-03-03 — Roadmap created

Progress: [░░░░░░░░░░] 0%

## Performance Metrics

**Velocity:**
- Total plans completed: 0
- Average duration: —
- Total execution time: —

**By Phase:**

| Phase | Plans | Total | Avg/Plan |
|-------|-------|-------|----------|
| - | - | - | - |

**Recent Trend:**
- Last 5 plans: —
- Trend: —

*Updated after each plan completion*

## Accumulated Context

### Decisions

Decisions are logged in PROJECT.md Key Decisions table.
Recent decisions affecting current work:

- [Init]: Firebase as primary cloud source of truth; Ditto as edge extension (not standalone)
- [Init]: UI reads exclusively from Ditto local store — never from Firestore directly
- [Research]: Inventory uses delta events (not LWW integers) to avoid CRDT data loss under concurrent offline edits
- [Research]: Single canonical UUID used as both Ditto _id and Firestore document ID — frozen before any write code

### Pending Todos

None yet.

### Blockers/Concerns

- [Phase 3]: Ditto DQL upsert syntax (`INSERT INTO ... ON ID CONFLICT DO UPDATE SET`) needs live validation against Ditto 4.14.3 docs during planning
- [Phase 3]: Firestore has no native connection state callback — Firebase Realtime Database `.info/connected` workaround needed; verify against SDK 34.10.0
- [Phase 3]: ChangeGuard coroutine dispatcher model needs validation — `runBlocking` may cause ANR from wrong dispatcher

## Session Continuity

Last session: 2026-03-03
Stopped at: Roadmap created, STATE.md initialized
Resume file: None
