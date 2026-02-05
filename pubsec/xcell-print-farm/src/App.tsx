import { useDitto } from './hooks/useDitto';

function App() {
  const { ditto, isInitialized, error } = useDitto();

  return (
    <div className="min-h-screen bg-gray-900 text-white p-8">
      <header className="mb-8">
        <h1 className="text-3xl font-bold">xCell Print Farm Monitor</h1>
        <p className="text-gray-400 mt-2">DDIL-Resilient Manufacturing Visibility</p>
      </header>

      <div className="bg-gray-800 rounded-lg p-6 max-w-md">
        <h2 className="text-xl font-semibold mb-4">Ditto Status</h2>

        <div className="space-y-3">
          <div className="flex justify-between">
            <span className="text-gray-400">SDK Status:</span>
            <span className={isInitialized ? 'text-green-400' : 'text-yellow-400'}>
              {isInitialized ? 'Initialized' : 'Initializing...'}
            </span>
          </div>

          {ditto && (
            <div className="flex justify-between">
              <span className="text-gray-400">App ID:</span>
              <span className="font-mono text-sm">
                {import.meta.env.DITTO_APP_ID?.slice(0, 8)}...
              </span>
            </div>
          )}

          {error && (
            <div className="mt-4 p-3 bg-red-900/50 rounded text-red-300">
              Error: {error.message}
            </div>
          )}
        </div>
      </div>
    </div>
  );
}

export default App;
