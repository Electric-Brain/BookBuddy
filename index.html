import { useState, useEffect } from 'react';
import { BookBuddyFace } from './components/BookBuddyFace';
import { ConnectionPanel } from './components/ConnectionPanel';
import { ReadingProgressChart, SubjectBarChart, TimeDistributionPie, EmotionTimeline } from './components/Charts';
import { useBookBuddy } from './hooks/useBookBuddy';
import {
  BookOpen, Clock, Timer, BookMarked,
  CheckCircle2, XCircle, RefreshCw, Smile,
} from 'lucide-react';

const BOOK_COLORS: Record<string, string> = {
  English: 'from-blue-500 to-blue-600',
  Hindi: 'from-orange-500 to-orange-600',
  Maths: 'from-violet-500 to-violet-600',
  Marathi: 'from-emerald-500 to-emerald-600',
  Science: 'from-amber-500 to-amber-600',
};

const BOOK_ICONS: Record<string, string> = {
  English: '📖',
  Hindi: '📕',
  Maths: '📐',
  Marathi: '📗',
  Science: '🔬',
};

function formatDuration(seconds: number): string {
  if (seconds < 60) return `${seconds}s`;
  const m = Math.floor(seconds / 60);
  const s = seconds % 60;
  if (m < 60) return `${m}m ${s}s`;
  const h = Math.floor(m / 60);
  return `${h}h ${m % 60}m`;
}

