// Mesh/presence types for Ditto Presence API

// Info about a connected peer in the mesh
export interface PeerInfo {
  deviceName: string;
  connections: string[]; // Connection types: 'WebSocket', 'LAN', 'Bluetooth', etc.
}

// Overall mesh health state from Ditto presence graph
export interface PresenceState {
  isConnected: boolean; // true if presence graph exists (SDK initialized)
  peerCount: number; // Number of remote peers visible
  peers: PeerInfo[]; // Details about each peer
  connectionTypes: string[]; // Unique connection types in use
}
