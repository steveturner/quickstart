# Phase 1: Local Foundation - Research

**Researched:** 2026-02-05
**Domain:** React web application with Ditto SDK offline-first architecture
**Confidence:** HIGH

## Summary

Phase 1 establishes the foundation for the entire xCell Print Farm monitoring system: a React web application that initializes the Ditto SDK and demonstrates local-first data persistence. This phase validates the core offline-first pattern before adding printer simulation, real-time dashboards, or mesh networking.

The standard approach leverages proven patterns from the existing ditto-quickstart/javascript-web example, which provides a production-ready template for React + TypeScript + Vite + Ditto integration. The key technical challenge is understanding Ditto's subscription + observer pattern, which differs from traditional REST API architectures.

For Docker Compose deployment, the research reveals that Ditto Big Peer is a managed cloud service, not a self-hosted Docker container. For local development, the "playground" mode connects directly to Ditto's cloud infrastructure via WebSocket, eliminating the need for local backend containers in Phase 1.

**Primary recommendation:** Start with the javascript-web quickstart pattern (React + Vite + Ditto SDK) and validate the subscription/observer pattern with a simple test document before layering in printer-specific features.

## Standard Stack

The established libraries/tools for React + Ditto web applications:

### Core

| Library | Version | Purpose | Why Standard |
|---------|---------|---------|--------------|
| React | 18.3.1+ | UI framework | Industry standard, proven with Ditto SDK in javascript-web quickstart |
| @dittolive/ditto | 4.13.1+ | Distributed sync SDK | Core technology - handles offline-first persistence and mesh sync |
| Vite | 6.0+ | Build tool | Modern replacement for CRA, fast HMR, used in all recent Ditto examples |
| TypeScript | 5.6+ | Type safety | Non-negotiable for Ditto SDK (provides type definitions), prevents runtime errors |

### Supporting

| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| Tailwind CSS | 3.4+ | Styling | Rapid UI development, used in javascript-web quickstart |
| ESLint | 9.15+ | Code quality | Standard linting, pre-configured in quickstart |
| Prettier | 3.5+ | Code formatting | Auto-format, eliminates style debates |

### Docker/Infrastructure

| Tool | Purpose | Why Standard |
|------|---------|--------------|
| Docker Compose | Container orchestration | Simple multi-container setup, industry standard for local development |
| Node.js 22 LTS | React dev server | LTS version ensures stability |
| nginx (optional) | Static file serving | Production-ready static hosting for built React app |

### Alternatives Considered

| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| Vite | Create React App | CRA is deprecated, Vite is 10-100x faster |
| React 18 | React 19 | React 19 released 2025, but Ditto examples use 18 - stick with proven version for Phase 1 |
| Tailwind CSS | styled-components | Tailwind has zero runtime overhead, faster builds |
| @dittolive/ditto | @dittolive/react-ditto | react-ditto is wrapper library - use core SDK for Phase 1 simplicity |

**Installation:**
```bash
# Core dependencies (from javascript-web quickstart)
npm install react@18.3.1 react-dom@18.3.1
npm install @dittolive/ditto@4.13.1
npm install -D vite@6.0.1 @vitejs/plugin-react-swc@3.5.0
npm install -D typescript@5.6.2 @types/react @types/react-dom

# Styling
npm install -D tailwindcss@3.4.16 autoprefixer postcss

# Code quality
npm install -D eslint@9.15.0 prettier@3.5.3
npm install -D eslint-config-prettier eslint-plugin-prettier
```

## Architecture Patterns

### Recommended Project Structure
```
xcell-dashboard/
├── src/
│   ├── App.tsx              # Main app component, Ditto initialization
│   ├── main.tsx             # React entry point
│   ├── index.css            # Global styles
│   ├── vite-env.d.ts        # Vite type declarations
│   └── components/
│       └── DittoInfo.tsx    # Ditto connection status display
├── public/                   # Static assets
├── .env                      # Ditto credentials (NEVER commit)
├── .env.sample               # Template for .env
├── docker-compose.yml        # Multi-container orchestration
├── Dockerfile                # React app container
├── vite.config.ts            # Vite configuration
├── tailwind.config.js        # Tailwind CSS config
├── tsconfig.json             # TypeScript config
├── package.json              # Dependencies
└── README.md                 # Setup instructions
```

