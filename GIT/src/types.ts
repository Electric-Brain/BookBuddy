export interface BookData {
  name: string;
  present: boolean;
  totalSeconds: number;
}

export interface DeviceStatus {
  status: string;
  uptime: number;
  emotion: string;
  booksPlaced: number;
  time: string;
  books: BookData[];
  totalStudySeconds: number;
}

export interface ReadingSession {
  book: string;
  startTime: number;
  endTime: number | null;
  duration: number;
}

export interface TimelineEntry {
  timestamp: number;
  timeLabel: string;
  English: number;
  Hindi: number;
  Maths: number;
  Marathi: number;
  Science: number;
  total: number;
  emotion: string;
  booksPlaced: number;
}
