interface QueuedJob {
  id: string;
  name: string;
  thumbnailUrl?: string;
}

interface JobQueueProps {
  jobs: QueuedJob[];
  activeJobId: string | null;
}

export function JobQueue({ jobs, activeJobId }: JobQueueProps) {
  if (jobs.length === 0) {
    return (
      <div className="text-gray-500 text-sm py-4 text-center">
        No jobs in queue
      </div>
    );
  }

  return (
    <div className="flex gap-2 overflow-x-auto py-2">
      {jobs.map((job) => {
        const isActive = job.id === activeJobId;
        return (
          <div
            key={job.id}
            className={`flex-shrink-0 w-16 h-16 rounded border-2 overflow-hidden
              ${
                isActive
                  ? 'border-blue-400 ring-2 ring-blue-400/50'
                  : 'border-gray-600'
              }`}
            title={job.name}
          >
            {job.thumbnailUrl ? (
              <img
                src={job.thumbnailUrl}
                alt={job.name}
                className="w-full h-full object-cover"
              />
            ) : (
              <div className="w-full h-full bg-gray-700 flex items-center justify-center">
                <svg
                  className="w-8 h-8 text-gray-500"
                  fill="none"
                  stroke="currentColor"
                  viewBox="0 0 24 24"
                >
                  <path
                    strokeLinecap="round"
                    strokeLinejoin="round"
                    strokeWidth={1.5}
                    d="M20 7l-8-4-8 4m16 0l-8 4m8-4v10l-8 4m0-10L4 7m8 4v10M4 7v10l8 4"
                  />
                </svg>
              </div>
            )}
          </div>
        );
      })}
    </div>
  );
}
