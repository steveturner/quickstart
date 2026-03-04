---
phase: 3
slug: sync-bridge-and-pos-ui
status: draft
nyquist_compliant: false
wave_0_complete: false
created: 2026-03-04
---

# Phase 3 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | JUnit 4 + Kotlin Test + mockk (via Gradle) |
| **Config file** | pubsec/android-firebase-pos/app/build.gradle.kts |
| **Quick run command** | `cd pubsec/android-firebase-pos && ./gradlew test` |
| **Full suite command** | `cd pubsec/android-firebase-pos && ./gradlew test` |
| **Estimated runtime** | ~20 seconds |

---

## Sampling Rate

- **After every task commit:** Run `cd pubsec/android-firebase-pos && ./gradlew test`
- **After every plan wave:** Run `cd pubsec/android-firebase-pos && ./gradlew test`
- **Before `/gsd:verify-work`:** Full suite must be green
- **Max feedback latency:** 20 seconds

---

## Per-Task Verification Map

*To be populated by planner after plans are created.*

---

## Manual-Only Verifications

| Behavior | Requirement | Why Manual | Test Instructions |
|----------|-------------|------------|-------------------|
| Products added in Firebase console appear in POS catalog | SYNC-01 | Requires Firebase console + running device | Add product in Firebase console, verify it appears in app catalog |
| Order created on device appears in Firestore | SYNC-02 | Requires device + Firebase console | Create order in app, check Firestore orders collection |
| No infinite write cascade | SYNC-03 | Requires running bridge on device | Monitor Logcat for repeated writes after single doc change |
| Offline POS operations work | SYNC-04 | Requires device with airplane mode | Disable Firebase, create order, verify inventory decrements |
| Offline orders sync after reconnect | SYNC-05 | Requires device reconnection | Create orders offline, restore connectivity, check Firestore |
| P2P sync between two devices | SYNC-05 | Requires two physical devices | Order on device A appears on device B via Ditto mesh |

---

## Validation Sign-Off

- [ ] All tasks have `<automated>` verify or Wave 0 dependencies
- [ ] Sampling continuity: no 3 consecutive tasks without automated verify
- [ ] Wave 0 covers all MISSING references
- [ ] No watch-mode flags
- [ ] Feedback latency < 20s
- [ ] `nyquist_compliant: true` set in frontmatter

**Approval:** pending
