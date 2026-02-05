# Architecture Patterns: xCell Print Farm Monitor

**Domain:** DDIL-resilient 3D printer farm monitoring with offline-first sync
**Researched:** 2026-02-05
**Overall confidence:** HIGH

## Recommended Architecture

This is a distributed monitoring system built around Ditto's offline-first mesh sync capabilities, designed for DDIL (Denied, Degraded, Intermittent, Limited) environments typical of expeditionary military operations.

```
┌─────────────────────────────────────────────────────────────────┐
│                         Cloud Layer                              │
│  ┌──────────────────┐         ┌─────────────────────────────┐  │
│  │  Ditto Big Peer  │◄────────┤   Web Dashboard (Theater)   │  │
│  │  (Cloud Datastore)│         │   - Fleet aggregate view    │  │
│  └────────┬──────────┘         │   - Cross-FOB visibility    │  │
│           │                    └─────────────────────────────┘  │
└───────────┼──────────────────────────────────────────────────────┘
            │ WebSocket Sync (when connectivity allows)
            │
┌───────────┼──────────────────────────────────────────────────────┐
│           │          Edge/Mesh Layer                             │
│  ┌────────▼──────────┐        ┌─────────────────────────────┐  │
│  │  Ditto Small Peer │◄───────┤  Web Dashboard (Local FOB)  │  │
│  │  (xCell Unit A)   │ BLE/   │  - Local mesh view          │  │
│  │                   │ LAN    │  - Real-time updates        │  │
│  └────────┬──────────┘        └─────────────────────────────┘  │
│           │                                                      │
│           │ P2P Mesh (BLE, LAN, WiFi Direct)                   │
│           │                                                      │
│  ┌────────▼──────────┐        ┌─────────────────────────────┐  │
│  │  Ditto Small Peer │◄───────┤  Web Dashboard (Local FOB)  │  │
│  │  (xCell Unit B)   │        │  - Local mesh view          │  │
│  │                   │        │  - Offline-capable          │  │
│  └────────┬──────────┘        └─────────────────────────────┘  │
│           │                                                      │
│  ┌────────▼──────────┐                                          │
│  │  Ditto Small Peer │                                          │
│  │  (xCell Unit C)   │                                          │
│  └────────┬──────────┘                                          │
└───────────┼──────────────────────────────────────────────────────┘
            │
┌───────────┼──────────────────────────────────────────────────────┐
│           │          Simulation Layer                            │
│  ┌────────▼──────────┐                                           │
│  │  Printer Simulator│                                           │
│  │  - Generates data │                                           │
│  │  - Models MJF API │                                           │
│  │  - Write-only     │                                           │
│  └───────────────────┘                                           │
└──────────────────────────────────────────────────────────────────┘
```

## Component Boundaries

### 1. Ditto Core Layer

**Responsibility:** Data synchronization, conflict resolution, mesh networking
**Communicates with:** All other components
**Technology:** Ditto SDK (JavaScript for web, potential Go/Rust for simulators)

**Key capabilities:**
- Local-first database on each node (xCell unit, dashboard instance)
- P2P mesh sync via BLE, LAN, WiFi Direct
- Cloud sync when connectivity available
- CRDT-based automatic conflict resolution
- Presence graph for intelligent routing

**Configuration:**
```javascript
// From existing quickstart patterns
const ditto = new Ditto({
  type: 'onlinePlayground',
  appID: process.env.DITTO_APP_ID,
  token: process.env.DITTO_PLAYGROUND_TOKEN,
  customAuthURL: process.env.DITTO_AUTH_URL,
});

// Enable all transport types for DDIL resilience
ditto.updateTransportConfig((config) => {
  config.peerToPeer.lan.enabled = true;
  config.peerToPeer.bluetoothLe.enabled = true;
  config.peerToPeer.wifiAware.enabled = true;
  config.connect.websocketURLs = [process.env.DITTO_WEBSOCKET_URL];
  return config;
});
```

### 2. Printer Simulator

**Responsibility:** Generate realistic 3D printer telemetry
**Communicates with:** Ditto Core (write-only)
**Technology:** Node.js/TypeScript or Go

**Data generation:**
- Printer status (idle, printing, paused, error)
- Temperature readings (build chamber, material bed)
- Print queue with estimated completion times
- Material levels (powder consumption rates)
- Error conditions (material shortage, thermal issues)
- Utilization metrics

