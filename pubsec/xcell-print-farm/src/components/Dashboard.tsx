import { useState, useMemo, useEffect } from 'react';
import type { Printer } from '../types/printer';
import { usePrinters } from '../hooks/usePrinters';
import { usePresence } from '../hooks/usePresence';
import { aggregateLocations } from '../utils/aggregateLocations';
import { Globe } from './globe/Globe';
import { PrinterTable } from './table/PrinterTable';
import { DetailPanel } from './panel/DetailPanel';
import { PrinterDetails } from './panel/PrinterDetails';
import { MeshIndicator, SiteSelector } from './mesh';

export function Dashboard() {
  // siteFilter: DQL-level data filtering from dropdown (controls what syncs)
  // selectedSite: UI-level highlighting from globe click (visual feedback only)
  const [siteFilter, setSiteFilter] = useState<string | null>(null);
  const [selectedSite, setSelectedSite] = useState<string | null>(null);
  const [selectedPrinter, setSelectedPrinter] = useState<Printer | null>(null);

  // IMPORTANT: Add 'ditto' to destructuring - it's needed for usePresence hook
  const { ditto, isInitialized, error, printers } = usePrinters({ siteCode: siteFilter });
  const presence = usePresence(ditto);

  // Derive available sites from printers (for SiteSelector dropdown options)
  const availableSites = useMemo(() => {
    const sites = new Set(printers.map((p) => p.location.site_code));
    return Array.from(sites).sort();
  }, [printers]);

  // Aggregate locations for globe markers
  const locations = useMemo(() => aggregateLocations(printers), [printers]);

  // Keep selected printer in sync with live data
  useEffect(() => {
    if (selectedPrinter) {
      const updated = printers.find((p) => p._id === selectedPrinter._id);
      if (updated) {
        setSelectedPrinter(updated);
      } else {
        // Printer disappeared (deleted or offline)
        setSelectedPrinter(null);
      }
    }
  }, [printers, selectedPrinter?._id]);

  // Handle globe location click
  const handleLocationClick = (siteCode: string | null) => {
    setSelectedSite(siteCode);
  };

  // Handle table row click
  const handleRowClick = (printer: Printer) => {
    setSelectedPrinter(printer);
  };

  // Handle panel close
  const handlePanelClose = () => {
    setSelectedPrinter(null);
  };

  // Clear site filter
  const handleClearFilter = () => {
    setSelectedSite(null);
  };

  if (error) {
    return (
      <div className="min-h-screen bg-gray-900 text-white flex items-center justify-center">
        <div className="bg-red-900/50 border border-red-700 rounded-lg p-6 max-w-md">
          <h2 className="text-xl font-semibold text-red-400 mb-2">
            Connection Error
          </h2>
          <p className="text-red-300">{error.message}</p>
        </div>
      </div>
    );
  }

  return (
    <div className="min-h-screen bg-gray-900 text-white">
      {/* Header with Globe */}
      <header className="h-[400px] relative">
        <Globe
          locations={locations}
          onLocationClick={handleLocationClick}
          selectedSite={selectedSite}
        />

        {/* Header controls */}
        <div className="absolute top-4 left-4 z-10">
          <h1 className="text-2xl font-bold">xCell Print Farm Monitor</h1>
          <div className="flex items-center gap-4 mt-2">
            <MeshIndicator presence={presence} />
            <SiteSelector
              sites={availableSites}
              selectedSite={siteFilter}
              onSiteChange={setSiteFilter}
            />
            <span className="text-gray-400 text-sm">
              {isInitialized
                ? `${printers.length} printer${printers.length !== 1 ? 's' : ''}`
                : 'Connecting...'}
            </span>
          </div>
        </div>

        {/* Site filter badge */}
        {selectedSite && (
          <div className="absolute top-4 right-4 z-10">
            <button
              onClick={handleClearFilter}
              className="flex items-center gap-2 bg-blue-600 hover:bg-blue-700
                px-3 py-1.5 rounded text-sm transition-colors"
            >
              <span>
                Filtered: <span className="font-medium">{selectedSite}</span>
              </span>
              <svg
                className="w-4 h-4"
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
          </div>
        )}
      </header>

      {/* Printer Table */}
      <main className="p-6">
        <PrinterTable
          printers={printers}
          selectedSite={selectedSite}
          onRowClick={handleRowClick}
          selectedPrinterId={selectedPrinter?._id ?? null}
        />
      </main>

      {/* Detail Panel */}
      <DetailPanel isOpen={selectedPrinter !== null} onClose={handlePanelClose}>
        {selectedPrinter && <PrinterDetails printer={selectedPrinter} />}
      </DetailPanel>
    </div>
  );
}
