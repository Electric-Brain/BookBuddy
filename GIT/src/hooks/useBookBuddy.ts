import { useState, useEffect, useCallback, useRef } from 'react';
import type { DeviceStatus, TimelineEntry, ReadingSession } from '../types';

const DEMO_BOOKS = ['English', 'Hindi', 'Maths', 'Marathi', 'Science'];

function generateDemoData(): DeviceStatus {
  const books = DEMO_BOOKS.map((name, i) => ({
    name,
    present: i < 3,
    totalSeconds: Math.floor(Math.random() * 3600) + 300,
  }));
  return {
    status: 'demo',
    uptime: Math.floor(Date.now() / 1000),
    emotion: 'Calm',
    booksPlaced: 3,
    time: new Date().toISOString(),
    books,
    totalStudySeconds: books.reduce((a, b) => a + b.totalSeconds, 0),
  };
}

export function useBookBuddy(ipAddress: string | null) {
  const [connected, setConnected] = useState(false);
  const [status, setStatus] = useState<DeviceStatus | null>(null);
  const [timeline, setTimeline] = useState<TimelineEntry[]>([]);
  const [sessions, setSessions] = useState<ReadingSession[]>([]);
  const [error, setError] = useState<string | null>(null);
  const [demoMode, setDemoMode] = useState(false);

  const wsRef = useRef<WebSocket | null>(null);
  const prevBooksRef = useRef<Record<string, boolean>>({});
  const sessionStartRef = useRef<Record<string, number>>({});
  const demoIntervalRef = useRef<ReturnType<typeof setInterval> | null>(null);
  const reconnectTimeoutRef = useRef<ReturnType<typeof setTimeout> | null>(null);

  const addTimelineEntry = useCallback((data: DeviceStatus) => {
    const entry: TimelineEntry = {
      timestamp: Date.now(),
      timeLabel: new Date().toLocaleTimeString('en-IN', { hour: '2-digit', minute: '2-digit', second: '2-digit' }),
      English: 0,
      Hindi: 0,
      Maths: 0,
      Marathi: 0,
      Science: 0,
      total: data.totalStudySeconds,
      emotion: data.emotion,
      booksPlaced: data.booksPlaced,
    };
    data.books.forEach((b) => {
      // eslint-disable-next-line @typescript-eslint/no-explicit-any
      (entry as any)[b.name] = b.totalSeconds;
    });
    setTimeline((prev) => {
      const next = [...prev, entry];
      if (next.length > 200) return next.slice(-200);
      return next;
    });
  }, []);

  const trackSessions = useCallback((data: DeviceStatus) => {
    const now = Date.now();
    data.books.forEach((book) => {
      const wasPresent = prevBooksRef.current[book.name] ?? false;
      if (book.present && !wasPresent) {
        sessionStartRef.current[book.name] = now;
      }
      if (!book.present && wasPresent) {
        const start = sessionStartRef.current[book.name] || now - book.totalSeconds * 1000;
        setSessions((prev) => [
          ...prev,
          {
            book: book.name,
            startTime: start,
            endTime: now,
            duration: Math.round((now - start) / 1000),
          },
        ]);
        delete sessionStartRef.current[book.name];
      }
      prevBooksRef.current[book.name] = book.present;
    });
  }, []);

  // Demo mode
  const startDemo = useCallback(() => {
    setDemoMode(true);
    setConnected(true);
    setError(null);
    
    let demoBooks = DEMO_BOOKS.map((name) => ({
      name,
      present: false,
      totalSeconds: 0,
    }));
    let demoEmotion = 'Calm';
    let tickCount = 0;

    const tick = () => {
      tickCount++;
      // Simulate placing/removing books over time
      const phase = Math.floor(tickCount / 5) % 12;
      
      demoBooks = demoBooks.map((b, i) => {
        let present = b.present;
        if (phase === 0 && i === 0) present = true;
        if (phase === 1 && i === 1) present = true;
        if (phase === 2 && i === 2) present = true;
        if (phase === 3 && i === 3) present = true;
        if (phase === 4 && i === 4) present = true;
        if (phase === 6 && i === 2) present = false;
        if (phase === 7 && i === 0) present = false;
        if (phase === 8 && i === 4) present = false;
        if (phase === 9 && i === 1) present = false;
        if (phase === 10 && i === 3) present = false;
        if (phase === 11) present = false;
        
        return {
          ...b,
          present,
          totalSeconds: b.totalSeconds + (present ? 2 : 0),
        };
      });

      const placedCount = demoBooks.filter(b => b.present).length;
      if (placedCount === 5) demoEmotion = 'Proud';
      else if (placedCount >= 3) demoEmotion = 'Happy';
      else if (placedCount >= 1) demoEmotion = 'Calm';
      else demoEmotion = 'Neutral';

      const data: DeviceStatus = {
        status: 'demo',
        uptime: tickCount * 2,
        emotion: demoEmotion,
        booksPlaced: placedCount,
        time: new Date().toISOString(),
        books: demoBooks,
        totalStudySeconds: demoBooks.reduce((a, b) => a + b.totalSeconds, 0),
      };
      setStatus(data);
      addTimelineEntry(data);
      trackSessions(data);
    };

    // Initial data
    const initialData = generateDemoData();
    setStatus(initialData);
    addTimelineEntry(initialData);

    demoIntervalRef.current = setInterval(tick, 2000);
  }, [addTimelineEntry, trackSessions]);

  const stopDemo = useCallback(() => {
    setDemoMode(false);
    if (demoIntervalRef.current) {
      clearInterval(demoIntervalRef.current);
      demoIntervalRef.current = null;
    }
  }, []);

  // WebSocket connection
  const connect = useCallback((ip: string) => {
    if (wsRef.current) {
      wsRef.current.close();
    }
    stopDemo();
    setError(null);

    const cleanIp = ip.replace(/^https?:\/\//, '').replace(/\/$/, '');

    try {
      const ws = new WebSocket(`ws://${cleanIp}/ws`);
      wsRef.current = ws;

      ws.onopen = () => {
        setConnected(true);
        setError(null);
        console.log('WebSocket connected to', cleanIp);
      };

      ws.onmessage = (event) => {
        try {
          const data: DeviceStatus = JSON.parse(event.data);
          setStatus(data);
          addTimelineEntry(data);
          trackSessions(data);
        } catch (e) {
          console.error('Parse error:', e);
        }
      };

      ws.onclose = () => {
        setConnected(false);
        // Try reconnecting after 3 seconds
        reconnectTimeoutRef.current = setTimeout(() => {
          if (ipAddress) connect(ipAddress);
        }, 3000);
      };

      ws.onerror = () => {
        setError(`Cannot connect to ${cleanIp}. Check if BookBuddy is on the same network.`);
        setConnected(false);
      };
    } catch {
      setError('Invalid IP address format');
    }
  }, [addTimelineEntry, trackSessions, ipAddress, stopDemo]);

  // HTTP fallback polling
  const pollHTTP = useCallback(async (ip: string) => {
    const cleanIp = ip.replace(/^https?:\/\//, '').replace(/\/$/, '');
    try {
      const res = await fetch(`http://${cleanIp}/api/status`);
      const data: DeviceStatus = await res.json();
      setStatus(data);
      setConnected(true);
      setError(null);
      addTimelineEntry(data);
      trackSessions(data);
    } catch {
      // silently fail, WS will handle reconnect
    }
  }, [addTimelineEntry, trackSessions]);

  const syncTime = useCallback(async (ip: string) => {
    const cleanIp = ip.replace(/^https?:\/\//, '').replace(/\/$/, '');
    try {
      const epoch = Math.floor(Date.now() / 1000);
      const res = await fetch(`http://${cleanIp}/api/synctime`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ epoch }),
      });
      const data = await res.json();
      return data;
    } catch {
      return { error: 'Failed to sync time' };
    }
  }, []);

  useEffect(() => {
    return () => {
      if (wsRef.current) wsRef.current.close();
      if (demoIntervalRef.current) clearInterval(demoIntervalRef.current);
      if (reconnectTimeoutRef.current) clearTimeout(reconnectTimeoutRef.current);
    };
  }, []);

  return {
    connected,
    status,
    timeline,
    sessions,
    error,
    demoMode,
    connect,
    pollHTTP,
    syncTime,
    startDemo,
    stopDemo,
  };
}
