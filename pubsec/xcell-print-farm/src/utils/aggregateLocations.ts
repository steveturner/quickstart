import type { Printer, LocationMarker } from '../types/printer';

/**
 * Aggregate printers by location for globe markers.
 * Groups printers by site_code and summarizes status counts.
 */
export function aggregateLocations(printers: Printer[]): LocationMarker[] {
  // Group printers by site_code
  const groups = new Map<
    string,
    {
      printers: Printer[];
      first: Printer;
    }
  >();

  for (const printer of printers) {
    const key = printer.location.site_code;
    const existing = groups.get(key);
    if (existing) {
      existing.printers.push(printer);
    } else {
      groups.set(key, { printers: [printer], first: printer });
    }
  }

  // Convert groups to LocationMarker array
  const markers: LocationMarker[] = [];

  for (const [site_code, group] of groups) {
    const statusSummary = {
      idle: 0,
      printing: 0,
      error: 0,
    };

    for (const printer of group.printers) {
      statusSummary[printer.status]++;
    }

    markers.push({
      lat: group.first.location.latitude,
      lng: group.first.location.longitude,
      site_code,
      name: group.first.location.name,
      printerCount: group.printers.length,
      statusSummary,
    });
  }

  return markers;
}
