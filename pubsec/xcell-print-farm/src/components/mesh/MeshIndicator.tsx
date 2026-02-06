import type { PresenceState } from '../../types/mesh';

interface MeshIndicatorProps {
  presence: PresenceState;
}

export function MeshIndicator({ presence }: MeshIndicatorProps) {
  const { isConnected, peerCount, connectionTypes } = presence;

  // Color based on connectivity
  const statusColor = isConnected ? 'bg-green-500' : 'bg-yellow-500';
  const statusText = isConnected
    ? `${peerCount} peer${peerCount !== 1 ? 's' : ''}`
    : 'Connecting...';

  // Show connection type if WebSocket (indicates cloud relay)
  const viaCloud = connectionTypes.includes('WebSocket');

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
