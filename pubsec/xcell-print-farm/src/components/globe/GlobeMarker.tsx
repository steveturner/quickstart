import type { LocationMarker } from '../../types/printer';

/**
 * Create a DOM element for a globe marker.
 * react-globe.gl uses raw DOM for htmlElement, not React components.
 * Tailwind classes work because they're compiled at build time.
 */
export function createMarkerElement(
  marker: LocationMarker,
  selectedSite: string | null,
  onClick: (siteCode: string) => void,
): HTMLElement {
  const el = document.createElement('div');
  const isSelected = selectedSite === marker.site_code;

  // Base styles - dark background, rounded, pointer cursor
  el.className = `
    bg-gray-900/90 rounded-lg px-2 py-1 cursor-pointer
    border border-gray-700 hover:border-blue-500
    transition-all duration-200 select-none
    ${isSelected ? 'ring-2 ring-blue-500 border-blue-500' : ''}
  `
    .trim()
    .replace(/\s+/g, ' ');

  // Build status dots HTML
  const dots = [];
  const { statusSummary } = marker;

  // Green dots for idle
  for (let i = 0; i < statusSummary.idle; i++) {
    dots.push(
      '<span class="w-2 h-2 rounded-full bg-green-500 inline-block"></span>',
    );
  }
  // Blue dots for printing
  for (let i = 0; i < statusSummary.printing; i++) {
    dots.push(
      '<span class="w-2 h-2 rounded-full bg-blue-500 inline-block"></span>',
    );
  }
  // Red dots for error
  for (let i = 0; i < statusSummary.error; i++) {
    dots.push(
      '<span class="w-2 h-2 rounded-full bg-red-500 inline-block"></span>',
    );
  }

  el.innerHTML = `
    <div class="text-white text-xs whitespace-nowrap">
      <div class="font-bold">${marker.site_code}</div>
      <div class="text-gray-400 text-[10px]">${marker.printerCount} printer${marker.printerCount !== 1 ? 's' : ''}</div>
      <div class="flex gap-1 mt-1">
        ${dots.join('')}
      </div>
    </div>
  `;

  // Click handler
  el.addEventListener('click', (e) => {
    e.stopPropagation(); // Prevent globe click from firing
    onClick(marker.site_code);
  });

  return el;
}

// Re-export type for convenience
export type { LocationMarker };
