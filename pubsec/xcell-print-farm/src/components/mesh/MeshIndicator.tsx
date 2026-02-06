import type { PresenceState } from '../../types/mesh';

interface MeshIndicatorProps {
  presence: PresenceState;
}

export function MeshIndicator({ presence }: MeshIndicatorProps) {
  const { isConnected, peerCount, connectionTypes } = presence;

  // Color based on connectivity
  const statusColor = isConnected ? 'bg-green-500' : 'bg-yellow-500';
  const statusText = isConnected
    ? peerCount > 0
      ? `${peerCount} peer${peerCount !== 1 ? 's' : ''}`
      : 'Syncing'
    : 'Connecting...';

  // Web browsers sync via WebSocket to Big Peer (no P2P peers in presence graph)
  // Show "(via cloud)" when connected with no P2P peers, or when WebSocket detected
  const viaCloud =
    connectionTypes.includes('WebSocket') || (isConnected && peerCount === 0);

  return (
    <div className="flex items-center gap-2 text-sm">
      <div className={`w-2 h-2 rounded-full ${statusColor} animate-pulse`} />
      <span className="text-gray-300">{statusText}</span>
      {viaCloud && isConnected && (
        <span className="text-xs text-gray-500">(via cloud)</span>
      )}
    </div>
  );
}