### Pattern 1: Ditto Initialization (Two-Stage Async)

**What:** Initialize Ditto in two stages - first `init()` to download WASM, then `new Ditto()` to create instance

**When to use:** Every React app using Ditto SDK

**Example:**
```typescript
// Source: ditto-quickstart/javascript-web/src/App.tsx
import { Ditto, IdentityOnlinePlayground, init } from '@dittolive/ditto';
import { useEffect, useRef, useState } from 'react';

const identity: IdentityOnlinePlayground = {
  type: 'onlinePlayground',
  appID: import.meta.env.DITTO_APP_ID,
  token: import.meta.env.DITTO_PLAYGROUND_TOKEN,
  customAuthURL: import.meta.env.DITTO_AUTH_URL,
  enableDittoCloudSync: false,
};

const App = () => {
  const ditto = useRef<Ditto | null>(null);
  const [promisedInitialization, setPromisedInitialization] = useState<Promise<void> | null>(null);
  const [isInitialized, setIsInitialized] = useState<boolean>(false);

  // Stage 1: Download WASM
  useEffect(() => {
    const initializeDitto = async () => {
      try {
        await init(); // Downloads Ditto WebAssembly binary
      } catch (e) {
        console.error('Failed to initialize Ditto:', e);
      }
    };

    if (!promisedInitialization) setPromisedInitialization(initializeDitto());
  }, [promisedInitialization]);

  // Stage 2: Create Ditto instance and start sync
  useEffect(() => {
    if (!promisedInitialization) return;

    (async () => {
      await promisedInitialization;
      try {
        ditto.current = new Ditto(identity);

        // Configure transport (WebSocket for playground mode)
        ditto.current.updateTransportConfig((config) => {
          config.connect.websocketURLs = [import.meta.env.DITTO_WEBSOCKET_URL];
          return config;
        });

        await ditto.current.disableSyncWithV3(); // Required for DQL
        await ditto.current.store.execute('ALTER SYSTEM SET DQL_STRICT_MODE = false');

        ditto.current.startSync();
        setIsInitialized(true);
      } catch (e) {
        console.error('Failed to create Ditto instance:', e);
      }
    })();

    return () => {
      ditto.current?.close();
      ditto.current = null;
    };
  }, [promisedInitialization]);

  return <div>{isInitialized ? 'Ditto Ready' : 'Loading...'}</div>;
};
```

**Why this pattern:** React's strict mode calls useEffect twice in development. Tracking initialization state prevents duplicate Ditto instances.

### Pattern 2: Subscription + Observer Pairing

**What:** Always pair a subscription (declares what to sync) with an observer (reacts to local changes)

**When to use:** Any component displaying Ditto data

**Example:**
```typescript
// Source: ditto-quickstart/javascript-web/src/App.tsx
import { SyncSubscription, StoreObserver } from '@dittolive/ditto';

const tasksSubscription = useRef<SyncSubscription | null>(null);
const tasksObserver = useRef<StoreObserver | null>(null);

useEffect(() => {
  if (!ditto.current) return;

  // Subscription: "I want all tasks to sync to this device"
  tasksSubscription.current = ditto.current.sync.registerSubscription(
    'SELECT * FROM tasks'
  );

  // Observer: "Tell me when local tasks change"
  tasksObserver.current = ditto.current.store.registerObserver<Task>(
    'SELECT * FROM tasks WHERE deleted=false ORDER BY title ASC',
    (results) => {
      const tasks = results.items.map((item) => item.value);
      setTasks(tasks);
    }
  );

  return () => {
    tasksSubscription.current?.cancel();
    tasksObserver.current?.cancel();
  };
}, []);
```

