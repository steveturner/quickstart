import { useEffect, useRef, useState } from 'react';
import {
  Ditto,
  init,
  IdentityOnlinePlayground,
  StoreObserver,
  SyncSubscription,
} from '@dittolive/ditto';

export type TestDocument = {
  _id: string;
  text: string;
  createdAt: string;
  deleted: boolean;
};

export function useDitto() {
  const dittoRef = useRef<Ditto | null>(null);
  const [isInitialized, setIsInitialized] = useState(false);
  const [error, setError] = useState<Error | null>(null);
  const [documents, setDocuments] = useState<TestDocument[]>([]);
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
          'ALTER SYSTEM SET DQL_STRICT_MODE = false',
        );

        dittoRef.current.startSync();

        // Register subscription (determines what syncs to this peer)
        subscriptionRef.current = dittoRef.current.sync.registerSubscription(
          'SELECT * FROM test_documents',
        );

        // Register observer (runs against local database)
        observerRef.current =
          dittoRef.current.store.registerObserver<TestDocument>(
            'SELECT * FROM test_documents WHERE deleted=false ORDER BY createdAt DESC',
            (results) => {
              const docs = results.items.map((item) => item.value);
              setDocuments(docs);
              console.log('Documents updated:', docs.length);
            },
          );

        setIsInitialized(true);
        console.log('Ditto initialized successfully');
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

  const createDocument = async (text: string) => {
    if (!dittoRef.current) return;

    await dittoRef.current.store.execute(
      'INSERT INTO test_documents DOCUMENTS (:doc)',
      {
        doc: {
          text,
          createdAt: new Date().toISOString(),
          deleted: false,
        },
      },
    );
  };

  return {
    ditto: dittoRef.current,
    isInitialized,
    error,
    documents,
    createDocument,
  };
}
