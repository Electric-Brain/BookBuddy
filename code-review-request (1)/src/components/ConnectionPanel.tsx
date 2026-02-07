import { useState } from 'react';
import { Wifi, WifiOff, Play, MonitorSmartphone, Download } from 'lucide-react';

interface Props {
  connected: boolean;
  error: string | null;
  demoMode: boolean;
  onConnect: (ip: string) => void;
  onDemo: () => void;
}

export function ConnectionPanel({ connected, error, demoMode, onConnect, onDemo }: Props) {
  const [ip, setIp] = useState('');

  const handleConnect = () => {
    const trimmed = ip.trim().replace(/^https?:\/\//, '').replace(/\/$/, '');
    if (!trimmed) return;
    onConnect(trimmed);
  };

  const handleKeyDown = (e: React.KeyboardEvent) => {
    if (e.key === 'Enter') handleConnect();
  };

  if (connected) {
    return (
      <div className="flex items-center gap-2 px-4 py-2 bg-emerald-500/10 border border-emerald-500/30 rounded-xl">
        <Wifi className="w-4 h-4 text-emerald-400" />
        <span className="text-emerald-300 text-sm font-medium">
          {demoMode ? 'Demo Mode Active' : `Connected to BookBuddy`}
        </span>
        <span className="relative flex h-2 w-2 ml-1">
          <span className="animate-ping absolute inline-flex h-full w-full rounded-full bg-emerald-400 opacity-75"></span>
          <span className="relative inline-flex rounded-full h-2 w-2 bg-emerald-500"></span>
        </span>
      </div>
    );
  }

  return (
    <div className="space-y-3">
      <div className="flex flex-col sm:flex-row gap-2">
        <div className="relative flex-1">
          <MonitorSmartphone className="absolute left-3 top-1/2 -translate-y-1/2 w-4 h-4 text-slate-400" />
          <input
            type="text"
            value={ip}
            onChange={(e) => setIp(e.target.value)}
            onKeyDown={handleKeyDown}
            placeholder="Enter IP address (e.g. 10.152.138.159)"
            className="w-full pl-10 pr-4 py-2.5 bg-slate-800 border border-slate-600 rounded-xl text-white placeholder-slate-400 focus:outline-none focus:border-sky-500 focus:ring-1 focus:ring-sky-500 text-sm"
          />
        </div>
        <button
          onClick={handleConnect}
          disabled={!ip.trim()}
          className="px-5 py-2.5 bg-sky-600 hover:bg-sky-500 disabled:bg-slate-600 disabled:cursor-not-allowed text-white rounded-xl font-medium text-sm transition-colors flex items-center justify-center gap-2 whitespace-nowrap"
        >
          <Wifi className="w-4 h-4" />
          Connect
        </button>
        <button
          onClick={onDemo}
          className="px-5 py-2.5 bg-violet-600 hover:bg-violet-500 text-white rounded-xl font-medium text-sm transition-colors flex items-center justify-center gap-2 whitespace-nowrap"
        >
          <Play className="w-4 h-4" />
          Demo
        </button>
      </div>
      {error && (
        <div className="flex items-center gap-2 px-3 py-2 bg-red-500/10 border border-red-500/30 rounded-lg">
          <WifiOff className="w-4 h-4 text-red-400 shrink-0" />
          <span className="text-red-300 text-xs">{error}</span>
        </div>
      )}
      <div className="flex items-center gap-2">
        <a
          href="./BookBuddy_V3.ino"
          download="BookBuddy_V3.ino"
          className="px-3 py-1.5 bg-slate-700 hover:bg-slate-600 rounded-lg text-xs text-slate-300 transition-colors flex items-center gap-1.5"
        >
          <Download className="w-3 h-3" />
          Download ESP32 .ino File
        </a>
        <span className="text-[10px] text-slate-500">Upload to Arduino IDE for ESP32</span>
      </div>
    </div>
  );
}