export function App() {
  const [ipAddress, setIpAddress] = useState<string | null>(null);
  const [currentTime, setCurrentTime] = useState(new Date());

  const {
    connected, status, timeline, error, demoMode,
    connect, syncTime, startDemo,
  } = useBookBuddy(ipAddress);

  // Clock tick
  useEffect(() => {
    const interval = setInterval(() => setCurrentTime(new Date()), 1000);
    return () => clearInterval(interval);
  }, []);

  const handleConnect = (ip: string) => {
    setIpAddress(ip);
    connect(ip);
  };

  const handleSyncTime = async () => {
    if (ipAddress) {
      await syncTime(ipAddress);
    }
  };

  const totalStudyTime = status?.totalStudySeconds || 0;
  const booksPlaced = status?.booksPlaced || 0;
  const emotion = status?.emotion || 'Neutral';
  const books = status?.books || [];

  return (
    <div className="min-h-screen bg-gradient-to-br from-slate-950 via-slate-900 to-slate-950 text-white">
      {/* Subtle animated background */}
      <div className="fixed inset-0 overflow-hidden pointer-events-none">
        <div className="absolute top-0 left-1/4 w-96 h-96 bg-sky-500/5 rounded-full blur-3xl animate-pulse" />
        <div className="absolute bottom-0 right-1/4 w-96 h-96 bg-violet-500/5 rounded-full blur-3xl animate-pulse" style={{ animationDelay: '2s' }} />
      </div>

      <div className="relative z-10 max-w-7xl mx-auto px-4 py-4">
        {/* Header */}
        <header className="flex flex-col lg:flex-row items-start lg:items-center justify-between gap-4 mb-6">
          <div className="flex items-center gap-3">
            <div className="w-10 h-10 rounded-xl bg-gradient-to-br from-sky-500 to-blue-600 flex items-center justify-center shadow-lg shadow-sky-500/20">
              <BookOpen className="w-5 h-5 text-white" />
            </div>
            <div>
              <h1 className="text-xl font-bold bg-gradient-to-r from-sky-300 to-blue-400 bg-clip-text text-transparent">
                BookBuddy Dashboard
              </h1>
              <p className="text-xs text-slate-400">
                {currentTime.toLocaleTimeString('en-IN', { hour: '2-digit', minute: '2-digit', second: '2-digit' })}
                {' · '}
                {currentTime.toLocaleDateString('en-IN', { weekday: 'short', day: 'numeric', month: 'short' })}
              </p>
            </div>
          </div>

          <div className="w-full lg:w-auto">
            <ConnectionPanel
              connected={connected}
              error={error}
              demoMode={demoMode}
              onConnect={handleConnect}
              onDemo={startDemo}
            />
          </div>
        </header>

        {!connected ? (
          /* ─── Welcome Screen ─── */
          <div className="flex flex-col items-center justify-center py-20">
            <div className="w-32 h-32 rounded-3xl bg-gradient-to-br from-sky-200 to-blue-300 flex items-center justify-center mb-6 shadow-2xl shadow-sky-500/20">
              <BookOpen className="w-16 h-16 text-slate-700" />
            </div>
            <h2 className="text-2xl font-bold mb-2">Welcome to BookBuddy</h2>
            <p className="text-slate-400 text-center max-w-md mb-6">
              Connect your ESP32 BookBuddy device by entering its IP address, or try the Demo mode to explore the dashboard.
            </p>
            <div className="flex flex-wrap gap-3 justify-center">
              <div className="px-4 py-2 bg-slate-800/60 rounded-xl border border-slate-700/50 text-sm text-slate-300">
                📡 Enter your ESP32 IP address above
              </div>
              <div className="px-4 py-2 bg-slate-800/60 rounded-xl border border-slate-700/50 text-sm text-slate-300">
                📊 Track 5 subjects in real-time
              </div>
              <div className="px-4 py-2 bg-slate-800/60 rounded-xl border border-slate-700/50 text-sm text-slate-300">
                😊 Live emotion tracking
              </div>
            </div>
          </div>
        ) : (
          /* ─── Main Dashboard ─── */
          <div className="space-y-4">
            {/* Top Row: Face + Stats */}
            <div className="grid grid-cols-1 lg:grid-cols-12 gap-4">
              {/* Face + Emotion */}
              <div className="lg:col-span-3">
                <div className="bg-slate-800/60 backdrop-blur rounded-2xl p-4 border border-slate-700/50 h-full flex flex-col items-center justify-center">
                  <BookBuddyFace emotion={emotion} booksPlaced={booksPlaced} />
                  {ipAddress && !demoMode && (
                    <button
                      onClick={handleSyncTime}
                      className="mt-2 flex items-center gap-1.5 px-3 py-1.5 bg-slate-700 hover:bg-slate-600 rounded-lg text-xs text-slate-300 transition-colors"
                    >
                      <RefreshCw className="w-3 h-3" />
                      Sync RTC Time
                    </button>
                  )}
                </div>
              </div>

              {/* Stat Cards */}
              <div className="lg:col-span-9">
                <div className="grid grid-cols-2 md:grid-cols-4 gap-3 mb-4">
                  <StatCard
                    icon={<BookMarked className="w-5 h-5" />}
                    label="Books Placed"
                    value={`${booksPlaced}/5`}
                    color="sky"
                    progress={booksPlaced / 5}
                  />
                  <StatCard
                    icon={<Timer className="w-5 h-5" />}
                    label="Total Study"
                    value={formatDuration(totalStudyTime)}
                    color="violet"
                  />
                  <StatCard
                    icon={<Smile className="w-5 h-5" />}
                    label="Mood"
                    value={emotion}
                    color="amber"
                  />
                  <StatCard
                    icon={<Clock className="w-5 h-5" />}
                    label="Device Time"
                    value={status?.time ? new Date(status.time).toLocaleTimeString('en-IN', { hour: '2-digit', minute: '2-digit' }) : '--:--'}
                    color="emerald"
                  />
                </div>

                {/* Book Cards Grid */}
                <div className="grid grid-cols-5 gap-2">
                  {books.map((book) => (
                    <BookCard
                      key={book.name}
                      name={book.name}
                      present={book.present}
                      totalSeconds={book.totalSeconds}
                    />
                  ))}
                </div>
              </div>
            </div>

            {/* Charts Row 1 */}
            <div className="grid grid-cols-1 lg:grid-cols-2 gap-4">
              <ReadingProgressChart data={timeline} />
              <SubjectBarChart books={books} />
            </div>

            {/* Charts Row 2 */}
            <div className="grid grid-cols-1 lg:grid-cols-2 gap-4">
              <TimeDistributionPie books={books} />
              <EmotionTimeline data={timeline} />
            </div>

            {/* Session Log */}
            <div className="bg-slate-800/60 backdrop-blur rounded-2xl p-4 border border-slate-700/50">
              <h3 className="text-white font-semibold mb-3 text-sm flex items-center gap-2">
                <span className="w-2 h-2 rounded-full bg-rose-400" />
                Emotion Timeline & System Info
              </h3>
              <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
                {/* Emotion Flow */}
                <div>
                  <p className="text-xs text-slate-400 mb-2 font-medium">Emotion Behavior</p>
                  <div className="space-y-2 text-xs text-slate-300">
                    <div className="flex items-start gap-2">
                      <span className="w-1.5 h-1.5 rounded-full bg-blue-400 mt-1.5 shrink-0" />
                      <span><strong className="text-blue-300">Boot/Default:</strong> Calm smile face stays for first 5 minutes</span>
                    </div>
                    <div className="flex items-start gap-2">
                      <span className="w-1.5 h-1.5 rounded-full bg-emerald-400 mt-1.5 shrink-0" />
                      <span><strong className="text-emerald-300">Books Placed:</strong> Subject announced, expression stays calm → progressively happier</span>
                    </div>
                    <div className="flex items-start gap-2">
                      <span className="w-1.5 h-1.5 rounded-full bg-amber-400 mt-1.5 shrink-0" />
                      <span><strong className="text-amber-300">All 5 Placed:</strong> 5 min calm hold, then degrades every 5 min (5 stages)</span>
                    </div>
                    <div className="flex items-start gap-2">
                      <span className="w-1.5 h-1.5 rounded-full bg-rose-400 mt-1.5 shrink-0" />
                      <span><strong className="text-rose-300">Degradation:</strong> Calm → Neutral → Neutral → Sad → Very Sad → Reminder</span>
                    </div>
                    <div className="flex items-start gap-2">
                      <span className="w-1.5 h-1.5 rounded-full bg-green-400 mt-1.5 shrink-0" />
                      <span><strong className="text-green-300">Book Picked:</strong> Emotion upgrades back up based on book count</span>
                    </div>
                  </div>
                </div>

                {/* System Info */}
                <div>
                  <p className="text-xs text-slate-400 mb-2 font-medium">System Info</p>
                  <div className="space-y-1.5 text-xs text-slate-300">
                    <InfoRow label="Status" value={connected ? (demoMode ? 'Demo Mode' : 'Connected') : 'Offline'} color={connected ? 'text-emerald-400' : 'text-red-400'} />
                    <InfoRow label="IP Address" value={ipAddress || 'N/A'} />
                    <InfoRow label="Uptime" value={status?.uptime ? formatDuration(status.uptime) : 'N/A'} />
                    <InfoRow label="Data Points" value={`${timeline.length}`} />
                    <InfoRow label="Mode" value={demoMode ? 'Simulation' : 'Live ESP32'} />
                    <InfoRow label="Protocol" value={demoMode ? 'Local' : 'WebSocket + REST'} />
                  </div>
                </div>
              </div>
            </div>
          </div>
        )}
      </div>
    </div>
  );
}