**Write pattern:**
```javascript
// Simulator writes to local Ditto store
await ditto.store.execute(
  'INSERT INTO printer_status DOCUMENTS (:status)',
  {
    status: {
      printerId: 'xcell-a-printer-1',
      xcellId: 'xcell-unit-alpha',
      status: 'printing',
      currentJob: jobId,
      temperature: { chamber: 185, bed: 80 },
      materialLevel: 0.73,
      timestamp: new Date().toISOString()
    }
  }
);
```

### 3. Web Dashboard (Local)

**Responsibility:** Real-time visualization of local mesh xCell units
**Communicates with:** Ditto Core (read/write), Cloud Dashboard (implicit via Ditto sync)
**Technology:** React + TypeScript + Vite + TailwindCSS

**UI Components:**
- Fleet status cards (per xCell unit)
- Live print queue visualization
- Material consumption gauges
- Health alerts
- Production timeline

**Read pattern (observer + subscription):**
```javascript
// Subscribe to sync relevant data
const subscription = ditto.sync.registerSubscription(
  'SELECT * FROM printer_status WHERE xcellId IN (:localUnits)',
  { localUnits: ['xcell-unit-alpha', 'xcell-unit-bravo'] }
);

// Observe changes for real-time UI updates
const observer = ditto.store.registerObserver(
  'SELECT * FROM printer_status WHERE xcellId IN (:localUnits) ORDER BY timestamp DESC',
  { localUnits: ['xcell-unit-alpha', 'xcell-unit-bravo'] },
  (results) => {
    setPrinterStatus(results.items.map(item => item.value));
  }
);
```

### 4. Web Dashboard (Theater/Cloud)

**Responsibility:** Fleet-wide aggregate view across all distributed xCells
**Communicates with:** Ditto Core (read-only), Ditto Big Peer
**Technology:** React + TypeScript + Vite + TailwindCSS

**UI Components:**
- Theater-wide map view
- Cross-FOB production metrics
- Fleet capacity aggregates
- Material inventory totals
- Anomaly detection/alerts

**Query pattern (fleet-wide):**
```javascript
// Subscribe to all xCell units
const subscription = ditto.sync.registerSubscription(
  'SELECT * FROM printer_status'
);

// Aggregate queries for fleet metrics
const fleetMetrics = await ditto.store.execute(
  'SELECT xcellId, COUNT(*) as printerCount, SUM(materialLevel) as totalMaterial ' +
  'FROM printer_status GROUP BY xcellId'
);
```

### 5. Ditto Big Peer (Cloud)

**Responsibility:** Central cloud datastore and sync coordinator
**Communicates with:** All Ditto Small Peers (when connectivity allows)
**Technology:** Ditto Cloud (managed service)

**Capabilities:**
- Persistent storage of all mesh data
- WebSocket server for cloud sync
- Authentication/authorization endpoint
- Historical data retention
- Cross-theater sync hub

### 6. xCell Unit Node

**Responsibility:** Edge computing node embedded in physical xCell container
**Communicates with:** Printer Simulator, Ditto Core, nearby xCell nodes
**Technology:** Raspberry Pi or similar edge hardware

**Services:**
- Ditto Small Peer instance
- Printer Simulator (if using simulated data)
- Local BLE/WiFi mesh networking
- Optional: Local dashboard instance

## Data Flow Patterns

### Pattern 1: Simulator → Local State → Mesh

**Sequence:**

1. **Simulator generates data** (every 10-30 seconds per printer)
   - Printer status, temperature, material level
   - Write to local Ditto store via `INSERT`/`UPDATE`

2. **Local Ditto store persists** (instant, local-first)
   - Data available immediately for local queries
   - No network required

3. **Ditto mesh propagation** (automatic, multi-hop)
   - Subscription matching triggers sync to nearby peers
   - Flood-fill pattern through presence graph
   - Data reaches all connected xCell units in mesh

**Code flow:**
```javascript
// Simulator writes
simulator.generateTelemetry()
  → ditto.store.execute('INSERT ...')
  → Local DB write

// Mesh sync (automatic)
Local DB write
  → Ditto detects change
  → Presence graph routing
  → Multi-hop to matching subscriptions
  → Remote peer DB writes
```

