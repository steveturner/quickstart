import { StrictMode } from 'react';
import { createRoot } from 'react-dom/client';
import './index.css';

function App() {
  return (
    <div className="flex items-center justify-center h-full bg-gray-900 text-white">
      <h1 className="text-4xl font-bold">xCell Print Farm Monitor</h1>
    </div>
  );
}

createRoot(document.getElementById('root')!).render(
  <StrictMode>
    <App />
  </StrictMode>,
);
