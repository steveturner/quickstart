import { useEffect, useRef, useState } from 'react';
import { Ditto, init, IdentityOnlinePlayground } from '@dittolive/ditto';

export function useDitto() {
  const dittoRef = useRef<Ditto | null>(null);
  const [isInitialized, setIsInitialized] = useState(false);
  const [error, setError] = useState<Error | null>(null);
  const initStarted = useRef(false);

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
        setIsInitialized(true);
        console.log('Ditto initialized successfully');
      } catch (e) {
        console.error('Failed to initialize Ditto:', e);
        setError(e as Error);
      }
    };

    initializeDitto();

    return () => {
      dittoRef.current?.close();
    };
  }, []);

  return { ditto: dittoRef.current, isInitialized, error };
}
