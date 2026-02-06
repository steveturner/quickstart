interface SiteSelectorProps {
  sites: string[]; // Available site codes
  selectedSite: string | null; // Currently selected (null = all)
  onSiteChange: (site: string | null) => void;
}

export function SiteSelector({
  sites,
  selectedSite,
  onSiteChange,
}: SiteSelectorProps) {
  const handleChange = (e: React.ChangeEvent<HTMLSelectElement>) => {
    const value = e.target.value;
    onSiteChange(value === 'all' ? null : value);
  };

  return (
    <div className="flex items-center gap-2">
      <label htmlFor="site-select" className="text-sm text-gray-400">
        Site:
      </label>
      <select
        id="site-select"
        value={selectedSite ?? 'all'}
        onChange={handleChange}
        className="bg-gray-800 border border-gray-700 rounded px-2 py-1 text-sm text-white focus:outline-none focus:border-blue-500"
      >
        <option value="all">All Sites (Theater)</option>
        {sites.map((site) => (
          <option key={site} value={site}>
            {site}
          </option>
        ))}
      </select>
    </div>
  );
}
