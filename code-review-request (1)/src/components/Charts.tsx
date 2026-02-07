import {
  AreaChart, Area, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer,
  BarChart, Bar, PieChart, Pie, Cell, Legend,
  LineChart, Line,
} from 'recharts';
import type { TimelineEntry, BookData } from '../types';

const COLORS: Record<string, string> = {
  English: '#60A5FA',
  Hindi: '#F97316',
  Maths: '#A78BFA',
  Marathi: '#34D399',
  Science: '#FBBF24',
};

const PIE_COLORS = ['#60A5FA', '#F97316', '#A78BFA', '#34D399', '#FBBF24'];

function formatTime(seconds: number): string {
  if (seconds < 60) return `${seconds}s`;
  const m = Math.floor(seconds / 60);
  const s = seconds % 60;
  if (m < 60) return `${m}m ${s}s`;
  const h = Math.floor(m / 60);
  return `${h}h ${m % 60}m`;
}

// ─── Stacked Area Chart: Reading Progress Over Time ───
export function ReadingProgressChart({ data }: { data: TimelineEntry[] }) {
  if (data.length < 2) {
    return <EmptyChart text="Collecting data... Progress chart will appear shortly." />;
  }

  const sampled = sampleData(data, 50);

  return (
    <div className="bg-slate-800/60 backdrop-blur rounded-2xl p-4 border border-slate-700/50">
      <h3 className="text-white font-semibold mb-3 text-sm flex items-center gap-2">
        <span className="w-2 h-2 rounded-full bg-sky-400" />
        Reading Progress Over Time
      </h3>
      <ResponsiveContainer width="100%" height={260}>
        <AreaChart data={sampled}>
          <defs>
            {Object.entries(COLORS).map(([name, color]) => (
              <linearGradient key={name} id={`grad-${name}`} x1="0" y1="0" x2="0" y2="1">
                <stop offset="5%" stopColor={color} stopOpacity={0.4} />
                <stop offset="95%" stopColor={color} stopOpacity={0.05} />
              </linearGradient>
            ))}
          </defs>
          <CartesianGrid strokeDasharray="3 3" stroke="#334155" />
          <XAxis dataKey="timeLabel" stroke="#64748B" tick={{ fontSize: 10 }} interval="preserveStartEnd" />
          <YAxis stroke="#64748B" tick={{ fontSize: 10 }} tickFormatter={(v: number) => formatTime(v)} />
          <Tooltip
            contentStyle={{ backgroundColor: '#1E293B', border: '1px solid #475569', borderRadius: '12px', fontSize: 12 }}
            labelStyle={{ color: '#94A3B8' }}
            // eslint-disable-next-line @typescript-eslint/no-explicit-any
            formatter={(value: any, name: any) => [formatTime(Number(value || 0)), String(name)]}
          />
          {Object.entries(COLORS).map(([name, color]) => (
            <Area
              key={name}
              type="monotone"
              dataKey={name}
              stroke={color}
              fill={`url(#grad-${name})`}
              strokeWidth={2}
            />
          ))}
        </AreaChart>
      </ResponsiveContainer>
    </div>
  );
}

// ─── Bar Chart: Subject Comparison ───
export function SubjectBarChart({ books }: { books: BookData[] }) {
  const data = books.map((b) => ({
    name: b.name,
    minutes: Math.round(b.totalSeconds / 60 * 10) / 10,
    seconds: b.totalSeconds,
    present: b.present,
  }));

  return (
    <div className="bg-slate-800/60 backdrop-blur rounded-2xl p-4 border border-slate-700/50">
      <h3 className="text-white font-semibold mb-3 text-sm flex items-center gap-2">
        <span className="w-2 h-2 rounded-full bg-violet-400" />
        Study Time by Subject (minutes)
      </h3>
      <ResponsiveContainer width="100%" height={220}>
        <BarChart data={data}>
          <CartesianGrid strokeDasharray="3 3" stroke="#334155" />
          <XAxis dataKey="name" stroke="#64748B" tick={{ fontSize: 11 }} />
          <YAxis stroke="#64748B" tick={{ fontSize: 10 }} />
          <Tooltip
            contentStyle={{ backgroundColor: '#1E293B', border: '1px solid #475569', borderRadius: '12px', fontSize: 12 }}
            // eslint-disable-next-line @typescript-eslint/no-explicit-any
            formatter={(_value: any, _name: any, props: any) => [formatTime(Number(props?.payload?.seconds || 0)), 'Time']}
          />
          <Bar dataKey="minutes" radius={[8, 8, 0, 0]}>
            {data.map((entry, i) => (
              <Cell key={i} fill={COLORS[entry.name] || PIE_COLORS[i]} opacity={entry.present ? 1 : 0.5} />
            ))}
          </Bar>
        </BarChart>
      </ResponsiveContainer>
    </div>
  );
}

