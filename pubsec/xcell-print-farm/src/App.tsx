import { useDitto } from './hooks/useDitto';

function App() {
  const { ditto, isInitialized, error, documents, createDocument } = useDitto();

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

      <div className="bg-gray-800 rounded-lg p-6 max-w-md mt-6">
        <div className="flex justify-between items-center mb-4">
          <h2 className="text-xl font-semibold">Test Documents</h2>
          <span className="text-gray-400">{documents.length} documents</span>
        </div>

        <button
          onClick={() => createDocument(`Test document ${Date.now()}`)}
          disabled={!isInitialized}
          className="w-full bg-blue-600 hover:bg-blue-700 disabled:bg-gray-600
                     disabled:cursor-not-allowed px-4 py-2 rounded mb-4
                     transition-colors"
        >
          Create Test Document
        </button>

        <div className="space-y-2 max-h-64 overflow-y-auto">
          {documents.map((doc) => (
            <div key={doc._id} className="bg-gray-700 rounded p-3">
              <p className="text-sm">{doc.text}</p>
              <p className="text-xs text-gray-400 mt-1">
                {new Date(doc.createdAt).toLocaleTimeString()}
              </p>
            </div>
          ))}

          {documents.length === 0 && (
            <p className="text-gray-500 text-center py-4">
              No documents yet. Click the button above to create one.
            </p>
          )}
        </div>
      </div>
    </div>
  );
}

export default App;
