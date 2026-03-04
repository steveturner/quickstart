---
phase: 1
slug: foundation
status: draft
nyquist_compliant: false
wave_0_complete: false
created: 2026-03-03
---

# Phase 1 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | JUnit 4 + Espresso (instrumented) |
| **Config file** | None — Gradle test runner conventions |
| **Quick run command** | `./gradlew test` |
| **Full suite command** | `./gradlew test connectedAndroidTest` |
| **Estimated runtime** | ~15 seconds (unit), ~60 seconds (instrumented) |

---

## Sampling Rate

- **After every task commit:** Run `./gradlew assembleDebug`
- **After every plan wave:** Run `./gradlew test`
- **Before `/gsd:verify-work`:** `./gradlew assembleDebug` green + manual smoke test on device
- **Max feedback latency:** 15 seconds

---

## Per-Task Verification Map

| Task ID | Plan | Wave | Requirement | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|-----------|-------------------|-------------|--------|
| 01-01-01 | 01 | 1 | FOUN-04 | build | `./gradlew assembleDebug` | Wave 0 | ⬜ pending |
| 01-01-02 | 01 | 1 | FOUN-01 | instrumented | `./gradlew connectedAndroidTest` | Wave 0 | ⬜ pending |
| 01-01-03 | 01 | 1 | FOUN-02 | unit | `./gradlew test` | Wave 0 | ⬜ pending |
| 01-01-04 | 01 | 1 | FOUN-03 | unit (koinTest) | `./gradlew test` | Wave 0 | ⬜ pending |
| 01-01-05 | 01 | 1 | FOUN-05 | unit | `./gradlew test` | Wave 0 | ⬜ pending |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

---

## Wave 0 Requirements

- [ ] `app/src/test/java/live/ditto/pubsec/pos/KoinModuleTest.kt` — stubs for FOUN-03 (Koin module verification via checkModules())
- [ ] `app/src/test/java/live/ditto/pubsec/pos/FirestoreSettingsTest.kt` — stubs for FOUN-02 (persistence disabled check)
- [ ] `app/src/test/java/live/ditto/pubsec/pos/ExampleUnitTest.kt` — placeholder covering FOUN-04 build compilation

*Existing infrastructure: none — new project.*

---

## Manual-Only Verifications

| Behavior | Requirement | Why Manual | Test Instructions |
|----------|-------------|------------|-------------------|
| App launches without crash on physical device | FOUN-01 | Requires real Android device/emulator | 1. Install APK 2. Launch app 3. Verify no crash in first 10 seconds |
| Ditto peer count readable | FOUN-01 | P2P mesh requires real device | 1. Check Logcat for Ditto init success 2. Verify presence observer reports peer count (even 0) |
| Firebase connects to project | FOUN-02 | Requires live Firebase project | 1. Check Logcat for Firestore init 2. Verify Firebase console shows connected client |

---

## Validation Sign-Off

- [ ] All tasks have `<automated>` verify or Wave 0 dependencies
- [ ] Sampling continuity: no 3 consecutive tasks without automated verify
- [ ] Wave 0 covers all MISSING references
- [ ] No watch-mode flags
- [ ] Feedback latency < 15s
- [ ] `nyquist_compliant: true` set in frontmatter

**Approval:** pending