**Characteristics:**
- Write-heavy at edge (simulator → Ditto)
- Read-light at edge (local dashboard queries)
- Automatic mesh propagation
- Millisecond local latency, second mesh latency

### Pattern 2: Mesh → Cloud (Opportunistic)

**Sequence:**

1. **WebSocket connection available** (intermittent)
   - xCell unit establishes WebSocket to Ditto Big Peer
   - May be denied/degraded/limited

2. **Ditto syncs delta** (efficient, compressed)
   - Only changes since last sync
   - CRDT merge at cloud
   - Bidirectional (cloud changes sync down)

3. **Cloud persists** (historical record)
   - Long-term storage
   - Cross-theater visibility

**Characteristics:**
- Tolerates connection loss mid-sync
- Resumes automatically when reconnected
- No data loss during disconnection
- Batched, efficient delta sync

### Pattern 3: Dashboard Real-Time Updates

**Sequence:**

1. **Dashboard registers observer** (on mount)
   - DQL query with parameters
   - Callback fires on local DB changes

2. **Subscription ensures data availability**
   - Declarative: "I want data matching this query"
   - Ditto ensures mesh sync of matching documents

3. **Observer callback fires** (instant, local)
   - Query re-evaluated on DB change
   - React state updated
   - UI re-renders

**Code flow:**
```javascript
useEffect(() => {
  // Subscription: "What data do I want?"
  const subscription = ditto.sync.registerSubscription(
    'SELECT * FROM printer_status WHERE xcellId = :id',
    { id: selectedXcell }
  );

  // Observer: "Tell me when it changes"
  const observer = ditto.store.registerObserver(
    'SELECT * FROM printer_status WHERE xcellId = :id ORDER BY timestamp DESC',
    { id: selectedXcell },
    (results) => {
      // This fires on every local DB change
      setPrinterData(results.items.map(i => i.value));
    }
  );

  return () => {
    subscription.cancel();
    observer.cancel();
  };
}, [selectedXcell]);
```

**Characteristics:**
- Local-first: reads always succeed
- Instant feedback: no network round-trip
- Reactive: UI updates automatically
- Scoped: subscriptions filter mesh traffic

## Architecture Patterns to Follow

### Pattern 1: Local-First, Sync-Later

**What:** All operations target local Ditto store first, mesh sync happens asynchronously

**When:** Every data write in the system

**Why:**
- Zero perceived latency for users
- Offline operation guaranteed
- Network failures don't block operations
- Ditto's CRDTs handle eventual consistency

**Example:**
```javascript
// Good: Write to local store immediately
const createPrintJob = async (jobData) => {
  await ditto.store.execute(
    'INSERT INTO print_jobs DOCUMENTS (:job)',
    { job: { ...jobData, status: 'queued' } }
  );
  // Sync happens automatically in background
  // UI updates via observer callback
};

// Bad: Wait for network confirmation
const createPrintJobBad = async (jobData) => {
  await fetch('/api/jobs', { method: 'POST', body: jobData });
  // Fails when offline, blocks user, slow even online
};
```

### Pattern 2: Subscription + Observer Pairing

**What:** Always pair a subscription (sync) with an observer (local query)

**When:** Any component that displays Ditto data

**Why:**
- Subscription ensures data syncs to device
- Observer provides reactive local updates
- Decouples sync from display logic

**Example:**
```javascript
// Good: Paired subscription and observer
const usePrinterStatus = (xcellId) => {
  const [status, setStatus] = useState(null);

  useEffect(() => {
    // Subscription: declare sync requirements
    const sub = ditto.sync.registerSubscription(
      'SELECT * FROM printer_status WHERE xcellId = :id',
      { id: xcellId }
    );

    // Observer: react to local changes
    const obs = ditto.store.registerObserver(
      'SELECT * FROM printer_status WHERE xcellId = :id',
      { id: xcellId },
      (results) => setStatus(results.items[0]?.value)
    );

    return () => { sub.cancel(); obs.cancel(); };
  }, [xcellId]);

  return status;
};

// Bad: Query without subscription
const usePrinterStatusBad = (xcellId) => {
  // Data never syncs! Observer sees only local data.
  const obs = ditto.store.registerObserver(...);
};
```

### Pattern 3: Scoped Subscriptions for Mesh Efficiency

