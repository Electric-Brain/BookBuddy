import { useState, useEffect, useRef } from 'react';

interface Props {
  emotion: string;
  booksPlaced: number;
}

interface FaceState {
  eyeOpen: number;
  pupilY: number;
  mouthCurve: number;
  mouthOpen: number;
  blush: number;
  browAngle: number;
}

const EMOTIONS: Record<string, FaceState> = {
  Neutral:      { eyeOpen: 1,    pupilY: 0, mouthCurve: 0,    mouthOpen: 0,   blush: 0.1,  browAngle: 0 },
  Calm:         { eyeOpen: 0.85, pupilY: 0, mouthCurve: 0.35, mouthOpen: 0,   blush: 0.3,  browAngle: 2 },
  Happy:        { eyeOpen: 0.15, pupilY: 0, mouthCurve: 0.7,  mouthOpen: 0,   blush: 0.55, browAngle: 5 },
  Proud:        { eyeOpen: 0,    pupilY: 0, mouthCurve: 1,    mouthOpen: 0.8, blush: 0.8,  browAngle: 6 },
  'Very Proud': { eyeOpen: 0,    pupilY: 0, mouthCurve: 1,    mouthOpen: 1,   blush: 1,    browAngle: 8 },
  Sad:          { eyeOpen: 1,    pupilY: 5, mouthCurve: -0.4, mouthOpen: 0,   blush: 0,    browAngle: -5 },
  'Very Sad':   { eyeOpen: 1,    pupilY: 7, mouthCurve: -0.7, mouthOpen: 0.2, blush: 0,    browAngle: -8 },
  Reminder:     { eyeOpen: 1,    pupilY: 5, mouthCurve: -0.5, mouthOpen: 0.1, blush: 0,    browAngle: -6 },
};

const BG_COLORS: Record<string, [string, string]> = {
  Neutral:      ['#87CEEB', '#5DADE2'],
  Calm:         ['#87CEEB', '#3498DB'],
  Happy:        ['#5DADE2', '#2E86C1'],
  Proud:        ['#F4D03F', '#F39C12'],
  'Very Proud': ['#F9E547', '#E67E22'],
  Sad:          ['#AAB7C4', '#85929E'],
  'Very Sad':   ['#85929E', '#707B7C'],
  Reminder:     ['#95A5A6', '#7F8C8D'],
};

function lerp(a: number, b: number, t: number): number {
  const d = b - a;
  return Math.abs(d) < 0.003 ? b : a + d * t;
}