**Why this pattern:** Subscription ensures data syncs to device. Observer provides reactive updates. Decouples sync from display logic.

### Pattern 3: Soft Delete for Offline Resilience

**What:** Mark documents as deleted rather than removing them

**When to use:** All delete operations in Ditto

**Example:**
```typescript
// Source: ditto-quickstart/javascript-web/src/App.tsx
const deleteTask = async (task: Task) => {
  await ditto.current?.store.execute(
    'UPDATE tasks SET deleted=true WHERE _id=:id',
    { id: task._id }
  );
};

// Observer filters out deleted items
const observer = ditto.current.store.registerObserver(
  'SELECT * FROM tasks WHERE deleted=false ORDER BY title ASC',
  (results) => setTasks(results.items.map(i => i.value))
);
```

**Why this pattern:** Hard deletes can create sync inconsistencies. Soft deletes allow deletion state to propagate through mesh.

### Pattern 4: Environment Variable Loading

**What:** Load Ditto credentials from .env file via Vite's import.meta.env

**When to use:** All Ditto configuration

**Example:**
```typescript
// vite.config.ts - Vite auto-loads .env files
export default defineConfig({
  plugins: [react()],
  // No special config needed - Vite handles .env automatically
});

// App.tsx - Access via import.meta.env
const identity: IdentityOnlinePlayground = {
  type: 'onlinePlayground',
  appID: import.meta.env.DITTO_APP_ID,
  token: import.meta.env.DITTO_PLAYGROUND_TOKEN,
  customAuthURL: import.meta.env.DITTO_AUTH_URL,
  enableDittoCloudSync: false,
};
```

**Why this pattern:** Keeps credentials out of source code. Vite's import.meta.env is type-safe and build-time replaced.

### Anti-Patterns to Avoid

- **Polling Ditto store:** Use observers instead of setInterval for reactive updates
- **Forgetting subscription:** Observer without subscription only sees locally-created data
- **Hard deletes:** Use soft delete pattern (deleted=true) for sync consistency
- **Blocking initialization:** Don't await Ditto init in render path - use useEffect
- **Direct .env access:** Use import.meta.env (Vite), not process.env (Node.js only)

## Don't Hand-Roll

Problems that look simple but have existing solutions:

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Reactive data updates | setInterval polling | Ditto store observers | Observers fire on local DB changes, no polling overhead |
| WASM loading | Custom fetch/instantiate | Ditto init() function | Handles WASM download, caching, initialization automatically |
| WebSocket reconnection | Custom retry logic | Ditto transport config | Handles reconnection, exponential backoff, fallback transports |
| Conflict resolution | Manual merge logic | Ditto CRDTs | Last-write-wins, counters, registers built-in |
| Offline state detection | navigator.onLine checks | Ditto sync status | More accurate than browser API, tracks mesh connectivity |

**Key insight:** Ditto SDK encapsulates complex distributed systems patterns. Treat it as a black box for Phase 1 - validate it works, don't reinvent its internals.

## Common Pitfalls

### Pitfall 1: Forgetting to Call init() Before new Ditto()

**What goes wrong:** Creating Ditto instance before WASM downloads causes cryptic errors

**Why it happens:** Ditto SDK requires WebAssembly module loaded before instantiation

**How to avoid:** Always call `await init()` in first useEffect, then create instance in second useEffect

**Warning signs:** Errors mentioning "WASM not initialized" or "Cannot read properties of undefined"

### Pitfall 2: Observer Without Matching Subscription

**What goes wrong:** Dashboard shows empty state despite other devices having data

**Why it happens:** Subscriptions declare what to sync; observers only see local data

**How to avoid:** Always pair subscription with observer. Use same query or broader in subscription.

**Warning signs:** Data appears when created locally but not when created on other devices

### Pitfall 3: Using process.env Instead of import.meta.env