**What:** Narrow subscriptions to only data needed by that device/view

**When:** Defining all subscription queries

**Why:**
- Reduces mesh bandwidth
- Prevents data islands (over-filtering)
- Optimizes local storage

**Example:**
```javascript
// Good: FOB-scoped subscription (local mesh)
const localSub = ditto.sync.registerSubscription(
  'SELECT * FROM printer_status WHERE xcellId IN (:localUnits)',
  { localUnits: ['xcell-alpha', 'xcell-bravo'] }
);

// Good: Theater-scoped subscription (cloud dashboard)
const theaterSub = ditto.sync.registerSubscription(
  'SELECT * FROM printer_status'  // All xCells
);

// Bad: Over-broad on edge device
const badSub = ditto.sync.registerSubscription(
  'SELECT * FROM *'  // Syncs everything, wastes bandwidth
);
```

### Pattern 4: Soft Delete for DDIL Resilience

**What:** Mark documents deleted rather than removing them

**When:** Any "delete" operation

**Why:**
- Sync operations propagate deletions
- Hard deletes can create inconsistencies in mesh
- Allows sync of deletion state

**Example:**
```javascript
// Good: Soft delete
await ditto.store.execute(
  'UPDATE print_jobs SET deleted = true, deletedAt = :now WHERE _id = :id',
  { id: jobId, now: new Date().toISOString() }
);

// Query filters deleted
const obs = ditto.store.registerObserver(
  'SELECT * FROM print_jobs WHERE deleted = false',
  (results) => setJobs(results.items.map(i => i.value))
);

// Bad: Hard delete
await ditto.store.execute('DELETE FROM print_jobs WHERE _id = :id', { id });
```

### Pattern 5: Presence Graph for Multi-Hop Awareness

**What:** Rely on Ditto's presence graph for multi-hop data flow

**When:** Designing mesh topology

**Why:**
- Ditto automatically routes through intermediate peers
- Self-healing when peers disconnect
- No manual routing logic needed

**Implementation:**
```javascript
// Enable all transports (Ditto handles routing)
ditto.updateTransportConfig((config) => {
  config.peerToPeer.lan.enabled = true;
  config.peerToPeer.bluetoothLe.enabled = true;
  config.peerToPeer.wifiAware.enabled = true;
  return config;
});

// Trust Ditto's presence graph
// If xCell-A ←→ xCell-B ←→ xCell-C (no direct A-C link)
// Data still flows A → B → C automatically
```

## Anti-Patterns to Avoid

### Anti-Pattern 1: Polling Local Database

**What:** Repeatedly querying Ditto store on interval

**Why bad:**
- Observers provide reactive updates for free
- Wastes CPU cycles
- Misses intermediate updates between polls

**Instead:**
```javascript
// Bad
setInterval(() => {
  const results = await ditto.store.execute('SELECT * FROM printer_status');
  setPrinterStatus(results.items);
}, 1000);

// Good
const observer = ditto.store.registerObserver(
  'SELECT * FROM printer_status',
  (results) => setPrinterStatus(results.items.map(i => i.value))
);
```

### Anti-Pattern 2: Server-Side State as Source of Truth

**What:** Treating cloud API as authoritative, local Ditto as cache

**Why bad:**
- Defeats offline-first architecture
- Breaks in DDIL environments
- Adds unnecessary latency

**Instead:**
```javascript
// Bad: Server-centric
const getStatus = async () => {
  try {
    const res = await fetch('/api/printer-status');
    return res.json();
  } catch (err) {
    return cachedStatus;  // Offline fallback bolted on
  }
};

// Good: Local-first
const getStatus = async () => {
  // Local query always works
  const results = await ditto.store.execute('SELECT * FROM printer_status');
  return results.items.map(i => i.value);
  // Mesh sync handles propagation automatically
};
```

### Anti-Pattern 3: Ignoring Subscription Scope

**What:** Forgetting that observers only see data matched by subscriptions

**Why bad:**
- Dashboard shows stale/incomplete data
- Confusing bug: queries work but return nothing
- Mesh bandwidth wasted on unused data

