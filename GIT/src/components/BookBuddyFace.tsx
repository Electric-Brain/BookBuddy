import { useState, useEffect, useCallback } from 'react';

interface Props {
  emotion: string;
  booksPlaced: number;
}

interface FaceParams {
  eyeOpenRatio: number;   // 0 = closed, 1 = fully open
  pupilOffsetY: number;   // vertical offset for pupil (sad = down)
  mouthCurve: number;     // -1 = deep frown, 0 = flat, 1 = big smile
  mouthOpen: number;      // 0 = closed, 1 = wide open (for proud)
  cheekOpacity: number;   // blush intensity
  bgFrom: string;
  bgTo: string;
  eyebrowAngle: number;   // 0 = normal, negative = sad, positive = happy
}

const EMOTION_MAP: Record<string, FaceParams> = {
  Neutral: {
    eyeOpenRatio: 1, pupilOffsetY: 0, mouthCurve: 0, mouthOpen: 0,
    cheekOpacity: 0.15, bgFrom: '#87CEEB', bgTo: '#5DADE2', eyebrowAngle: 0,
  },
  Calm: {
    eyeOpenRatio: 0.9, pupilOffsetY: 0, mouthCurve: 0.3, mouthOpen: 0,
    cheekOpacity: 0.35, bgFrom: '#87CEEB', bgTo: '#3498DB', eyebrowAngle: 2,
  },
  Happy: {
    eyeOpenRatio: 0.2, pupilOffsetY: 0, mouthCurve: 0.7, mouthOpen: 0,
    cheekOpacity: 0.6, bgFrom: '#5DADE2', bgTo: '#2E86C1', eyebrowAngle: 5,
  },
  Proud: {
    eyeOpenRatio: 0, pupilOffsetY: 0, mouthCurve: 1, mouthOpen: 1,
    cheekOpacity: 0.85, bgFrom: '#F4D03F', bgTo: '#F39C12', eyebrowAngle: 6,
  },
  'Very Proud': {
    eyeOpenRatio: 0, pupilOffsetY: 0, mouthCurve: 1, mouthOpen: 1,
    cheekOpacity: 1, bgFrom: '#F9E547', bgTo: '#E67E22', eyebrowAngle: 8,
  },
  Sad: {
    eyeOpenRatio: 1, pupilOffsetY: 4, mouthCurve: -0.4, mouthOpen: 0,
    cheekOpacity: 0, bgFrom: '#AAB7C4', bgTo: '#85929E', eyebrowAngle: -5,
  },
  'Very Sad': {
    eyeOpenRatio: 1, pupilOffsetY: 6, mouthCurve: -0.7, mouthOpen: 0.3,
    cheekOpacity: 0, bgFrom: '#85929E', bgTo: '#707B7C', eyebrowAngle: -8,
  },
  Reminder: {
    eyeOpenRatio: 1, pupilOffsetY: 5, mouthCurve: -0.5, mouthOpen: 0.15,
    cheekOpacity: 0, bgFrom: '#95A5A6', bgTo: '#7F8C8D', eyebrowAngle: -6,
  },
};

