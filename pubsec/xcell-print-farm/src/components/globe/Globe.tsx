import { useRef, useEffect, useCallback } from 'react';
import ReactGlobe, { GlobeMethods } from 'react-globe.gl';
import type { LocationMarker } from '../../types/printer';
import { createMarkerElement } from './GlobeMarker';

interface GlobeProps {
  locations: LocationMarker[];
  onLocationClick: (siteCode: string | null) => void;
  selectedSite: string | null;
}

export function Globe({ locations, onLocationClick, selectedSite }: GlobeProps) {
  const globeRef = useRef<GlobeMethods | undefined>();
  const containerRef = useRef<HTMLDivElement>(null);

  // Auto-rotate configuration
  useEffect(() => {
    if (globeRef.current) {
      const controls = globeRef.current.controls();
      controls.autoRotate = true;
      controls.autoRotateSpeed = 0.5;
      controls.enableZoom = true;
      controls.minDistance = 200;
      controls.maxDistance = 500;
    }
  }, []);

  // Handle visibility change to pause animation when tab inactive
  useEffect(() => {
    const handleVisibility = () => {
      if (globeRef.current) {
        const controls = globeRef.current.controls();
        controls.autoRotate = !document.hidden;
      }
    };
    document.addEventListener('visibilitychange', handleVisibility);
    return () => document.removeEventListener('visibilitychange', handleVisibility);
  }, []);

  // Handle globe background click to clear selection
  const handleGlobeClick = useCallback(() => {
    onLocationClick(null);
  }, [onLocationClick]);

  // Create HTML element for each marker
  const markerElement = useCallback(
    (d: object) => {
      const marker = d as LocationMarker;
      return createMarkerElement(marker, selectedSite, (siteCode) => {
        onLocationClick(siteCode);
      });
    },
    [selectedSite, onLocationClick]
  );

  return (
    <div ref={containerRef} className="w-full h-full">
      <ReactGlobe
        ref={globeRef}
        globeImageUrl="//unpkg.com/three-globe/example/img/earth-night.jpg"
        backgroundColor="rgba(0,0,0,0)"
        atmosphereColor="#1e40af"
        atmosphereAltitude={0.15}
        htmlElementsData={locations}
        htmlElement={markerElement}
        htmlAltitude={0.01}
        onGlobeClick={handleGlobeClick}
        width={containerRef.current?.clientWidth || 400}
        height={containerRef.current?.clientHeight || 300}
      />
    </div>
  );
}

export default Globe;