**Instead:**
```javascript
// Bad: Observer without matching subscription
const observer = ditto.store.registerObserver(
  'SELECT * FROM jobs WHERE status = "complete"',
  (results) => setCompletedJobs(results.items.map(i => i.value))
);
// No subscription! Only sees locally-created jobs.

// Good: Subscription matches observer
const subscription = ditto.sync.registerSubscription(
  'SELECT * FROM jobs WHERE status = "complete"'
);
const observer = ditto.store.registerObserver(
  'SELECT * FROM jobs WHERE status = "complete"',
  (results) => setCompletedJobs(results.items.map(i => i.value))
);
```

### Anti-Pattern 4: Tightly Coupling Components to Mesh State

**What:** Exposing Ditto sync status directly in UI logic

**Why bad:**
- Sync state is transient and noisy
- UI flickers as connections change
- Misunderstands local-first architecture

**Instead:**
```javascript
// Bad: UI depends on sync state
const Dashboard = () => {
  const [isSyncing, setIsSyncing] = useState(false);

  useEffect(() => {
    const interval = setInterval(() => {
      setIsSyncing(ditto.isSyncActive);
    }, 100);
  }, []);

  if (!isSyncing) return <div>Offline, data may be stale</div>;
  // Data is NOT stale! Local DB is always current.
};

// Good: Show connectivity as metadata, not blocker
const Dashboard = () => {
  const [meshStatus, setMeshStatus] = useState('checking');

  useEffect(() => {
    // Optional: Show connection indicator
    const interval = setInterval(() => {
      setMeshStatus(ditto.isSyncActive ? 'connected' : 'offline');
    }, 5000);  // Slow poll, not critical
  }, []);

  return (
    <div>
      <StatusBadge status={meshStatus} />  {/* Non-blocking indicator */}
      <PrinterList data={printerData} />    {/* Works regardless */}
    </div>
  );
};
```

### Anti-Pattern 5: Complex Conflict Resolution Logic

**What:** Implementing custom merge strategies for concurrent edits

**Why bad:**
- Ditto's CRDTs handle this automatically
- Adds fragile, hard-to-test code
- Last-write-wins is sufficient for monitoring data

**Instead:**
```javascript
// Bad: Manual conflict resolution
const updatePrinterStatus = async (printerId, newStatus) => {
  const current = await ditto.store.execute(
    'SELECT * FROM printer_status WHERE printerId = :id',
    { id: printerId }
  );

  if (current.version > newStatus.version) {
    // Complex merge logic...
  }
};

// Good: Let Ditto handle it
const updatePrinterStatus = async (printerId, newStatus) => {
  await ditto.store.execute(
    'UPDATE printer_status SET status = :status WHERE printerId = :id',
    { id: printerId, status: newStatus }
  );
  // CRDTs merge automatically across mesh
};
```

## Build Order and Dependencies

### Phase 1: Local Ditto Foundation
**Build first:** Core Ditto integration in minimal app
**Dependencies:** None
**Validation:** Single device writes/reads from local store

**Deliverables:**
- Ditto initialization pattern
- Basic subscription + observer
- Environment variable configuration

### Phase 2: Printer Simulator
**Build second:** Simulated telemetry generation
**Dependencies:** Phase 1 (needs Ditto write API)
**Validation:** Simulator writes appear in local Ditto store

**Deliverables:**
- Printer data model (status, temperature, materials)
- Print queue data model (jobs, progress)
- Realistic data generation loop

### Phase 3: Local Dashboard (Single Device)
**Build third:** React UI with single-device view
**Dependencies:** Phase 1 + 2 (needs data to display)
**Validation:** Dashboard shows live printer data

**Deliverables:**
- Fleet status cards
- Print queue visualization
- Material level gauges
- Observer-driven reactive updates

### Phase 4: Multi-Device Mesh
**Build fourth:** Test with 2+ devices forming mesh
**Dependencies:** Phase 1-3 (needs complete stack)
**Validation:** Data syncs between devices via BLE/LAN

**Deliverables:**
- Transport configuration (BLE + LAN enabled)
- Mesh testing protocol
- Multi-device subscription scoping

### Phase 5: Cloud Sync
**Build fifth:** Integrate Ditto Big Peer
**Dependencies:** Phase 1-4 (needs working mesh)
**Validation:** Data syncs to cloud when online

**Deliverables:**
- WebSocket configuration
- Cloud dashboard instance
- Theater-wide aggregate queries

