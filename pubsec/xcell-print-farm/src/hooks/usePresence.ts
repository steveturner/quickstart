import { useEffect, useRef, useState } from 'react';
import type { Ditto, Observer } from '@dittolive/ditto';
import type { PresenceState, PeerInfo } from '../types/mesh';

export function usePresence(ditto: Ditto | null): PresenceState {
  const [state, setState] = useState<PresenceState>({
    isConnected: false,
    peerCount: 0,
    peers: [],
    connectionTypes: [],
  });
  const observerRef = useRef<Observer | null>(null);

  useEffect(() => {
    if (!ditto) return;

    observerRef.current = ditto.presence.observe((graph) => {
      const peers: PeerInfo[] = graph.remotePeers.map((p) => ({
        deviceName: p.deviceName,
        connections: p.connections.map((c) => c.connectionType),
      }));

      const connectionTypes = [...new Set(peers.flatMap((p) => p.connections))];

      setState({
        isConnected: true,
        peerCount: graph.remotePeers.length,
        peers,
        connectionTypes,
      });
    });

    return () => {
      observerRef.current?.stop();
    };
  }, [ditto]);

  return state;
}
