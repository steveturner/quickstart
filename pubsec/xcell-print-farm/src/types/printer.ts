// Printer types matching C++ simulator schema (PrinterSimulator.cpp)

// Status values (from PrinterState.h state machine)
export type PrinterStatus = 'idle' | 'printing' | 'error';

// Location info (from Location struct in PrinterState.h)
export interface Location {
  name: string; // e.g., "Ramstein Air Base"
  site_code: string; // e.g., "FOB-ALPHA"
  latitude: number;
  longitude: number;
}

// Active print job (from PrintingState in PrinterState.h)
export interface PrintJob {
  job_id: string;
  progress: number; // 0.0 - 1.0
  current_layer: number;
  total_layers: number;
  nozzle_temp: number;
  bed_temp: number;
  filament_used_mm: number;
  filament_total_mm: number;
}

// Error state (from ErrorState in PrinterState.h)
export interface PrinterError {
  code: string; // E001, E002, E003, E004
  message: string;
}

// Main printer document (matches getTelemetry() output)
export interface Printer {
  _id: string;
  printer_id: string;
  status: PrinterStatus;
  location: Location;
  timestamp: number; // Unix epoch milliseconds
  deleted: boolean;
  job?: PrintJob; // Present when status === 'printing'
  error?: PrinterError; // Present when status === 'error'
}

// Aggregated location for globe markers
export interface LocationMarker {
  lat: number;
  lng: number;
  site_code: string;
  name: string;
  printerCount: number;
  statusSummary: {
    idle: number;
    printing: number;
    error: number;
  };
}
