import { useMemo, useState } from 'react';
import {
  useReactTable,
  getCoreRowModel,
  getSortedRowModel,
  getFilteredRowModel,
  flexRender,
  SortingState,
} from '@tanstack/react-table';
import { Printer } from '../../types/printer';
import { columns } from './columns';

interface PrinterTableProps {
  printers: Printer[];
  selectedSite: string | null;
  onRowClick: (printer: Printer) => void;
  selectedPrinterId: string | null;
}

export function PrinterTable({
  printers,
  selectedSite,
  onRowClick,
  selectedPrinterId,
}: PrinterTableProps) {
  const [sorting, setSorting] = useState<SortingState>([]);

  // Filter printers by site when selectedSite is set
  const filteredPrinters = useMemo(() => {
    if (!selectedSite) return printers;
    return printers.filter((p) => p.location.site_code === selectedSite);
  }, [printers, selectedSite]);

  const table = useReactTable({
    data: filteredPrinters,
    columns,
    state: { sorting },
    onSortingChange: setSorting,
    getCoreRowModel: getCoreRowModel(),
    getSortedRowModel: getSortedRowModel(),
    getFilteredRowModel: getFilteredRowModel(),
  });

  return (
    <div className="overflow-auto">
      <table className="w-full text-sm">
        <thead className="bg-gray-800 sticky top-0">
          {table.getHeaderGroups().map((headerGroup) => (
            <tr key={headerGroup.id}>
              {headerGroup.headers.map((header) => (
                <th
                  key={header.id}
                  className="px-3 py-2 text-left text-xs font-medium text-gray-400 uppercase tracking-wider cursor-pointer select-none hover:text-white"
                  onClick={header.column.getToggleSortingHandler()}
                >
                  <div className="flex items-center gap-1">
                    {flexRender(
                      header.column.columnDef.header,
                      header.getContext(),
                    )}
                    {{
                      asc: ' ^',
                      desc: ' v',
                    }[header.column.getIsSorted() as string] ?? null}
                  </div>
                </th>
              ))}
            </tr>
          ))}
        </thead>
        <tbody className="divide-y divide-gray-700">
          {table.getRowModel().rows.map((row) => (
            <tr
              key={row.id}
              className={`cursor-pointer hover:bg-gray-700 ${
                row.original._id === selectedPrinterId ? 'bg-gray-700' : ''
              } even:bg-gray-800/50`}
              onClick={() => onRowClick(row.original)}
            >
              {row.getVisibleCells().map((cell) => (
                <td key={cell.id} className="px-3 py-2 whitespace-nowrap">
                  {flexRender(cell.column.columnDef.cell, cell.getContext())}
                </td>
              ))}
            </tr>
          ))}
        </tbody>
      </table>
      {filteredPrinters.length === 0 && (
        <div className="text-center py-8 text-gray-500">
          {selectedSite
            ? `No printers at ${selectedSite}`
            : 'No printers connected'}
        </div>
      )}
    </div>
  );
}