**What goes wrong:** Environment variables are undefined at runtime

**Why it happens:** Vite uses import.meta.env, not Node.js process.env

**How to avoid:** Always use `import.meta.env.VITE_*` for Vite projects

**Warning signs:** Ditto initialization fails with "appID is undefined"

### Pitfall 4: Not Disabling Sync with V3 Peers

**What goes wrong:** DQL queries fail or return unexpected results

**Why it happens:** Ditto v4 SDK requires explicit opt-out of v3 peer sync for DQL compatibility

**How to avoid:** Call `await ditto.disableSyncWithV3()` after creating instance

**Warning signs:** Errors mentioning "DQL not available" or "incompatible peer version"

### Pitfall 5: Hard Deleting Documents

**What goes wrong:** Deleted items reappear when syncing with peers

**Why it happens:** Hard deletes don't propagate well in mesh sync; CRDT tombstones needed

**How to avoid:** Use soft delete pattern (UPDATE SET deleted=true, not DELETE)

**Warning signs:** Inconsistent data across devices, items "resurrecting" after sync

## Code Examples

Verified patterns from official Ditto quickstart:

### Creating Documents
```typescript
// Source: ditto-quickstart/javascript-web/src/App.tsx
const createTask = async (title: string) => {
  await ditto.current?.store.execute(
    'INSERT INTO tasks DOCUMENTS (:task)',
    {
      task: {
        title,
        done: false,
        deleted: false,
      },
    },
  );
};
```

### Updating Documents
```typescript
// Source: ditto-quickstart/javascript-web/src/App.tsx
const toggleTask = async (task: Task) => {
  await ditto.current?.store.execute(
    'UPDATE tasks SET done=:done WHERE _id=:id',
    {
      id: task._id,
      done: !task.done,
    },
  );
};
```

### Docker Compose for React Development
```yaml
# Example structure (to be created in Phase 1)
version: '3.8'

services:
  dashboard:
    build:
      context: .
      dockerfile: Dockerfile
    ports:
      - "5173:5173"
    volumes:
      - .:/app
      - /app/node_modules
    environment:
      - DITTO_APP_ID=${DITTO_APP_ID}
      - DITTO_PLAYGROUND_TOKEN=${DITTO_PLAYGROUND_TOKEN}
      - DITTO_AUTH_URL=${DITTO_AUTH_URL}
      - DITTO_WEBSOCKET_URL=${DITTO_WEBSOCKET_URL}
    command: npm run dev
```

