import { createColumnHelper } from '@tanstack/react-table';
import { Printer, PrintJob } from '../../types/printer';
import { StatusBadge } from './StatusBadge';

const columnHelper = createColumnHelper<Printer>();

// Format remaining time as "Xh Ym (HH:MM)"
export function formatTimeRemaining(
  progress: number,
  totalSeconds: number,
): string {
  if (progress >= 1) return 'Complete';
  const remainingSeconds = Math.round(totalSeconds * (1 - progress));
  const hours = Math.floor(remainingSeconds / 3600);
  const minutes = Math.floor((remainingSeconds % 3600) / 60);

  // Calculate ETA
  const now = new Date();
  const eta = new Date(now.getTime() + remainingSeconds * 1000);
  const etaStr = eta.toLocaleTimeString('en-US', {
    hour: '2-digit',
    minute: '2-digit',
    hour12: false,
  });

  if (hours > 0) {
    return `${hours}h ${minutes}m (${etaStr})`;
  }
  return `${minutes}m (${etaStr})`;
}

// Calculate material percentage remaining
export function getMaterialPercentage(job: PrintJob): number {
  if (job.filament_total_mm === 0) return 100;
  const used = job.filament_used_mm / job.filament_total_mm;
  return Math.round((1 - used) * 100);
}

// Get color class based on material level
export function getMaterialColorClass(percentage: number): string {
  if (percentage < 10) return 'text-red-400';
  if (percentage < 20) return 'text-amber-400';
  return 'text-gray-300';
}

export const columns = [
  columnHelper.accessor('_id', {
    header: 'ID',
    cell: (info) => {
      const id = info.getValue();
      return (
        <span title={id} className="font-mono">
          {id.length > 8 ? `${id.slice(0, 8)}...` : id}
        </span>
      );
    },
  }),

  columnHelper.accessor('status', {
    header: 'Status',
    cell: (info) => <StatusBadge status={info.getValue()} />,
  }),

  columnHelper.accessor('location', {
    header: 'Location',
    cell: (info) => {
      const loc = info.getValue();
      return (
        <span title={loc.name} className="cursor-help">
          {loc.site_code}
        </span>
      );
    },
  }),

  columnHelper.accessor('job', {
    header: 'Job',
    cell: (info) => {
      const job = info.getValue();
      return job ? (
        <span className="font-mono text-xs">{job.job_id}</span>
      ) : (
        <span className="text-gray-500">-</span>
      );
    },
  }),

  columnHelper.display({
    id: 'progress',
    header: 'Progress',
    cell: (info) => {
      const printer = info.row.original;
      if (printer.status !== 'printing' || !printer.job) {
        return <span className="text-gray-500">-</span>;
      }
      const pct = Math.round(printer.job.progress * 100);
      return (
        <div className="flex items-center gap-2">
          <div className="w-16 h-2 bg-gray-700 rounded-full overflow-hidden">
            <div
              className="h-full bg-blue-500 transition-all"
              style={{ width: `${pct}%` }}
            />
          </div>
          <span className="text-xs">{pct}%</span>
        </div>
      );
    },
  }),

  columnHelper.display({
    id: 'timeLeft',
    header: 'Time Left',
    cell: (info) => {
      const printer = info.row.original;
      if (printer.status !== 'printing' || !printer.job) {
        return <span className="text-gray-500">-</span>;
      }
      // total_duration_seconds is calculated from (total_layers * layer_time)
      // For display, we estimate from progress and a typical 2hr print
      const totalSeconds = printer.job.total_layers * 10; // ~10s per layer estimate
      return (
        <span className="text-xs">
          {formatTimeRemaining(printer.job.progress, totalSeconds)}
        </span>
      );
    },
  }),

  columnHelper.display({
    id: 'temps',
    header: 'Temps',
    cell: (info) => {
      const printer = info.row.original;
      if (printer.status !== 'printing' || !printer.job) {
        return <span className="text-gray-500">-/-</span>;
      }
      return (
        <span className="font-mono text-xs">
          {Math.round(printer.job.nozzle_temp)}/
          {Math.round(printer.job.bed_temp)}
        </span>
      );
    },
  }),

  columnHelper.display({
    id: 'material',
    header: 'Material',
    cell: (info) => {
      const printer = info.row.original;
      if (printer.status !== 'printing' || !printer.job) {
        return <span className="text-gray-500">-</span>;
      }
      const pct = getMaterialPercentage(printer.job);
      const colorClass = getMaterialColorClass(pct);
      return <span className={`text-xs ${colorClass}`}>{pct}%</span>;
    },
  }),
];
