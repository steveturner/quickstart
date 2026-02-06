import { useEffect, useRef, useState } from 'react';
import {
  Ditto,
  init,
  IdentityOnlinePlayground,
  StoreObserver,
  SyncSubscription,
} from '@dittolive/ditto';
import type { Printer } from '../types/printer';

export function usePrinters() {
  const dittoRef = useRef<Ditto | null>(null);
  const [isInitialized, setIsInitialized] = useState(false);
  const [error, setError] = useState<Error | null>(null);
  const [printers, setPrinters] = useState<Printer[]>([]);
  const initStarted = useRef(false);
  const subscriptionRef = useRef<SyncSubscription | null>(null);
  const observerRef = useRef<StoreObserver | null>(null);

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
          'ALTER SYSTEM SET DQL_STRICT_MODE = false'
        );

        dittoRef.current.startSync();

        // Register subscription (determines what syncs to this peer)
        subscriptionRef.current = dittoRef.current.sync.registerSubscription(
          'SELECT * FROM printers'
        );

        // Register observer (runs against local database)
        observerRef.current = dittoRef.current.store.registerObserver<Printer>(
          'SELECT * FROM printers WHERE deleted = false ORDER BY location.site_code, _id',
          (results) => {
            const docs = results.items.map((item) => item.value);
            setPrinters(docs);
            console.log('Printers updated:', docs.length);
          }
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

  return { ditto: dittoRef.current, isInitialized, error, printers };
}