// ─── Stat Card ───
function StatCard({ icon, label, value, color, progress }: {
  icon: React.ReactNode;
  label: string;
  value: string;
  color: string;
  progress?: number;
}) {
  const bgMap: Record<string, string> = {
    sky: 'from-sky-500/20 to-sky-600/10 border-sky-500/30',
    violet: 'from-violet-500/20 to-violet-600/10 border-violet-500/30',
    amber: 'from-amber-500/20 to-amber-600/10 border-amber-500/30',
    emerald: 'from-emerald-500/20 to-emerald-600/10 border-emerald-500/30',
  };
  const iconColorMap: Record<string, string> = {
    sky: 'text-sky-400',
    violet: 'text-violet-400',
    amber: 'text-amber-400',
    emerald: 'text-emerald-400',
  };

  return (
    <div className={`bg-gradient-to-br ${bgMap[color]} border rounded-2xl p-3 relative overflow-hidden`}>
      <div className={`${iconColorMap[color]} mb-1`}>{icon}</div>
      <p className="text-xs text-slate-400">{label}</p>
      <p className="text-lg font-bold text-white mt-0.5">{value}</p>
      {progress !== undefined && (
        <div className="mt-2 h-1 bg-slate-700 rounded-full overflow-hidden">
          <div
            className="h-full bg-sky-400 rounded-full transition-all duration-500"
            style={{ width: `${progress * 100}%` }}
          />
        </div>
      )}
    </div>
  );
}

// ─── Book Card ───
function BookCard({ name, present, totalSeconds }: {
  name: string;
  present: boolean;
  totalSeconds: number;
}) {
  return (
    <div className={`rounded-xl p-3 border transition-all duration-300 ${
      present
        ? `bg-gradient-to-br ${BOOK_COLORS[name]} border-transparent shadow-lg`
        : 'bg-slate-800/40 border-slate-700/50'
    }`}>
      <div className="text-center">
        <span className="text-xl">{BOOK_ICONS[name]}</span>
        <p className={`text-xs font-semibold mt-1 ${present ? 'text-white' : 'text-slate-300'}`}>
          {name}
        </p>
        <div className="flex items-center justify-center gap-1 mt-1">
          {present ? (
            <CheckCircle2 className="w-3 h-3 text-white" />
          ) : (
            <XCircle className="w-3 h-3 text-slate-500" />
          )}
          <span className={`text-[10px] ${present ? 'text-white/80' : 'text-slate-500'}`}>
            {present ? 'ON' : 'OFF'}
          </span>
        </div>
        <p className={`text-[10px] mt-1 ${present ? 'text-white/70' : 'text-slate-500'}`}>
          {formatDuration(totalSeconds)}
        </p>
      </div>
    </div>
  );
}

// ─── Info Row ───
function InfoRow({ label, value, color }: { label: string; value: string; color?: string }) {
  return (
    <div className="flex justify-between items-center py-1 px-2 bg-slate-900/40 rounded-lg">
      <span className="text-slate-500">{label}</span>
      <span className={color || 'text-slate-200'}>{value}</span>
    </div>
  );
}