### Dockerfile for React + Vite
```dockerfile
# Example structure (to be created in Phase 1)
FROM node:22-alpine

WORKDIR /app

# Copy package files
COPY package*.json ./

# Install dependencies
RUN npm install

# Copy source code
COPY . .

# Expose Vite dev server port
EXPOSE 5173

# Default command (overridable in docker-compose)
CMD ["npm", "run", "dev", "--", "--host", "0.0.0.0"]
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| Create React App | Vite | 2023 | 10-100x faster builds, native ESM support |
| process.env | import.meta.env | Vite adoption | Type-safe env vars, build-time replacement |
| Ditto v3 SDK | Ditto v4 SDK + DQL | 2024 | SQL-like queries replace document API |
| Manual WebSocket | Ditto transport config | Always | Simplified configuration, automatic reconnection |
| Redux for sync state | Ditto observers | Always | Reactive updates without Redux boilerplate |

**Deprecated/outdated:**
- Ditto v3 SDK: Use v4.13+ for DQL support
- @dittolive/react-ditto early versions: Core SDK has improved React patterns
- enableDittoCloudSync=true: Set to false for playground mode to avoid confusion

## Open Questions

Things that couldn't be fully resolved:

1. **Ditto Big Peer Docker Self-Hosting**
   - What we know: Big Peer is Ditto's managed cloud service, not a self-hosted container
   - What's unclear: Whether self-hosted Big Peer exists or if it's cloud-only
   - Recommendation: For Phase 1, use playground mode (connects to Ditto cloud). Defer Big Peer investigation to Phase 5 (Cloud Sync). Contact Ditto support if self-hosting is critical.

2. **React 19 Compatibility**
   - What we know: Ditto quickstart uses React 18.3.1, React 19 released Dec 2024
   - What's unclear: Whether @dittolive/ditto@4.13.1 has been tested with React 19
   - Recommendation: Stick with React 18 for Phase 1 (proven), upgrade to 19 in later phase if needed

3. **Docker Compose Ditto Backend**
   - What we know: Playground mode connects to Ditto cloud via WebSocket, no local backend needed
   - What's unclear: PROJECT.md mentions "Ditto sync backend runs as container" (INFRA-04)
   - Recommendation: Clarify requirement - if local development only needs playground mode, INFRA-04 may be deferred to Phase 5 when exploring self-hosted options

4. **DQL Strict Mode**
   - What we know: Quickstart disables strict mode with `ALTER SYSTEM SET DQL_STRICT_MODE = false`
   - What's unclear: Production best practices for strict mode (should it be enabled?)
   - Recommendation: Keep disabled for Phase 1 (matches quickstart), revisit in Phase 3 when defining printer data schemas

## Sources

### Primary (HIGH confidence)
- [Ditto JavaScript SDK Install Guide](https://docs.ditto.live/sdk/latest/install-guides/js) - Official installation documentation
- [ditto-quickstart/javascript-web](https://github.com/getditto/ditto-quickstart/tree/main/javascript-web) - Production reference implementation
- [@dittolive/ditto npm package](https://www.npmjs.com/package/@dittolive/ditto) - Package versions and API
- [Ditto DQL Documentation](https://docs.ditto.live/dql/) - Query language reference
- [Vite Documentation](https://vite.dev/guide/) - Build tool configuration

### Secondary (MEDIUM confidence)
- [About Ditto](https://docs.ditto.live/about-ditto) - Platform overview, Big Peer architecture
- [Ditto Cloud Release Notes](https://docs.ditto.live/cloud/release-notes) - Big Peer version history
- [@dittolive/react-ditto library](https://getditto.github.io/react-ditto/) - React wrapper patterns
- [Ditto for Developers](https://resources.ditto.live/developers) - General resources

### Tertiary (LOW confidence - marked for validation)
- Eclipse Ditto project Docker examples - Different product (Eclipse IoT digital twins), not applicable to Ditto.live
- Self-hosted Big Peer references - No concrete Docker setup found, may require enterprise license

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH - Verified from official quickstart, actively maintained
- Architecture patterns: HIGH - Extracted directly from working javascript-web example
- Docker Compose: MEDIUM - General Docker knowledge applied, Ditto-specific backend unclear
- Big Peer self-hosting: LOW - No official documentation found, may be cloud-only

**Research date:** 2026-02-05
**Valid until:** 2026-03-05 (30 days - stable SDK versions, minimal API churn expected)

## Phase 1 Success Criteria Mapping

Mapping research findings to Phase 1 success criteria:

1. **Docker Compose brings up full stack (React app + Ditto backend)**
   - React app: Standard Vite + Docker pattern documented above
   - Ditto backend: Open question - playground mode may not need local backend container
   - **Action needed:** Clarify INFRA-04 requirement with user

2. **React app successfully initializes Ditto SDK and creates local store**
   - Proven pattern: Two-stage initialization (init() → new Ditto())
   - Confidence: HIGH (exact code from javascript-web quickstart)

3. **App writes test document to Ditto and observes it reactively**
   - Proven pattern: INSERT INTO + registerObserver
   - Confidence: HIGH (working examples available)

4. **App works fully offline without network connectivity**
   - Ditto local store persists to IndexedDB automatically
   - Observers react to local writes immediately
   - Confidence: HIGH (offline-first is Ditto's core design)

**Overall Phase 1 readiness:** HIGH - All patterns proven except Docker backend question