export function BookBuddyFace({ emotion, booksPlaced }: Props) {
  const [blinking, setBlinking] = useState(false);
  const [animParams, setAnimParams] = useState<FaceParams>(EMOTION_MAP.Calm);

  const targetParams = EMOTION_MAP[emotion] || EMOTION_MAP.Calm;

  // Smooth animation interpolation
  useEffect(() => {
    let frame: number;
    const animate = () => {
      setAnimParams(prev => ({
        eyeOpenRatio: lerp(prev.eyeOpenRatio, targetParams.eyeOpenRatio, 0.08),
        pupilOffsetY: lerp(prev.pupilOffsetY, targetParams.pupilOffsetY, 0.08),
        mouthCurve: lerp(prev.mouthCurve, targetParams.mouthCurve, 0.06),
        mouthOpen: lerp(prev.mouthOpen, targetParams.mouthOpen, 0.06),
        cheekOpacity: lerp(prev.cheekOpacity, targetParams.cheekOpacity, 0.05),
        eyebrowAngle: lerp(prev.eyebrowAngle, targetParams.eyebrowAngle, 0.07),
        bgFrom: targetParams.bgFrom,
        bgTo: targetParams.bgTo,
      }));
      frame = requestAnimationFrame(animate);
    };
    frame = requestAnimationFrame(animate);
    return () => cancelAnimationFrame(frame);
  }, [targetParams]);

  // Blinking
  const doBlink = useCallback(() => {
    if (animParams.eyeOpenRatio > 0.3) {
      setBlinking(true);
      setTimeout(() => setBlinking(false), 120);
    }
  }, [animParams.eyeOpenRatio]);

  useEffect(() => {
    const interval = setInterval(doBlink, 2800 + Math.random() * 2500);
    return () => clearInterval(interval);
  }, [doBlink]);

  const eyeOpen = blinking ? 0 : animParams.eyeOpenRatio;

  return (
    <div className="relative w-full max-w-[260px] mx-auto">
      <svg viewBox="0 0 300 300" className="w-full h-full drop-shadow-2xl">
        <defs>
          <linearGradient id="faceBg" x1="0" y1="0" x2="1" y2="1">
            <stop offset="0%" stopColor={animParams.bgFrom} />
            <stop offset="100%" stopColor={animParams.bgTo} />
          </linearGradient>
          <linearGradient id="eyeWhite" x1="0" y1="0" x2="0" y2="1">
            <stop offset="0%" stopColor="#FFFFFF" />
            <stop offset="100%" stopColor="#F0F0F5" />
          </linearGradient>
          <radialGradient id="cheekGrad" cx="50%" cy="50%" r="50%">
            <stop offset="0%" stopColor="#FFB6C1" stopOpacity="0.8" />
            <stop offset="100%" stopColor="#FFB6C1" stopOpacity="0" />
          </radialGradient>
          <filter id="softShadow" x="-10%" y="-10%" width="120%" height="120%">
            <feDropShadow dx="0" dy="4" stdDeviation="6" floodColor="#00000022" />
          </filter>
          <filter id="innerGlow">
            <feGaussianBlur in="SourceAlpha" stdDeviation="3" result="blur" />
            <feComposite in="SourceGraphic" in2="blur" operator="over" />
          </filter>
        </defs>

        {/* Face background - rounded rectangle */}
        <rect x="15" y="15" width="270" height="270" rx="60" ry="60"
          fill="url(#faceBg)" filter="url(#softShadow)" />

        {/* Subtle face shine */}
        <ellipse cx="120" cy="70" rx="80" ry="40" fill="white" opacity="0.12" />

        {/* Eyebrows */}
        <g>
          {/* Left eyebrow */}
          <line
            x1={85} y1={95 - animParams.eyebrowAngle}
            x2={120} y2={95 + Math.abs(animParams.eyebrowAngle) * 0.3}
            stroke="#2C3E50" strokeWidth="4" strokeLinecap="round"
            opacity={Math.abs(animParams.eyebrowAngle) > 2 ? 0.6 : 0.3}
          />
          {/* Right eyebrow */}
          <line
            x1={180} y1={95 + Math.abs(animParams.eyebrowAngle) * 0.3}
            x2={215} y2={95 - animParams.eyebrowAngle}
            stroke="#2C3E50" strokeWidth="4" strokeLinecap="round"
            opacity={Math.abs(animParams.eyebrowAngle) > 2 ? 0.6 : 0.3}
          />
        </g>

        {/* Left Eye */}
        <g transform="translate(105, 130)">
          {eyeOpen < 0.15 ? (
            // Closed eye - curved line
            <path
              d={`M-25,0 Q0,${8 + animParams.mouthCurve * 3} 25,0`}
              stroke="#2C3E50" strokeWidth="4.5" fill="none" strokeLinecap="round"
            />
          ) : (
            <>
              {/* Eye white */}
              <ellipse cx="0" cy="0" rx="28" ry={22 * eyeOpen}
                fill="url(#eyeWhite)" stroke="#D5D8DC" strokeWidth="1.5" />
              {/* Iris */}
              <ellipse cx="0" cy={animParams.pupilOffsetY}
                rx={13} ry={Math.min(13, 20 * eyeOpen)}
                fill="#2C3E50" />
              {/* Pupil */}
              <ellipse cx="0" cy={animParams.pupilOffsetY}
                rx={7} ry={Math.min(7, 12 * eyeOpen)}
                fill="#1A1A2E" />
              {/* Highlight */}
              <ellipse cx="-5" cy={-5 + animParams.pupilOffsetY}
                rx="5" ry={Math.min(5, 5 * eyeOpen)}
                fill="white" opacity="0.9" />
              <ellipse cx="4" cy={-8 + animParams.pupilOffsetY}
                rx="2.5" ry={Math.min(2.5, 2.5 * eyeOpen)}
                fill="white" opacity="0.5" />
            </>
          )}
        </g>

        {/* Right Eye */}
        <g transform="translate(195, 130)">
          {eyeOpen < 0.15 ? (
            <path
              d={`M-25,0 Q0,${8 + animParams.mouthCurve * 3} 25,0`}
              stroke="#2C3E50" strokeWidth="4.5" fill="none" strokeLinecap="round"
            />
          ) : (
            <>
              <ellipse cx="0" cy="0" rx="28" ry={22 * eyeOpen}
                fill="url(#eyeWhite)" stroke="#D5D8DC" strokeWidth="1.5" />
              <ellipse cx="0" cy={animParams.pupilOffsetY}
                rx={13} ry={Math.min(13, 20 * eyeOpen)}
                fill="#2C3E50" />
              <ellipse cx="0" cy={animParams.pupilOffsetY}
                rx={7} ry={Math.min(7, 12 * eyeOpen)}
                fill="#1A1A2E" />
              <ellipse cx="-5" cy={-5 + animParams.pupilOffsetY}
                rx="5" ry={Math.min(5, 5 * eyeOpen)}
                fill="white" opacity="0.9" />
              <ellipse cx="4" cy={-8 + animParams.pupilOffsetY}
                rx="2.5" ry={Math.min(2.5, 2.5 * eyeOpen)}
                fill="white" opacity="0.5" />
            </>
          )}
        </g>

        {/* Cheeks (blush circles) */}
        <circle cx="60" cy="165" r="28" fill="url(#cheekGrad)"
          opacity={animParams.cheekOpacity} />
        <circle cx="240" cy="165" r="28" fill="url(#cheekGrad)"
          opacity={animParams.cheekOpacity} />

        {/* Mouth */}
        <g transform="translate(150, 195)">
          <MouthShape curve={animParams.mouthCurve} openAmount={animParams.mouthOpen} />
        </g>

        {/* Book indicator dots */}
        <g transform="translate(150, 270)">
          {[0, 1, 2, 3, 4].map(i => (
            <g key={i}>
              <circle
                cx={(i - 2) * 18}
                cy="0"
                r="5.5"
                fill={i < booksPlaced ? '#2ECC71' : 'rgba(255,255,255,0.2)'}
                stroke={i < booksPlaced ? '#27AE60' : 'rgba(255,255,255,0.1)'}
                strokeWidth="1"
              />
              {i < booksPlaced && (
                <circle cx={(i - 2) * 18} cy="-2" r="2" fill="white" opacity="0.4" />
              )}
            </g>
          ))}
        </g>
      </svg>

      {/* Emotion label below face */}
      <div className="text-center mt-1">
        <span className="text-xs font-medium px-3 py-1 rounded-full"
          style={{
            background: `linear-gradient(135deg, ${animParams.bgFrom}44, ${animParams.bgTo}44)`,
            color: animParams.bgFrom,
          }}>
          {emotion}
        </span>
      </div>
    </div>
  );
}