// ─── Pie Chart: Time Distribution ───
export function TimeDistributionPie({ books }: { books: BookData[] }) {
  const data = books
    .filter((b) => b.totalSeconds > 0)
    .map((b) => ({
      name: b.name,
      value: b.totalSeconds,
    }));

  if (data.length === 0) {
    return <EmptyChart text="No study time recorded yet. Place books to start tracking!" />;
  }

  return (
    <div className="bg-slate-800/60 backdrop-blur rounded-2xl p-4 border border-slate-700/50">
      <h3 className="text-white font-semibold mb-3 text-sm flex items-center gap-2">
        <span className="w-2 h-2 rounded-full bg-amber-400" />
        Time Distribution
      </h3>
      <ResponsiveContainer width="100%" height={220}>
        <PieChart>
          <Pie
            data={data}
            cx="50%"
            cy="50%"
            innerRadius={50}
            outerRadius={80}
            paddingAngle={3}
            dataKey="value"
          >
            {data.map((entry, i) => (
              <Cell key={i} fill={COLORS[entry.name] || PIE_COLORS[i]} />
            ))}
          </Pie>
          <Tooltip
            contentStyle={{ backgroundColor: '#1E293B', border: '1px solid #475569', borderRadius: '12px', fontSize: 12 }}
            // eslint-disable-next-line @typescript-eslint/no-explicit-any
            formatter={(value: any) => [formatTime(Number(value || 0)), 'Time']}
          />
          <Legend
            wrapperStyle={{ fontSize: 11, color: '#94A3B8' }}
          />
        </PieChart>
      </ResponsiveContainer>
    </div>
  );
}

// ─── Line Chart: Emotion/Books Placed Timeline ───
export function EmotionTimeline({ data }: { data: TimelineEntry[] }) {
  if (data.length < 2) {
    return <EmptyChart text="Emotion tracking will appear as data comes in..." />;
  }

  const sampled = sampleData(data, 50);

  return (
    <div className="bg-slate-800/60 backdrop-blur rounded-2xl p-4 border border-slate-700/50">
      <h3 className="text-white font-semibold mb-3 text-sm flex items-center gap-2">
        <span className="w-2 h-2 rounded-full bg-emerald-400" />
        Books Placed & Total Study Time
      </h3>
      <ResponsiveContainer width="100%" height={200}>
        <LineChart data={sampled}>
          <CartesianGrid strokeDasharray="3 3" stroke="#334155" />
          <XAxis dataKey="timeLabel" stroke="#64748B" tick={{ fontSize: 10 }} interval="preserveStartEnd" />
          <YAxis yAxisId="left" stroke="#64748B" tick={{ fontSize: 10 }} domain={[0, 5]} />
          <YAxis yAxisId="right" orientation="right" stroke="#64748B" tick={{ fontSize: 10 }} tickFormatter={(v: number) => formatTime(v)} />
          <Tooltip
            contentStyle={{ backgroundColor: '#1E293B', border: '1px solid #475569', borderRadius: '12px', fontSize: 12 }}
          />
          <Line yAxisId="left" type="stepAfter" dataKey="booksPlaced" stroke="#4ADE80" strokeWidth={2} dot={false} name="Books Placed" />
          <Line yAxisId="right" type="monotone" dataKey="total" stroke="#60A5FA" strokeWidth={2} dot={false} name="Total Study (s)" />
        </LineChart>
      </ResponsiveContainer>
    </div>
  );
}

function EmptyChart({ text }: { text: string }) {
  return (
    <div className="bg-slate-800/60 backdrop-blur rounded-2xl p-6 border border-slate-700/50 flex items-center justify-center min-h-[200px]">
      <p className="text-slate-400 text-sm text-center">{text}</p>
    </div>
  );
}

function sampleData(data: TimelineEntry[], maxPoints: number): TimelineEntry[] {
  if (data.length <= maxPoints) return data;
  const step = Math.ceil(data.length / maxPoints);
  return data.filter((_, i) => i % step === 0 || i === data.length - 1);
}