export function BookBuddyFace({ emotion, booksPlaced }: Props) {
  const target = EMOTIONS[emotion] || EMOTIONS.Calm;
  const colors = BG_COLORS[emotion] || BG_COLORS.Calm;

  const [state, setState] = useState<FaceState>({ ...target });
  const [blinking, setBlinking] = useState(false);
  const frameRef = useRef(0);
  const timerRef = useRef<number | null>(null);

  // Smooth animation loop
  useEffect(() => {
    let active = true;
    const tick = () => {
      if (!active) return;
      setState(prev => ({
        eyeOpen:    lerp(prev.eyeOpen, target.eyeOpen, 0.07),
        pupilY:     lerp(prev.pupilY, target.pupilY, 0.07),
        mouthCurve: lerp(prev.mouthCurve, target.mouthCurve, 0.05),
        mouthOpen:  lerp(prev.mouthOpen, target.mouthOpen, 0.05),
        blush:      lerp(prev.blush, target.blush, 0.04),
        browAngle:  lerp(prev.browAngle, target.browAngle, 0.06),
      }));
      frameRef.current = requestAnimationFrame(tick);
    };
    frameRef.current = requestAnimationFrame(tick);
    return () => {
      active = false;
      cancelAnimationFrame(frameRef.current);
    };
  }, [target]);

  // Blinking effect
  const canBlink = state.eyeOpen > 0.3;
  useEffect(() => {
    const doBlink = () => {
      if (canBlink) {
        setBlinking(true);
        window.setTimeout(() => setBlinking(false), 130);
      }
      timerRef.current = window.setTimeout(doBlink, 2500 + Math.random() * 3000);
    };
    timerRef.current = window.setTimeout(doBlink, 2000 + Math.random() * 2000);
    return () => {
      if (timerRef.current !== null) window.clearTimeout(timerRef.current);
    };
  }, [canBlink]);

  const eye = blinking ? 0 : state.eyeOpen;

  const CX = 150;
  const CY = 140;
  const eyeSpacing = 48;
  const eyeY = CY - 12;

  return (
    <div className="relative w-full max-w-[240px] mx-auto select-none">
      <svg viewBox="0 0 300 300" className="w-full h-full" style={{ filter: 'drop-shadow(0 8px 24px rgba(0,0,0,0.25))' }}>
        <defs>
          <linearGradient id="faceBg" x1="0" y1="0" x2="1" y2="1">
            <stop offset="0%" stopColor={colors[0]} />
            <stop offset="100%" stopColor={colors[1]} />
          </linearGradient>
          <linearGradient id="eyeWhiteGrad" x1="0" y1="0" x2="0" y2="1">
            <stop offset="0%" stopColor="#fff" />
            <stop offset="100%" stopColor="#eef0f5" />
          </linearGradient>
          <radialGradient id="blushGradL" cx="50%" cy="50%" r="50%">
            <stop offset="0%" stopColor="#FF9EAE" stopOpacity={0.75 * state.blush} />
            <stop offset="100%" stopColor="#FF9EAE" stopOpacity="0" />
          </radialGradient>
          <radialGradient id="blushGradR" cx="50%" cy="50%" r="50%">
            <stop offset="0%" stopColor="#FF9EAE" stopOpacity={0.75 * state.blush} />
            <stop offset="100%" stopColor="#FF9EAE" stopOpacity="0" />
          </radialGradient>
        </defs>

        {/* Face background */}
        <rect x="18" y="18" width="264" height="264" rx="56" fill="url(#faceBg)" />
        <ellipse cx="115" cy="65" rx="70" ry="32" fill="white" opacity="0.1" />

        {/* Eyebrows */}
        {Math.abs(state.browAngle) > 1.5 && (
          <>
            <line
              x1={CX - eyeSpacing - 18} y1={eyeY - 28 - state.browAngle * 0.8}
              x2={CX - eyeSpacing + 14} y2={eyeY - 28 + Math.abs(state.browAngle) * 0.2}
              stroke="#2C3E50" strokeWidth="3.5" strokeLinecap="round" opacity="0.55"
            />
            <line
              x1={CX + eyeSpacing - 14} y1={eyeY - 28 + Math.abs(state.browAngle) * 0.2}
              x2={CX + eyeSpacing + 18} y2={eyeY - 28 - state.browAngle * 0.8}
              stroke="#2C3E50" strokeWidth="3.5" strokeLinecap="round" opacity="0.55"
            />
          </>
        )}

        {/* Left Eye */}
        <g transform={`translate(${CX - eyeSpacing}, ${eyeY})`}>
          <Eye openRatio={eye} pupilY={state.pupilY} mouthCurve={state.mouthCurve} />
        </g>

        {/* Right Eye */}
        <g transform={`translate(${CX + eyeSpacing}, ${eyeY})`}>
          <Eye openRatio={eye} pupilY={state.pupilY} mouthCurve={state.mouthCurve} />
        </g>

        {/* Cheeks */}
        <circle cx={CX - 82} cy={CY + 18} r="22" fill="url(#blushGradL)" />
        <circle cx={CX + 82} cy={CY + 18} r="22" fill="url(#blushGradR)" />

        {/* Mouth */}
        <g transform={`translate(${CX}, ${CY + 48})`}>
          <Mouth curve={state.mouthCurve} open={state.mouthOpen} />
        </g>

        {/* Book indicator dots */}
        <g transform={`translate(${CX}, 268)`}>
          {[0, 1, 2, 3, 4].map(i => (
            <circle
              key={i}
              cx={(i - 2) * 16}
              cy="0"
              r="5"
              fill={i < booksPlaced ? '#2ECC71' : 'rgba(255,255,255,0.18)'}
              stroke={i < booksPlaced ? '#27AE60' : 'rgba(255,255,255,0.08)'}
              strokeWidth="0.8"
            />
          ))}
        </g>
      </svg>

      <div className="text-center mt-1.5">
        <span
          className="text-[11px] font-semibold px-3 py-0.5 rounded-full"
          style={{
            background: `linear-gradient(135deg, ${colors[0]}33, ${colors[1]}33)`,
            color: colors[1],
            border: `1px solid ${colors[1]}44`,
          }}
        >
          {emotion}
        </span>
      </div>
    </div>
  );
}

