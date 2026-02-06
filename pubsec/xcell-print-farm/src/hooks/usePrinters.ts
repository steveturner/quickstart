import { useEffect, useRef, useState } from 'react';
import {
  Ditto,
  init,
  IdentityOnlinePlayground,
  StoreObserver,
  SyncSubscription,
} from '@dittolive/ditto';
import type { Printer } from '../types/printer';

interface UsePrintersOptions {
  siteCode?: string | null;
}

export function usePrinters(options?: UsePrintersOptions) {
  const dittoRef = useRef<Ditto | null>(null);
  const [isInitialized, setIsInitialized] = useState(false);
  const [error, setError] = useState<Error | null>(null);
  const [printers, setPrinters] = useState<Printer[]>([]);
  const initStarted = useRef(false);
  const subscriptionRef = useRef<SyncSubscription | null>(null);
  const observerRef = useRef<StoreObserver | null>(null);
  const currentSiteRef = useRef<string | null | undefined>(undefined);

  useEffect(() => {
    if (initStarted.current) return;
    initStarted.current = true;

    const initializeDitto = async () => {
      try {
        // Two-stage init: first init() loads WASM, then new Ditto()
        await init();

        const identity: IdentityOnlinePlayground = {
          type: 'onlinePlayground',
          appID: import.meta.env.DITTO_APP_ID,
          token: import.meta.env.DITTO_PLAYGROUND_TOKEN,
          customAuthURL: import.meta.env.DITTO_AUTH_URL,
          enableDittoCloudSync: false,
        };

        dittoRef.current = new Ditto(identity);

        // Configure WebSocket transport
        dittoRef.current.updateTransportConfig((config) => {
          config.connect.websocketURLs = [import.meta.env.DITTO_WEBSOCKET_URL];
          return config;
        });

        // Disable sync with v3 peers (required for DQL)
        await dittoRef.current.disableSyncWithV3();

        // Disable strict mode for flexible queries
        await dittoRef.current.store.execute(
          'ALTER SYSTEM SET DQL_STRICT_MODE = false',
        );

        dittoRef.current.startSync();

        // Build queries based on siteCode
        const siteCode = options?.siteCode;
        const params = siteCode ? { site: siteCode } : undefined;

        const subscriptionQuery = siteCode
          ? 'SELECT * FROM printers WHERE location.site_code = :site'
          : 'SELECT * FROM printers';

        const observerQuery = siteCode
          ? 'SELECT * FROM printers WHERE location.site_code = :site AND deleted = false ORDER BY _id'
          : 'SELECT * FROM printers WHERE deleted = false ORDER BY location.site_code, _id';

        // Track initial site
        currentSiteRef.current = siteCode ?? null;

        // Register subscription (determines what syncs to this peer)
        subscriptionRef.current = dittoRef.current.sync.registerSubscription(
          subscriptionQuery,
          params,
        );

        // Register observer (runs against local database)
        observerRef.current = dittoRef.current.store.registerObserver<Printer>(
          observerQuery,
          (results) => {
            const docs = results.items.map((item) => item.value);
            setPrinters(docs);
            console.log('Printers updated:', docs.length);
          },
          params,
        );

        setIsInitialized(true);
        console.log('Ditto initialized for printers');
      } catch (e) {
        console.error('Failed to initialize Ditto:', e);
        setError(e as Error);
      }
    };

    initializeDitto();

    return () => {
      subscriptionRef.current?.cancel();
      observerRef.current?.cancel();
      dittoRef.current?.close();
    };
  }, []);

  // Handle siteCode changes after initialization
  useEffect(() => {
    // Skip on first render (init handles it)
    if (currentSiteRef.current === undefined) {
      return;
    }

    const newSite = options?.siteCode ?? null;
    if (currentSiteRef.current === newSite) return;
    currentSiteRef.current = newSite;

    // Re-subscribe only if Ditto is already initialized
    if (!dittoRef.current) return;

    // Cancel old subscription/observer
    subscriptionRef.current?.cancel();
    observerRef.current?.cancel();

    // Build new queries
    const params = newSite ? { site: newSite } : undefined;
    const subscriptionQuery = newSite
      ? 'SELECT * FROM printers WHERE location.site_code = :site'
      : 'SELECT * FROM printers';
    const observerQuery = newSite
      ? 'SELECT * FROM printers WHERE location.site_code = :site AND deleted = false ORDER BY _id'
      : 'SELECT * FROM printers WHERE deleted = false ORDER BY location.site_code, _id';

    // Register new subscription/observer
    subscriptionRef.current = dittoRef.current.sync.registerSubscription(
      subscriptionQuery,
      params,
    );
    observerRef.current = dittoRef.current.store.registerObserver<Printer>(
      observerQuery,
      (results) => {
        setPrinters(results.items.map((item) => item.value));
      },
      params,
    );
  }, [options?.siteCode]);

  return { ditto: dittoRef.current, isInitialized, error, printers };
}
