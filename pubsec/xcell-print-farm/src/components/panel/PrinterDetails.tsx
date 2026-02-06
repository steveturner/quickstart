import type { Printer } from '../../types/printer';
import { StatusBadge } from '../table/StatusBadge';
import { JobQueue } from '../queue/JobQueue';

interface PrinterDetailsProps {
  printer: Printer;
}

export function PrinterDetails({ printer }: PrinterDetailsProps) {
  const { job, error, location, status } = printer;

  // Calculate material percentage
  const materialPercent = job
    ? Math.round(
        ((job.filament_total_mm - job.filament_used_mm) /
          job.filament_total_mm) *
          100,
      )
    : 100;

  // Material color coding
  const getMaterialColor = (percent: number) => {
    if (percent < 10) return 'bg-red-500';
    if (percent < 20) return 'bg-amber-500';
    return 'bg-blue-500';
  };

  // Build queue from current job (future: job_queue collection)
  const queuedJobs = job
    ? [
        {
          id: job.job_id,
          name: `Job ${job.job_id}`,
          thumbnailUrl: undefined,
        },
      ]
    : [];

  return (
    <div className="space-y-6">
      {/* Header: Printer ID + Status */}
      <div className="flex items-center justify-between">
        <h2 className="text-xl font-semibold">{printer.printer_id}</h2>
        <StatusBadge status={status} />
      </div>

      {/* Location */}
      <section>
        <h3 className="text-sm font-medium text-gray-400 mb-2">Location</h3>
        <div className="bg-gray-700/50 rounded p-3 space-y-1">
          <p className="font-medium">{location.name}</p>
          <p className="text-sm text-gray-400">{location.site_code}</p>
          <p className="text-xs text-gray-500">
            {location.latitude.toFixed(4)}, {location.longitude.toFixed(4)}
          </p>
        </div>
      </section>

      {/* Current Job */}
      {status === 'printing' && job && (
        <section>
          <h3 className="text-sm font-medium text-gray-400 mb-2">
            Current Job
          </h3>
          <div className="bg-gray-700/50 rounded p-3 space-y-3">
            <p className="font-mono text-sm">{job.job_id}</p>

            {/* Progress bar */}
            <div>
              <div className="flex justify-between text-sm mb-1">
                <span>Progress</span>
                <span>{Math.round(job.progress * 100)}%</span>
              </div>
              <div className="h-2 bg-gray-600 rounded-full overflow-hidden">
                <div
                  className="h-full bg-blue-500 transition-all duration-500"
                  style={{ width: `${job.progress * 100}%` }}
                />
              </div>
            </div>

            {/* Layer progress */}
            <p className="text-sm text-gray-400">
              Layer {job.current_layer} of {job.total_layers}
            </p>
          </div>
        </section>
      )}

      {/* Temperatures */}
      {status === 'printing' && job && (
        <section>
          <h3 className="text-sm font-medium text-gray-400 mb-2">
            Temperatures
          </h3>
          <div className="bg-gray-700/50 rounded p-3 space-y-2">
            <div className="flex justify-between">
              <span className="text-gray-400">Nozzle</span>
              <span>
                {job.nozzle_temp.toFixed(0)}C{' '}
                <span className="text-gray-500">/ 220C target</span>
              </span>
            </div>
            <div className="flex justify-between">
              <span className="text-gray-400">Bed</span>
              <span>
                {job.bed_temp.toFixed(0)}C{' '}
                <span className="text-gray-500">/ 60C target</span>
              </span>
            </div>
          </div>
        </section>
      )}

      {/* Material */}
      {status === 'printing' && job && (
        <section>
          <h3 className="text-sm font-medium text-gray-400 mb-2">Material</h3>
          <div className="bg-gray-700/50 rounded p-3 space-y-3">
            {/* Material bar */}
            <div>
              <div className="flex justify-between text-sm mb-1">
                <span>Remaining</span>
                <span
                  className={
                    materialPercent < 10
                      ? 'text-red-400'
                      : materialPercent < 20
                        ? 'text-amber-400'
                        : ''
                  }
                >
                  {materialPercent}%
                </span>
              </div>
              <div className="h-2 bg-gray-600 rounded-full overflow-hidden">
                <div
                  className={`h-full transition-all duration-500 ${getMaterialColor(materialPercent)}`}
                  style={{ width: `${materialPercent}%` }}
                />
              </div>
            </div>

            {/* Consumption */}
            <p className="text-sm text-gray-400">
              {job.filament_used_mm.toFixed(0)} mm used /{' '}
              {job.filament_total_mm.toFixed(0)} mm total
            </p>
          </div>
        </section>
      )}

      {/* Error */}
      {status === 'error' && error && (
        <section>
          <h3 className="text-sm font-medium text-gray-400 mb-2">Error</h3>
          <div className="bg-red-900/30 border border-red-700 rounded p-3">
            <p className="font-mono text-red-400 text-sm mb-1">{error.code}</p>
            <p className="text-red-300">{error.message}</p>
          </div>
        </section>
      )}

      {/* Job Queue */}
      <section>
        <h3 className="text-sm font-medium text-gray-400 mb-2">Job Queue</h3>
        <div className="bg-gray-700/50 rounded p-3">
          <JobQueue jobs={queuedJobs} activeJobId={job?.job_id ?? null} />
        </div>
      </section>

      {/* Timestamp */}
      <p className="text-xs text-gray-500 text-center">
        Last update: {new Date(printer.timestamp).toLocaleTimeString()}
      </p>
    </div>
  );
}
