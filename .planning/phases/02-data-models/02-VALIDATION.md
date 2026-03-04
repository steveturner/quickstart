---
phase: 2
slug: data-models
status: draft
nyquist_compliant: false
wave_0_complete: false
created: 2026-03-04
---

# Phase 2 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | JUnit 5 + Kotlin Test (via Gradle) |
| **Config file** | pubsec/android-firebase-pos/app/build.gradle.kts |
| **Quick run command** | `cd pubsec/android-firebase-pos && ./gradlew test` |
| **Full suite command** | `cd pubsec/android-firebase-pos && ./gradlew test` |
| **Estimated runtime** | ~15 seconds |

---

## Sampling Rate

- **After every task commit:** Run `cd pubsec/android-firebase-pos && ./gradlew test`
- **After every plan wave:** Run `cd pubsec/android-firebase-pos && ./gradlew test`
- **Before `/gsd:verify-work`:** Full suite must be green
- **Max feedback latency:** 15 seconds

---

## Per-Task Verification Map

| Task ID | Plan | Wave | Requirement | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|-----------|-------------------|-------------|--------|
| 02-01-01 | 01 | 1 | DATA-01 | unit | `./gradlew test --tests "*ProductTest*"` | ❌ W0 | ⬜ pending |
| 02-01-02 | 01 | 1 | DATA-02 | unit | `./gradlew test --tests "*OrderTest*"` | ❌ W0 | ⬜ pending |
| 02-01-03 | 01 | 1 | DATA-03 | unit | `./gradlew test --tests "*InventoryTest*"` | ❌ W0 | ⬜ pending |
| 02-01-04 | 01 | 1 | DATA-04 | unit | `./gradlew test --tests "*SoftDeleteTest*"` | ❌ W0 | ⬜ pending |
| 02-01-05 | 01 | 1 | DATA-05 | unit | `./gradlew test --tests "*DeltaEventTest*"` | ❌ W0 | ⬜ pending |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

---

## Wave 0 Requirements

- [ ] `app/src/test/java/live/ditto/pubsec/pos/model/ProductTest.kt` — data class field validation
- [ ] `app/src/test/java/live/ditto/pubsec/pos/model/OrderTest.kt` — order structure + line items
- [ ] `app/src/test/java/live/ditto/pubsec/pos/model/InventoryItemTest.kt` — inventory + delta pattern
- [ ] `app/src/test/java/live/ditto/pubsec/pos/data/SeedDataTest.kt` — seed data completeness + syncSource tagging

*Existing JUnit infrastructure from Phase 1 covers test runner setup.*

---

## Manual-Only Verifications

| Behavior | Requirement | Why Manual | Test Instructions |
|----------|-------------|------------|-------------------|
| Firestore seeded data visible in console | DATA-02 | Requires Firebase project + console access | Run app, open Firebase console, check products/orders/inventory collections |
| Ditto COUNTER init with seed value | DATA-05 | Requires Ditto SDK runtime (JNI) | Run on device, verify inventory quantity via Ditto portal or debug log |

---

## Validation Sign-Off

- [ ] All tasks have `<automated>` verify or Wave 0 dependencies
- [ ] Sampling continuity: no 3 consecutive tasks without automated verify
- [ ] Wave 0 covers all MISSING references
- [ ] No watch-mode flags
- [ ] Feedback latency < 15s
- [ ] `nyquist_compliant: true` set in frontmatter

**Approval:** pending
