import { ReactNode } from 'react';

interface DetailPanelProps {
  isOpen: boolean;
  onClose: () => void;
  children: ReactNode;
}

export function DetailPanel({ isOpen, onClose, children }: DetailPanelProps) {
  return (
    <>
      {/* Overlay backdrop */}
      {isOpen && (
        <div
          className="fixed inset-0 bg-black/50 z-40"
          onClick={onClose}
          aria-hidden="true"
        />
      )}

      {/* Slide-out panel */}
      <div
        className={`fixed top-0 right-0 h-full w-96 bg-gray-800 z-50
          transform transition-transform duration-300 ease-in-out
          ${isOpen ? 'translate-x-0' : 'translate-x-full'}`}
      >
        {/* Close button */}
        <button
          onClick={onClose}
          className="absolute top-4 right-4 text-gray-400 hover:text-white
            p-1 rounded hover:bg-gray-700 transition-colors"
          aria-label="Close panel"
        >
          <svg
            className="w-6 h-6"
            fill="none"
            stroke="currentColor"
            viewBox="0 0 24 24"
          >
            <path
              strokeLinecap="round"
              strokeLinejoin="round"
              strokeWidth={2}
              d="M6 18L18 6M6 6l12 12"
            />
          </svg>
        </button>

        {/* Scrollable content area */}
        <div className="h-full overflow-y-auto p-6 pt-14">{children}</div>
      </div>
    </>
  );
}
