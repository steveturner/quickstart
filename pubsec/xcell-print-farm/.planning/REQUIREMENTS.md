# Requirements: xCell Print Farm Monitor

**Defined:** 2026-02-04
**Core Value:** Commanders see real-time manufacturing capacity across distributed xCell units even when connectivity is degraded

## v1 Requirements

Requirements for initial release. Each maps to roadmap phases.

### Printer Monitoring

- [ ] **MNTR-01**: Dashboard shows real-time printer status (idle, printing, error, offline)
- [ ] **MNTR-02**: Dashboard shows print job progress (% complete, time remaining)
- [ ] **MNTR-03**: Dashboard shows temperature readings (bed, chamber, extruder)
- [ ] **MNTR-04**: Dashboard shows print history with success/failure tracking
- [ ] **MNTR-05**: Dashboard shows air quality and environmental readings

### Job Management

- [ ] **JOB-01**: Dashboard shows print queue per printer
- [ ] **JOB-02**: Dashboard shows job status (queued, printing, complete, failed)
- [ ] **JOB-03**: Dashboard shows Starcraft-style production queue ("3 Tempests building, first ready in 12 min")

### Material Tracking

- [ ] **MAT-01**: Dashboard shows current material/powder levels per printer
- [ ] **MAT-02**: Dashboard shows material consumption rates and depletion predictions
- [ ] **MAT-03**: Dashboard shows low material alerts

### Fleet Management

- [ ] **FLEET-01**: Dashboard shows all xCell printers in unified view
- [ ] **FLEET-02**: Dashboard shows aggregate fleet status and health overview
- [ ] **FLEET-03**: Dashboard supports multi-site grouping (FOB vs theater level)
- [ ] **FLEET-04**: Dashboard shows fleet analytics and trends

### Production Lifecycle

- [ ] **LIFE-01**: Dashboard tracks finished goods status
- [ ] **LIFE-02**: Dashboard tracks full lifecycle (printing → QA → staged → assigned)
- [ ] **LIFE-03**: Dashboard shows production metrics (units completed per period)

### DDIL Resilience

- [ ] **DDIL-01**: System works fully offline with local data persistence
- [ ] **DDIL-02**: System supports P2P mesh sync between nearby xCells via Ditto
- [ ] **DDIL-03**: System auto-reconnects and syncs when connectivity returns
- [ ] **DDIL-04**: System supports theater-wide sync via Ditto cloud bridge

### Simulation

- [ ] **SIM-01**: C++ printer simulator generates realistic telemetry data
- [ ] **SIM-02**: Simulator models print jobs with progress over time
- [ ] **SIM-03**: Simulator models material consumption
- [ ] **SIM-04**: Simulator models occasional errors and failures
- [ ] **SIM-05**: Simulated webcam placeholder shows printer state

### Infrastructure

- [ ] **INFRA-01**: Full system runs via docker-compose
- [ ] **INFRA-02**: Multiple printer simulators run as separate containers
- [ ] **INFRA-03**: Web dashboard runs as container
- [ ] **INFRA-04**: Ditto sync backend runs as container

## v2 Requirements

Deferred to future release. Tracked but not in current roadmap.

### Advanced Monitoring

- **ADV-01**: Predictive maintenance alerts based on telemetry patterns
- **ADV-02**: AI-based failure detection from webcam feeds
- **ADV-03**: Live video streaming from printers

### Advanced Job Management

- **ADV-04**: Smart auto-queue routing across printers
- **ADV-05**: Priority-based job scheduling
- **ADV-06**: Remote job submission

### TAK Integration

- **TAK-01**: CoT messages for xCell positions on TAK map
- **TAK-02**: Status overlay in ATAK/WinTAK
- **TAK-03**: Deep TAK integration for queue management

## Out of Scope

Explicitly excluded. Documented to prevent scope creep.

| Feature | Reason |
|---------|--------|
| Real xCell hardware integration | Demo uses simulated data only |
| User authentication | Demo assumes trusted environment |
| Production hardening | Architecture demonstration, not production system |
| Deep TAK integration | Notional only for this demo |
| Mobile native app | Web-first, responsive design sufficient |
| Real webcam streaming | Simulated placeholder sufficient for demo |
| Multi-tenant support | Single organization demo |

## Traceability

Which phases cover which requirements. Updated during roadmap creation.

| Requirement | Phase | Status |
|-------------|-------|--------|
| INFRA-01 | Phase 1 | Pending |
| INFRA-03 | Phase 1 | Pending |
| INFRA-04 | Phase 1 | Pending |
| DDIL-01 | Phase 1 | Pending |
| SIM-01 | Phase 2 | Pending |
| SIM-02 | Phase 2 | Pending |
| SIM-03 | Phase 2 | Pending |
| SIM-04 | Phase 2 | Pending |
| SIM-05 | Phase 2 | Pending |
| INFRA-02 | Phase 2 | Pending |
| MNTR-01 | Phase 3 | Pending |
| MNTR-02 | Phase 3 | Pending |
| MNTR-03 | Phase 3 | Pending |
| MNTR-04 | Phase 3 | Pending |
| MNTR-05 | Phase 3 | Pending |
| JOB-01 | Phase 3 | Pending |
| JOB-02 | Phase 3 | Pending |
| JOB-03 | Phase 3 | Pending |
| MAT-01 | Phase 3 | Pending |
| MAT-02 | Phase 3 | Pending |
| MAT-03 | Phase 3 | Pending |
| LIFE-01 | Phase 3 | Pending |
| LIFE-02 | Phase 3 | Pending |
| LIFE-03 | Phase 3 | Pending |
| DDIL-02 | Phase 4 | Pending |
| DDIL-03 | Phase 4 | Pending |
| FLEET-01 | Phase 4 | Pending |
| FLEET-03 | Phase 4 | Pending |
| DDIL-04 | Phase 5 | Pending |
| FLEET-02 | Phase 6 | Pending |
| FLEET-04 | Phase 6 | Pending |

**Coverage:**
- v1 requirements: 24 total
- Mapped to phases: 24
- Unmapped: 0

---
*Requirements defined: 2026-02-04*
*Last updated: 2026-02-05 after roadmap creation*