// Mouth sub-component
function MouthShape({ curve, openAmount }: { curve: number; openAmount: number }) {
  const w = 40; // half-width of mouth

  if (Math.abs(curve) < 0.05 && openAmount < 0.05) {
    // Neutral: straight line
    return (
      <line x1={-w} y1="0" x2={w} y2="0" stroke="#2C3E50" strokeWidth="4.5" strokeLinecap="round" />
    );
  }

  if (curve >= 0) {
    // Smile
    const curveDepth = curve * 28;
    const path = `M${-w},0 Q0,${curveDepth} ${w},0`;

    if (openAmount > 0.1) {
      // Open mouth smile
      const openH = openAmount * 18;
      const openPath = `M${-w * 0.7},0 Q0,${curveDepth} ${w * 0.7},0 Q0,${curveDepth + openH} ${-w * 0.7},0 Z`;
      return (
        <g>
          <path d={openPath} fill="#2C3E50" />
          {/* Tongue */}
          <ellipse cx="0" cy={curveDepth * 0.6 + openH * 0.4}
            rx={w * 0.35} ry={openH * 0.45}
            fill="#E74C3C" opacity="0.7" />
          {/* Teeth hint */}
          <rect x={-w * 0.5} y={-1} width={w} height={Math.min(6, openH * 0.5)}
            rx="2" fill="white" opacity="0.85" />
        </g>
      );
    }

    return <path d={path} stroke="#2C3E50" strokeWidth="4.5" fill="none" strokeLinecap="round" />;
  } else {
    // Frown
    const curveRise = Math.abs(curve) * 22;
    const path = `M${-w},0 Q0,${-curveRise} ${w},0`;

    if (openAmount > 0.1) {
      const openH = openAmount * 12;
      const frowPath = `M${-w * 0.6},0 Q0,${-curveRise} ${w * 0.6},0 Q0,${-curveRise - openH} ${-w * 0.6},0 Z`;
      return <path d={frowPath} fill="#2C3E50" />;
    }

    return <path d={path} stroke="#2C3E50" strokeWidth="4.5" fill="none" strokeLinecap="round" />;
  }
}

function lerp(current: number, target: number, speed: number): number {
  const diff = target - current;
  if (Math.abs(diff) < 0.005) return target;
  return current + diff * speed;
}
