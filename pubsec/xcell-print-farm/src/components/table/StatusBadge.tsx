import { PrinterStatus } from '../../types/printer';

interface StatusBadgeProps {
  status: PrinterStatus;
}

const statusConfig: Record<
  PrinterStatus,
  { bgClass: string; textClass: string; label: string }
> = {
  idle: {
    bgClass: 'bg-green-500/20',
    textClass: 'text-green-400',
    label: 'Idle',
  },
  printing: {
    bgClass: 'bg-blue-500/20',
    textClass: 'text-blue-400',
    label: 'Printing',
  },
  error: {
    bgClass: 'bg-red-500/20',
    textClass: 'text-red-400',
    label: 'Error',
  },
};

export function StatusBadge({ status }: StatusBadgeProps) {
  const config = statusConfig[status];
  return (
    <span
      className={`px-2 py-1 rounded text-xs font-medium ${config.bgClass} ${config.textClass}`}
    >
      {config.label}
    </span>
  );
}