### Phase 6: Fleet Dashboard
**Build sixth:** Theater-wide visualization
**Dependencies:** Phase 5 (needs cloud sync)
**Validation:** Single dashboard shows all xCell units

**Deliverables:**
- Fleet aggregate views
- Cross-FOB metrics
- Historical data queries

## Technology Integration Points

### Ditto SDK
- **JavaScript/TypeScript** for web dashboard (React)
- **Node.js** for printer simulator
- **Potential Go/Rust** if simulator needs embedded deployment

### Data Storage
- **Ditto local store** (embedded SQLite with sync)
- **Ditto Big Peer** (cloud PostgreSQL backend)
- No separate database required

### Transport Layer
- **BLE** for xCell-to-xCell at FOB
- **LAN** for wired/WiFi local network
- **WebSocket** for cloud connectivity
- All configured via `updateTransportConfig()`

### UI Framework
- **React + TypeScript** (consistent with javascript-web quickstart)
- **Vite** for build tooling
- **TailwindCSS** for styling
- **Ditto hooks** for observer management

## Scalability Considerations

### At 10 Printers (1-2 xCell Units)
**Approach:** Direct mesh, single dashboard instance
**Bottleneck:** None
**Pattern:** All printers in single subscription

### At 100 Printers (10-20 xCell Units)
**Approach:** FOB-scoped subscriptions, regional dashboards
**Bottleneck:** BLE mesh density
**Pattern:** Subscription filtering by `xcellId` or geographic region

**Mitigation:**
```javascript
// FOB-A dashboard subscribes only to local units
const subscription = ditto.sync.registerSubscription(
  'SELECT * FROM printer_status WHERE xcellId IN (:fobAUnits)',
  { fobAUnits: ['xcell-1', 'xcell-2', 'xcell-3'] }
);
```

### At 1000 Printers (100+ xCell Units, Theater-Wide)
**Approach:** Hierarchical mesh, cloud aggregation
**Bottleneck:** Cloud WebSocket bandwidth, dashboard query performance
**Pattern:** Cloud-side aggregation queries, regional rollups

**Mitigation:**
```javascript
// Theater dashboard queries aggregates, not raw telemetry
const fleetSummary = await ditto.store.execute(
  'SELECT xcellId, AVG(materialLevel) as avgMaterial, ' +
  'COUNT(CASE WHEN status = "printing" THEN 1 END) as activePrinters ' +
  'FROM printer_status GROUP BY xcellId'
);
```

**Additional:**
- Introduce data retention policies (archive old telemetry)
- Regional Ditto Big Peers (multiple clouds for geographic distribution)
- Sampling (not every printer update needs theater-wide sync)

## DDIL Resilience Patterns

### Disconnection Handling
**Pattern:** Optimistic UI updates, background reconciliation

```javascript
// UI always shows local state
const [printerStatus, setPrinterStatus] = useState([]);

useEffect(() => {
  const observer = ditto.store.registerObserver(
    'SELECT * FROM printer_status',
    (results) => setPrinterStatus(results.items.map(i => i.value))
  );
  // Works offline, updates when mesh reconnects
}, []);
```

### Intermittent Connectivity
**Pattern:** Automatic reconnection, delta sync

Ditto handles this transparently:
- Connection drops mid-sync → resumes when restored
- Batches changes → efficient bandwidth usage
- CRDT merge → no duplicate data

### Denied/Degraded Connectivity
**Pattern:** Fall back to local mesh, defer cloud sync

```javascript
// Subscription tries cloud first, falls back to mesh
ditto.updateTransportConfig((config) => {
  config.connect.websocketURLs = [cloudURL];  // Try cloud
  config.peerToPeer.lan.enabled = true;        // Fall back to LAN
  config.peerToPeer.bluetoothLe.enabled = true; // Fall back to BLE
  return config;
});
```

### Limited Bandwidth
**Pattern:** Prioritize critical data, sample high-frequency telemetry

```javascript
// High priority: error states (always sync)
const criticalSub = ditto.sync.registerSubscription(
  'SELECT * FROM printer_status WHERE status IN ("error", "material_low")'
);

// Low priority: routine telemetry (sample)
const telemetrySub = ditto.sync.registerSubscription(
  'SELECT * FROM printer_telemetry WHERE timestamp > :recent',
  { recent: Date.now() - 3600000 }  // Last hour only
);
```