/* ─── Eye Component ─── */
function Eye({ openRatio, pupilY, mouthCurve }: { openRatio: number; pupilY: number; mouthCurve: number }) {
  if (openRatio < 0.12) {
    return (
      <path
        d={`M-20,0 Q0,${7 + mouthCurve * 2} 20,0`}
        stroke="#2C3E50"
        strokeWidth="4"
        fill="none"
        strokeLinecap="round"
      />
    );
  }

  const ry = 19 * openRatio;
  const irisRy = Math.min(11, 17 * openRatio);
  const pupilRy = Math.min(6, 10 * openRatio);
  const hlRy1 = Math.min(4, 4 * openRatio);
  const hlRy2 = Math.min(2, 2 * openRatio);
  const py = pupilY * 0.7;

  return (
    <>
      <ellipse cx="0" cy="0" rx="24" ry={ry} fill="url(#eyeWhiteGrad)" stroke="#D5D8DC" strokeWidth="1.2" />
      <ellipse cx="0" cy={py} rx="11" ry={irisRy} fill="#34495E" />
      <ellipse cx="0" cy={py} rx="6" ry={pupilRy} fill="#1A1A2E" />
      <ellipse cx="-4" cy={-4 + pupilY * 0.4} rx="4" ry={hlRy1} fill="white" opacity="0.92" />
      <ellipse cx="3" cy={-7 + pupilY * 0.3} rx="2" ry={hlRy2} fill="white" opacity="0.5" />
    </>
  );
}

/* ─── Mouth Component ─── */
function Mouth({ curve, open }: { curve: number; open: number }) {
  const W = 35;

  if (Math.abs(curve) < 0.04 && open < 0.04) {
    return (
      <line x1={-W} y1="0" x2={W} y2="0" stroke="#2C3E50" strokeWidth="4" strokeLinecap="round" />
    );
  }

  if (curve >= 0) {
    const depth = curve * 24;
    if (open > 0.08) {
      const oh = open * 16;
      return (
        <g>
          <path
            d={`M${-W * 0.75},0 Q0,${depth} ${W * 0.75},0 Q0,${depth + oh} ${-W * 0.75},0 Z`}
            fill="#2C3E50"
          />
          <rect x={-W * 0.5} y={-1} width={W} height={Math.min(5, oh * 0.4)} rx="1.5" fill="white" opacity="0.9" />
          <ellipse cx="0" cy={depth * 0.5 + oh * 0.45} rx={W * 0.3} ry={oh * 0.4} fill="#E74C3C" opacity="0.65" />
        </g>
      );
    }
    return (
      <path d={`M${-W},0 Q0,${depth} ${W},0`} stroke="#2C3E50" strokeWidth="4" fill="none" strokeLinecap="round" />
    );
  } else {
    const rise = Math.abs(curve) * 18;
    if (open > 0.08) {
      const oh = open * 10;
      return (
        <path
          d={`M${-W * 0.6},0 Q0,${-rise} ${W * 0.6},0 Q0,${-rise - oh} ${-W * 0.6},0 Z`}
          fill="#2C3E50"
        />
      );
    }
    return (
      <path d={`M${-W},0 Q0,${-rise} ${W},0`} stroke="#2C3E50" strokeWidth="4" fill="none" strokeLinecap="round" />
    );
  }
}