## Sources

### Official Ditto Documentation
- [Mesh Networking 101](https://docs.ditto.live/sync/concepts/mesh-networking-101) - Presence graph, flood-fill patterns
- [Managing Subscriptions](https://docs.ditto.live/sdk/latest/sync/syncing-data) - Subscription + observer pattern
- [Observing Data Changes](https://docs.ditto.live/crud/observing-data-changes) - Store observers, reactive updates
- [Hello World Sync](https://docs.ditto.live/v4-7/get-started/hello-world-sync) - Basic integration pattern

### DDIL Architecture Research
- [Strata.io DDIL Guide](https://www.strata.io/blog/identity-continuity/ddil-resilient-identity-continuity/) - DDIL environment requirements
- [Legion Intelligence Centurion](https://www.globenewswire.com/news-release/2026/01/28/3227669/0/en/Legion-Intelligence-Introduces-Centurion-a-Deployable-Edge-AI-System-for-DDIL-Environments.html) - Multi-node mesh, offline-first patterns

### Offline-First Patterns
- [LogRocket: Offline-First Apps 2025](https://blog.logrocket.com/offline-first-frontend-apps-2025-indexeddb-sqlite/) - Local-first data flow
- [Medium: Offline-First Architecture](https://medium.com/@jusuftopic/offline-first-architecture-designing-for-reality-not-just-the-cloud-e5fd18e50a79) - Design patterns for offline operation

### 3D Printer Farm Management
- [3DPrinterOS](https://www.3dprinteros.com/3d-printer-farm-management-software) - Dashboard components, queue management
- [SimplyPrint](https://simplyprint.io/print-farms) - Multi-printer monitoring patterns
- [FDM Monster](https://github.com/fdm-monster/fdm-monster) - Open-source farm architecture reference

### Edge Database Patterns
- [ObjectBox: Edge Databases](https://objectbox.io/what-is-an-edge-database-and-why-do-you-need-one/) - Edge computing data flows
- [InfoQ: Data Patterns for Edge](https://www.infoq.com/articles/data-patterns-edge/) - Localization, sync strategies

## Confidence Assessment

| Area | Level | Notes |
|------|-------|-------|
| Ditto SDK Integration | HIGH | Verified from official docs + existing quickstart examples |
| Offline-First Patterns | HIGH | Well-documented, industry-standard approaches |
| DDIL Resilience | MEDIUM | Based on Legion/Strata references, but xCell-specific testing needed |
| 3D Printer Data Model | MEDIUM | Inferred from farm management platforms, needs real xCell API validation |
| Mesh Scalability | MEDIUM | Ditto's presence graph handles 100s of nodes, but printer-specific load unknown |

## Critical Build Dependencies

**Hard dependencies (blocking):**
1. Phase 1 blocks Phase 2: Simulator needs Ditto write API
2. Phase 2 blocks Phase 3: Dashboard needs data to display
3. Phase 3 blocks Phase 4: Multi-device testing needs working UI
4. Phase 4 blocks Phase 5: Cloud sync needs mesh validation first

**Soft dependencies (parallel work possible):**
- Printer data model can be defined alongside Phase 1
- Dashboard UI can be mocked with fake data during Phase 2
- Cloud dashboard can be developed in parallel with local dashboard (Phase 3 + 5)

## Next Steps for Roadmap Creation

**Phase structure implications:**

1. **MVP: Single-Device Local**
   - Phase 1-3 deliver a working dashboard with simulated data
   - No mesh required, validates core Ditto patterns
   - Milestone: "I can see printer status on one screen"

2. **V2: Local Mesh**
   - Phase 4 adds multi-device sync
   - Validates DDIL resilience in local environment
   - Milestone: "Two xCells sync via BLE at FOB"

3. **V3: Theater-Wide**
   - Phase 5-6 add cloud sync and fleet dashboard
   - Validates cross-FOB visibility
   - Milestone: "Commander sees all xCells from HQ"

**Research flags:**
- **Phase 2 (Printer Simulator):** Likely needs deeper research into xCell/MJF printer APIs if real integration planned
- **Phase 4 (Mesh Testing):** May need hardware setup guidance (Raspberry Pi, BLE dongles)
- **Phase 6 (Scalability):** May need load testing plan for 100+ printer scenario
