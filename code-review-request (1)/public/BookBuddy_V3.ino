/*
 * ========================================
 * BookBuddy V3 - Complete ESP32 System
 * ========================================
 * Display: 480x320 LANDSCAPE (ILI9488)
 * 
 * FIXES APPLIED:
 * ✓ LANDSCAPE MODE (rotation=1, 480x320)
 * ✓ Face renders properly without distortion
 * ✓ BLACK text only, big and bold, centered
 * ✓ Full screen text for all messages
 * ✓ Flicker-free eye rendering
 * ✓ RTC time tracking with cumulative seconds
 * ✓ Progressive emotion logic:
 *     - Boot: Calm smile for 5 minutes
 *     - Books placed: subject announced, stays calm
 *     - All 5 placed: 5 min calm hold → 5 stage degradation
 *     - Book removed: emotion upgrades based on count
 * ✓ WebSocket support for remote dashboard
 * ✓ REST API for status and time sync
 * 
 * WIRING:
 *   - TFT: SPI (configured in TFT_eSPI User_Setup.h)
 *   - RTC DS3231: SDA=21, SCL=22
 *   - DFPlayer: RX=5, TX=18
 *   - Touch Sensor: GPIO4
 *   - Book Switches: GPIO 12,14,25,32,33
 * 
 * ========================================
 */

#include <Wire.h>
#include <RTClib.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <DFRobotDFPlayerMini.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// ========================== WIFI CREDENTIALS ==========================
const char* WIFI_SSID = "20";             // <-- CHANGE THIS
const char* WIFI_PASS = "12345678";        // <-- CHANGE THIS

// ========================== PIN DEFINITIONS ==========================
#define BTN_ENGLISH   12
#define BTN_HINDI     14
#define BTN_MATHS     25
#define BTN_MARATHI   32
#define BTN_SCIENCE   33
#define TOUCH_SENSOR  4
#define DFPLAYER_RX   5
#define DFPLAYER_TX   18

// ========================== OBJECTS ==========================
TFT_eSPI tft = TFT_eSPI();
RTC_DS3231 rtc;
HardwareSerial dfSerial(1);
DFRobotDFPlayerMini df;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// ========================== COLORS ==========================
#define BLACK       0x0000
#define WHITE       0xFFFF
#define BLUE        0x1E9F  // Background color

// ========================== CONSTANTS ==========================
const uint8_t NUM_BOOKS = 5;
const char* BOOK_NAMES[NUM_BOOKS] = {"English", "Hindi", "Maths", "Marathi", "Science"};
const uint8_t BOOK_PINS[NUM_BOOKS] = {BTN_ENGLISH, BTN_HINDI, BTN_MATHS, BTN_MARATHI, BTN_SCIENCE};

// Audio file indices on DFPlayer SD card
#define AUDIO_HEY               1
#define AUDIO_I_AM              2
#define AUDIO_BOOKBUDDY         3
#define AUDIO_YOUR_PARTNER      4
#define AUDIO_TIME_TO_STUDY     5
#define AUDIO_ENGLISH_PLACED    11
#define AUDIO_HINDI_PLACED      12
#define AUDIO_MATHS_PLACED      13
#define AUDIO_MARATHI_PLACED    14
#define AUDIO_SCIENCE_PLACED    15
#define AUDIO_ENGLISH_TIME      21
#define AUDIO_HINDI_TIME        22
#define AUDIO_MATHS_TIME        23
#define AUDIO_MARATHI_TIME      24
#define AUDIO_SCIENCE_TIME      25
#define AUDIO_ALL_READY         30
#define AUDIO_GOOD_JOB          31
#define AUDIO_WELL_DONE         32
#define AUDIO_UH_OH             33
#define AUDIO_CHIME             34
#define AUDIO_CLICK             35

// Emotions
enum Emotion {
  EMOTION_NEUTRAL,
  EMOTION_CALM,
  EMOTION_HAPPY,
  EMOTION_PROUD,
  EMOTION_VERY_PROUD,
  EMOTION_SAD,
  EMOTION_VERY_SAD,
  EMOTION_REMINDER
};

// ====================== TIMING CONSTANTS (EASILY CHANGEABLE) ======================
const uint32_t BOOT_CALM_TIME     = 300000;   // 5 minutes calm after boot
const uint32_t CALM_HOLD_TIME     = 300000;   // 5 minutes calm after all books placed
const uint32_t DEGRADE_INTERVAL   = 300000;   // 5 minutes per degrade stage
const uint8_t  DEGRADE_STAGES     = 5;        // 5 degradation stages
const uint32_t LONG_PRESS_TIME    = 2000;     // 2 seconds for long press

// ========================== BOOK STATE ==========================
struct BookState {
  bool present;
  bool wasPresent;
  uint32_t lastPlacedEpoch;
  uint32_t cumulativeSeconds;
  bool tracking;
};

BookState books[NUM_BOOKS];

// ========================== STATE VARIABLES ==========================
Emotion currentEmotion = EMOTION_CALM;
Emotion lastEmotion    = EMOTION_CALM;

uint32_t lastInteractionTime = 0;
uint8_t  currentBookCount    = 0;
uint8_t  lastBookCount       = 0;

bool touchPressed          = false;
uint32_t touchStartTime    = 0;
bool bootSequenceComplete  = false;
uint32_t bootCompleteTime  = 0;    // Track when boot finished
bool allBooksAnnouncedOnce = false;
bool dfPlayerReady         = false;
bool rtcReady              = false;
bool wifiConnected         = false;

// Emotion timing state
bool allBooksWerePlaced     = false;
uint32_t allBooksPlacedTime = 0;
bool calmHoldActive         = false;
uint32_t calmHoldStartTime  = 0;
bool degradeActive          = false;
uint32_t degradeStartTime   = 0;
uint8_t currentDegradeStage = 0;
bool reminderSpoken         = false;
bool inBootCalmPeriod       = true;  // True during initial 5 min calm

// Face overlay
bool overlayActive = false;

// Face parameters (current animated values)
int eyeOpen     = 28;
int pupilOffset = 0;
int mouthCurve  = 5;
bool eyesClosed = false;

// Face parameters (targets for smooth interpolation)
int t_eyeOpen     = 28;
int t_pupilOffset = 0;
int t_mouthCurve  = 5;
bool t_eyesClosed = false;

unsigned long lastFrame = 0;
unsigned long lastBlink = 0;

// Screen dimensions (480x320 LANDSCAPE)
int screenWidth  = 480;
int screenHeight = 320;
int centerX      = 240;
int centerY      = 160;

// Face geometry (scaled for 480x320 landscape)
const int EYE_SPACING    = 70;
const int EYE_Y_OFFSET   = -30;
const int EYE_RADIUS     = 34;
const int PUPIL_RADIUS   = 14;
const int CHEEK_RADIUS   = 20;
const int CHEEK_X_OFFSET = 110;
const int CHEEK_Y_OFFSET = 25;
const int MOUTH_Y        = 60;
const int MOUTH_WIDTH    = 55;

// WebSocket update throttle
uint32_t lastWsUpdate = 0;
const uint32_t WS_UPDATE_INTERVAL = 2000; // Send update every 2 seconds

// ========================== SETUP ==========================
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n========================================");
  Serial.println("    BOOKBUDDY V3 - LANDSCAPE MODE");
  Serial.println("========================================\n");
  
  // ─── Initialize TFT - LANDSCAPE MODE ───
  Serial.println("Initializing TFT Display...");
  tft.init();
  tft.setRotation(1);  // *** LANDSCAPE: 480x320 ***
  tft.fillScreen(BLUE);
  
  screenWidth  = tft.width();   // Should be 480
  screenHeight = tft.height();  // Should be 320
  centerX = screenWidth / 2;    // 240
  centerY = screenHeight / 2;   // 160
  
  Serial.print("Display: ");
  Serial.print(screenWidth);
  Serial.print(" x ");
  Serial.println(screenHeight);
  
  // ─── Initialize DFPlayer ───
  Serial.println("\nInitializing DFPlayer...");
  dfSerial.begin(9600, SERIAL_8N1, DFPLAYER_RX, DFPLAYER_TX);
  delay(1500);
  
  if (!df.begin(dfSerial)) {
    Serial.println("DFPlayer NOT detected!");
  } else {
    Serial.println("DFPlayer connected!");
    df.volume(22);
    dfPlayerReady = true;
  }
  
  // ─── Initialize RTC ───
  Serial.println("\nInitializing RTC...");
  Wire.begin(21, 22);
  
  if (!rtc.begin()) {
    Serial.println("RTC NOT found!");
  } else {
    Serial.println("RTC initialized");
    rtcReady = true;
    
    if (rtc.lostPower()) {
      Serial.println("RTC lost power, setting time...");
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    
    DateTime now = rtc.now();
    Serial.print("Current time: ");
    printTime(now);
  }
  
  // ─── Initialize Book Switches ───
  Serial.println("\nInitializing switches...");
  for (uint8_t i = 0; i < NUM_BOOKS; i++) {
    pinMode(BOOK_PINS[i], INPUT_PULLUP);
    books[i].present          = false;
    books[i].wasPresent       = false;
    books[i].lastPlacedEpoch  = 0;
    books[i].cumulativeSeconds = 0;
    books[i].tracking         = false;
    
    Serial.print(BOOK_NAMES[i]);
    Serial.print(" -> GPIO");
    Serial.println(BOOK_PINS[i]);
  }
  
  // Touch sensor
  pinMode(TOUCH_SENSOR, INPUT);
  Serial.print("\nTouch -> GPIO");
  Serial.println(TOUCH_SENSOR);
  
  // ─── Connect WiFi ───
  Serial.println("\nConnecting WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  showBigText("CONNECTING", "WIFI...", 6, 5);
  
  uint8_t attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println("\nWiFi Connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    
    // Show IP address on screen
    char ipBuf[30];
    sprintf(ipBuf, "%s", WiFi.localIP().toString().c_str());
    showBigText("WIFI OK", ipBuf, 6, 4);
    delay(2000);
  } else {
    Serial.println("\nWiFi failed");
    showBigText("WIFI", "OFFLINE", 6, 5);
    delay(1000);
  }
  
  // ─── Setup WebSocket & Web Server ───
  if (wifiConnected) {
    setupWebSocket();
    setupWebServer();
  }
  
  lastInteractionTime = millis();
  
  // ─── Show Intro ───
  Serial.println("\nStarting intro...");
  showIntro();
  
  bootSequenceComplete = true;
  bootCompleteTime = millis();
  inBootCalmPeriod = true;
  Serial.println("\nBookBuddy ready! (5 min boot calm period started)");
  
  // Start with calm smile
  currentEmotion = EMOTION_CALM;
  setEmotionTargets(EMOTION_CALM);
  eyeOpen     = t_eyeOpen;
  pupilOffset = t_pupilOffset;
  mouthCurve  = t_mouthCurve;
  eyesClosed  = t_eyesClosed;
  
  tft.fillScreen(BLUE);
  drawFaceComplete();
  overlayActive = false;
}

// ========================== HELPER: Print Time ==========================
void printTime(DateTime dt) {
  char buf[20];
  sprintf(buf, "%02d:%02d:%02d", dt.hour(), dt.minute(), dt.second());
  Serial.println(buf);
}

// ========================== HELPER: Get RTC Epoch ==========================
uint32_t getRTCEpoch() {
  if (!rtcReady) return millis() / 1000;
  DateTime now = rtc.now();
  return now.unixtime();
}

// ========================== HELPER: Format Duration ==========================
String formatDuration(uint32_t totalSeconds) {
  uint32_t hours = totalSeconds / 3600;
  uint32_t mins  = (totalSeconds % 3600) / 60;
  uint32_t secs  = totalSeconds % 60;
  
  char buf[20];
  if (hours > 0) {
    sprintf(buf, "%dh %dm", hours, mins);
  } else if (mins > 0) {
    sprintf(buf, "%dm %ds", mins, secs);
  } else {
    sprintf(buf, "%ds", secs);
  }
  return String(buf);
}

// ========================== HELPER: Get Book Total Time ==========================
uint32_t getBookTotalSeconds(uint8_t idx) {
  uint32_t total = books[idx].cumulativeSeconds;
  if (books[idx].tracking && books[idx].lastPlacedEpoch > 0) {
    uint32_t now = getRTCEpoch();
    if (now > books[idx].lastPlacedEpoch) {
      total += (now - books[idx].lastPlacedEpoch);
    }
  }
  return total;
}

// ========================== BIG TEXT HELPERS ==========================
void showBigText(const char* line1, const char* line2, uint8_t size1, uint8_t size2) {
  tft.fillScreen(BLUE);
  tft.setTextColor(BLACK, BLUE);
  
  // Line 1
  tft.setTextSize(size1);
  int16_t w1 = strlen(line1) * size1 * 6;
  int16_t x1 = (screenWidth - w1) / 2;
  int16_t y1 = centerY - 50;
  tft.setCursor(x1, y1);
  tft.print(line1);
  
  // Line 2
  if (line2 != nullptr && strlen(line2) > 0) {
    tft.setTextSize(size2);
    int16_t w2 = strlen(line2) * size2 * 6;
    int16_t x2 = (screenWidth - w2) / 2;
    int16_t y2 = centerY + 30;
    tft.setCursor(x2, y2);
    tft.print(line2);
  }
}

void showSingleBigText(const char* text, uint8_t size) {
  tft.fillScreen(BLUE);
  tft.setTextColor(BLACK, BLUE);
  tft.setTextSize(size);
  
  int16_t w = strlen(text) * size * 6;
  int16_t x = (screenWidth - w) / 2;
  int16_t y = centerY - (size * 4);
  tft.setCursor(x, y);
  tft.print(text);
}

// ========================== INTRO SEQUENCE ==========================
void showIntro() {
  overlayActive = true;
  
  // Slide 1: "HEYY!" - Big centered
  tft.fillScreen(BLUE);
  tft.setTextColor(BLACK, BLUE);
  tft.setTextSize(10);
  const char* hey = "HEYY!";
  int16_t w = strlen(hey) * 10 * 6;
  int16_t x = (screenWidth - w) / 2;
  int16_t y = centerY - 40;
  tft.setCursor(x, y);
  for (int i = 0; i < (int)strlen(hey); i++) {
    tft.print(hey[i]);
    delay(80);
  }
  if (dfPlayerReady) df.playMp3Folder(AUDIO_HEY);
  delay(1500);
  
  // Slide 2: "I AM"
  tft.fillScreen(BLUE);
  tft.setTextSize(10);
  const char* iam = "I AM";
  w = strlen(iam) * 10 * 6;
  x = (screenWidth - w) / 2;
  tft.setCursor(x, y);
  for (int i = 0; i < (int)strlen(iam); i++) {
    tft.print(iam[i]);
    delay(80);
  }
  if (dfPlayerReady) df.playMp3Folder(AUDIO_I_AM);
  delay(1200);
  
  // Slide 3: "BOOK BUDDY"
  showBigText("BOOK", "BUDDY", 8, 8);
  if (dfPlayerReady) df.playMp3Folder(AUDIO_BOOKBUDDY);
  delay(2000);
  
  // Slide 4: "YOUR STUDY PARTNER"
  showBigText("YOUR STUDY", "PARTNER", 5, 7);
  if (dfPlayerReady) df.playMp3Folder(AUDIO_YOUR_PARTNER);
  delay(2500);
  
  overlayActive = false;
}

// ========================== BOOK STATE MANAGEMENT ==========================
void updateBookStates() {
  uint32_t currentTime = millis();
  uint8_t bookCount = 0;
  
  for (uint8_t i = 0; i < NUM_BOOKS; i++) {
    bool currentState = !digitalRead(BOOK_PINS[i]);
    
    // Debounce
    if (currentState != books[i].present) {
      delay(50);
      currentState = !digitalRead(BOOK_PINS[i]);
    }
    
    // ─── State change detected ───
    if (currentState != books[i].wasPresent) {
      lastInteractionTime = currentTime;
      
      // End boot calm period on any interaction
      inBootCalmPeriod = false;
      
      // Reset degrade/calm hold on book change
      calmHoldActive    = false;
      degradeActive     = false;
      currentDegradeStage = 0;
      reminderSpoken    = false;
      
      if (currentState) {
        // ===== BOOK PLACED =====
        Serial.print(BOOK_NAMES[i]);
        Serial.println(" PLACED");
        
        // Start RTC tracking
        books[i].lastPlacedEpoch = getRTCEpoch();
        books[i].tracking = true;
        
        // Show announcement
        showBookAnnouncement(i, true);
        
        // Audio
        if (dfPlayerReady) {
          df.playMp3Folder(AUDIO_CHIME);
          delay(700);
          df.playMp3Folder(AUDIO_ENGLISH_PLACED + i);
        }
        
        delay(1500);
        
        // Restore face
        tft.fillScreen(BLUE);
        drawFaceComplete();
        
      } else {
        // ===== BOOK REMOVED =====
        Serial.print(BOOK_NAMES[i]);
        Serial.println(" REMOVED");
        
        // Stop tracking, accumulate time
        if (books[i].tracking && books[i].lastPlacedEpoch > 0) {
          uint32_t now = getRTCEpoch();
          if (now > books[i].lastPlacedEpoch) {
            uint32_t elapsed = now - books[i].lastPlacedEpoch;
            books[i].cumulativeSeconds += elapsed;
            Serial.print("Added ");
            Serial.print(elapsed);
            Serial.print("s, Total: ");
            Serial.print(books[i].cumulativeSeconds);
            Serial.println("s");
          }
        }
        books[i].tracking = false;
        books[i].lastPlacedEpoch = 0;
        
        // Show removal announcement with time
        showBookAnnouncement(i, false);
        
        // Audio
        if (dfPlayerReady) {
          delay(300);
          df.playMp3Folder(AUDIO_ENGLISH_TIME + i);
        }
        
        delay(1500);
        
        // Restore face
        tft.fillScreen(BLUE);
        drawFaceComplete();
      }
      
      books[i].wasPresent = currentState;
    }
    
    books[i].present = currentState;
    if (currentState) bookCount++;
  }
  
  lastBookCount    = currentBookCount;
  currentBookCount = bookCount;
  
  // ─── ALL 5 BOOKS PLACED - CELEBRATION ───
  if (currentBookCount == 5 && lastBookCount < 5 && !allBooksAnnouncedOnce) {
    Serial.println("ALL 5 BOOKS PLACED!");
    
    allBooksAnnouncedOnce = true;
    allBooksWerePlaced    = true;
    allBooksPlacedTime    = currentTime;
    
    // Show celebration
    showCelebration();
    
    if (dfPlayerReady) {
      delay(500);
      df.playMp3Folder(AUDIO_ALL_READY);
      delay(2000);
      df.playMp3Folder(AUDIO_WELL_DONE);
    }
    
    delay(2000);
    
    // Start calm hold period (5 min calm, then degrade)
    calmHoldActive    = true;
    calmHoldStartTime = millis();
    degradeActive     = false;
    currentDegradeStage = 0;
    reminderSpoken    = false;
    
    // Restore calm face
    tft.fillScreen(BLUE);
    currentEmotion = EMOTION_CALM;
    setEmotionTargets(EMOTION_CALM);
    eyeOpen     = t_eyeOpen;
    pupilOffset = t_pupilOffset;
    mouthCurve  = t_mouthCurve;
    eyesClosed  = t_eyesClosed;
    drawFaceComplete();
  }
  
  if (currentBookCount < 5) {
    allBooksAnnouncedOnce = false;
  }
  
  // ─── Send WebSocket update (throttled) ───
  if (wifiConnected && (millis() - lastWsUpdate >= WS_UPDATE_INTERVAL)) {
    lastWsUpdate = millis();
    sendWebSocketUpdate();
  }
}

// ========================== SHOW BOOK ANNOUNCEMENT ==========================
void showBookAnnouncement(uint8_t bookIdx, bool placed) {
  overlayActive = true;
  tft.fillScreen(BLUE);
  tft.setTextColor(BLACK, BLUE);
  
  // Book name - BIG
  String bookName = BOOK_NAMES[bookIdx];
  bookName.toUpperCase();
  
  tft.setTextSize(6);
  int16_t w = bookName.length() * 6 * 6;
  int16_t x = (screenWidth - w) / 2;
  tft.setCursor(x, centerY - 60);
  tft.print(bookName);
  
  // "PLACED" or "REMOVED"
  const char* action = placed ? "PLACED!" : "REMOVED";
  tft.setTextSize(6);
  w = strlen(action) * 6 * 6;
  x = (screenWidth - w) / 2;
  tft.setCursor(x, centerY + 20);
  tft.print(action);
  
  // Show study time on removal
  if (!placed) {
    String timeStr = formatDuration(getBookTotalSeconds(bookIdx));
    tft.setTextSize(3);
    String msg = "Time: " + timeStr;
    w = msg.length() * 3 * 6;
    x = (screenWidth - w) / 2;
    tft.setCursor(x, centerY + 90);
    tft.print(msg);
  }
  
  overlayActive = false;
}

// ========================== SHOW CELEBRATION ==========================
void showCelebration() {
  overlayActive = true;
  tft.fillScreen(BLUE);
  tft.setTextColor(BLACK, BLUE);
  
  tft.setTextSize(5);
  const char* l1 = "ALL BOOKS";
  int16_t w = strlen(l1) * 5 * 6;
  int16_t x = (screenWidth - w) / 2;
  tft.setCursor(x, centerY - 80);
  tft.print(l1);
  
  tft.setTextSize(7);
  const char* l2 = "PLACED!";
  w = strlen(l2) * 7 * 6;
  x = (screenWidth - w) / 2;
  tft.setCursor(x, centerY - 10);
  tft.print(l2);
  
  tft.setTextSize(4);
  const char* l3 = "WELL DONE!";
  w = strlen(l3) * 4 * 6;
  x = (screenWidth - w) / 2;
  tft.setCursor(x, centerY + 70);
  tft.print(l3);
  
  overlayActive = false;
}

// ========================== EMOTION CALCULATION ==========================
/*
 * EMOTION LOGIC (as discussed):
 * 
 * 1. BOOT: Calm smile for first 5 minutes (BOOT_CALM_TIME)
 * 2. Books placed/removed: progressive emotion based on count
 *    0 books = Neutral, 1-2 = Calm, 3-4 = Happy, 5 = Proud
 * 3. All 5 placed: CALM_HOLD_TIME (5 min) of calm
 *    Then degrades every DEGRADE_INTERVAL:
 *    Stage 0: Calm → 1: Neutral → 2: Neutral → 3: Sad → 4: Very Sad → 5: Reminder
 * 4. When books are picked up/placed, emotion resets based on book count (upgrades)
 */
void calculateEmotion() {
  uint32_t currentTime = millis();
  
  // ─── Phase 0: Boot calm period (first 5 minutes) ───
  if (inBootCalmPeriod) {
    uint32_t elapsed = currentTime - bootCompleteTime;
    if (elapsed < BOOT_CALM_TIME) {
      currentEmotion = EMOTION_CALM;
      return;
    } else {
      inBootCalmPeriod = false;
      Serial.println("Boot calm period ended");
    }
  }
  
  // ─── Phase 1: Calm hold after ALL books placed (5 min calm) ───
  if (calmHoldActive) {
    uint32_t elapsed = currentTime - calmHoldStartTime;
    if (elapsed < CALM_HOLD_TIME) {
      currentEmotion = EMOTION_CALM;
      return;
    } else {
      // Calm hold expired → start degrading
      calmHoldActive  = false;
      degradeActive   = true;
      degradeStartTime = currentTime;
      currentDegradeStage = 0;
      Serial.println("Calm hold expired, starting degradation");
    }
  }
  
  // ─── Phase 2: Degradation (every DEGRADE_INTERVAL) ───
  if (degradeActive) {
    uint32_t elapsed = currentTime - degradeStartTime;
    uint8_t stage = elapsed / DEGRADE_INTERVAL;
    
    if (stage > DEGRADE_STAGES) stage = DEGRADE_STAGES;
    currentDegradeStage = stage;
    
    // Degradation stages:
    //   0: Calm
    //   1: Neutral
    //   2: Neutral (maintained)
    //   3: Sad
    //   4: Very Sad
    //   5: Reminder (audio + sad face)
    
    switch (stage) {
      case 0: currentEmotion = EMOTION_CALM;    break;
      case 1: currentEmotion = EMOTION_NEUTRAL; break;
      case 2: currentEmotion = EMOTION_NEUTRAL; break;
      case 3: currentEmotion = EMOTION_SAD;     break;
      case 4: currentEmotion = EMOTION_VERY_SAD; break;
      case 5:
        currentEmotion = EMOTION_REMINDER;
        if (!reminderSpoken) {
          reminderSpoken = true;
          Serial.println("REMINDER: Time to study!");
          if (dfPlayerReady) {
            df.playMp3Folder(AUDIO_TIME_TO_STUDY);
          }
        }
        break;
    }
    return;
  }
  
  // ─── Normal: Emotion based on book count (progressive upgrade) ───
  switch (currentBookCount) {
    case 0:  currentEmotion = EMOTION_NEUTRAL; break;
    case 1:  currentEmotion = EMOTION_CALM;    break;
    case 2:  currentEmotion = EMOTION_CALM;    break;
    case 3:  currentEmotion = EMOTION_HAPPY;   break;
    case 4:  currentEmotion = EMOTION_HAPPY;   break;
    case 5:  currentEmotion = EMOTION_PROUD;   break;
  }
}

// ========================== TOUCH SENSOR ==========================
void handleTouchSensor() {
  uint32_t currentTime = millis();
  bool touchReading = digitalRead(TOUCH_SENSOR);
  
  if (touchReading && !touchPressed) {
    touchPressed  = true;
    touchStartTime = currentTime;
  }
  
  if (!touchReading && touchPressed) {
    uint32_t pressDuration = currentTime - touchStartTime;
    
    if (pressDuration >= LONG_PRESS_TIME) {
      // ===== LONG PRESS: Detailed Stats =====
      Serial.println("LONG PRESS -> Detailed Stats");
      if (dfPlayerReady) df.playMp3Folder(AUDIO_CHIME);
      showDetailedStats();
      
    } else if (pressDuration > 50) {
      // ===== SHORT PRESS: Quick Status =====
      Serial.println("SHORT PRESS -> Quick Status");
      if (dfPlayerReady) df.playMp3Folder(AUDIO_CLICK);
      showQuickStatus();
      lastInteractionTime = currentTime;
      
      // Reset degrade on interaction
      if (degradeActive || calmHoldActive) {
        calmHoldActive    = false;
        degradeActive     = false;
        currentDegradeStage = 0;
        reminderSpoken    = false;
      }
    }
    
    touchPressed = false;
  }
}

// ========================== QUICK STATUS (Short Press) ==========================
void showQuickStatus() {
  overlayActive = true;
  tft.fillScreen(BLUE);
  tft.setTextColor(BLACK, BLUE);
  
  // "GREAT JOB!"
  tft.setTextSize(5);
  const char* l1 = "GREAT JOB!";
  int16_t w = strlen(l1) * 5 * 6;
  int16_t x = (screenWidth - w) / 2;
  tft.setCursor(x, 15);
  tft.print(l1);
  
  // Separator
  tft.drawFastHLine(30, 65, screenWidth - 60, BLACK);
  
  // Time
  tft.setTextSize(3);
  if (rtcReady) {
    DateTime now = rtc.now();
    char timeBuf[30];
    sprintf(timeBuf, "Time: %02d:%02d:%02d", now.hour(), now.minute(), now.second());
    w = strlen(timeBuf) * 3 * 6;
    x = (screenWidth - w) / 2;
    tft.setCursor(x, 80);
    tft.print(timeBuf);
  }
  
  // Books count
  tft.setTextSize(4);
  char bookBuf[20];
  sprintf(bookBuf, "Books: %d / 5", currentBookCount);
  w = strlen(bookBuf) * 4 * 6;
  x = (screenWidth - w) / 2;
  tft.setCursor(x, 130);
  tft.print(bookBuf);
  
  // Total study time
  uint32_t totalSecs = 0;
  for (int i = 0; i < NUM_BOOKS; i++) {
    totalSecs += getBookTotalSeconds(i);
  }
  String totalStr = "Study: " + formatDuration(totalSecs);
  tft.setTextSize(4);
  w = totalStr.length() * 4 * 6;
  x = (screenWidth - w) / 2;
  tft.setCursor(x, 200);
  tft.print(totalStr);
  
  // WiFi IP
  if (wifiConnected) {
    tft.setTextSize(2);
    String ipStr = "IP: " + WiFi.localIP().toString();
    w = ipStr.length() * 2 * 6;
    x = (screenWidth - w) / 2;
    tft.setCursor(x, 270);
    tft.print(ipStr);
  }
  
  delay(3000);
  
  tft.fillScreen(BLUE);
  drawFaceComplete();
  overlayActive = false;
}

// ========================== DETAILED STATS (Long Press) ==========================
void showDetailedStats() {
  overlayActive = true;
  tft.fillScreen(BLUE);
  tft.setTextColor(BLACK, BLUE);
  
  // Title
  tft.setTextSize(4);
  const char* t1 = "TODAY'S PROGRESS";
  int16_t w = strlen(t1) * 4 * 6;
  int16_t x = (screenWidth - w) / 2;
  tft.setCursor(x, 10);
  tft.print(t1);
  
  // Separator
  tft.drawFastHLine(15, 55, screenWidth - 30, BLACK);
  
  // Each subject with time (landscape: more horizontal space)
  tft.setTextSize(3);
  int yPos = 70;
  int lineHeight = 42;
  
  for (int i = 0; i < NUM_BOOKS; i++) {
    uint32_t secs = getBookTotalSeconds(i);
    String timeStr = formatDuration(secs);
    
    // Status indicator
    if (books[i].present) {
      tft.fillCircle(25, yPos + 10, 6, 0x07E0); // Green dot
    } else {
      tft.drawCircle(25, yPos + 10, 6, BLACK);
    }
    
    // Subject name
    tft.setCursor(45, yPos);
    tft.print(BOOK_NAMES[i]);
    tft.print(":");
    
    // Time (right side)
    int16_t tw = timeStr.length() * 3 * 6;
    tft.setCursor(screenWidth - tw - 30, yPos);
    tft.print(timeStr);
    
    yPos += lineHeight;
  }
  
  // Footer
  tft.setTextSize(3);
  const char* tip = "KEEP GOING!";
  w = strlen(tip) * 3 * 6;
  x = (screenWidth - w) / 2;
  tft.setCursor(x, 290);
  tft.print(tip);
  
  delay(5000);
  
  tft.fillScreen(BLUE);
  drawFaceComplete();
  overlayActive = false;
}

// ========================== EMOTION AUDIO ==========================
void playEmotionAudio() {
  if (!dfPlayerReady) return;
  
  switch (currentEmotion) {
    case EMOTION_HAPPY:
      df.playMp3Folder(AUDIO_GOOD_JOB);
      break;
    case EMOTION_PROUD:
      df.playMp3Folder(AUDIO_WELL_DONE);
      break;
    case EMOTION_SAD:
      df.playMp3Folder(AUDIO_UH_OH);
      break;
    default:
      break;
  }
}

// ========================== EMOTION TARGETS ==========================
void setEmotionTargets(Emotion e) {
  switch (e) {
    case EMOTION_NEUTRAL:
      t_eyeOpen = 30; t_pupilOffset = 0; t_mouthCurve = 0; t_eyesClosed = false;
      break;
    case EMOTION_CALM:
      t_eyeOpen = 28; t_pupilOffset = 0; t_mouthCurve = 5; t_eyesClosed = false;
      break;
    case EMOTION_HAPPY:
      t_eyeOpen = 0; t_pupilOffset = 0; t_mouthCurve = 10; t_eyesClosed = true;
      break;
    case EMOTION_PROUD:
      t_eyeOpen = 0; t_pupilOffset = 0; t_mouthCurve = 14; t_eyesClosed = true;
      break;
    case EMOTION_VERY_PROUD:
      t_eyeOpen = 0; t_pupilOffset = 0; t_mouthCurve = 16; t_eyesClosed = true;
      break;
    case EMOTION_SAD:
      t_eyeOpen = 32; t_pupilOffset = 5; t_mouthCurve = -5; t_eyesClosed = false;
      break;
    case EMOTION_VERY_SAD:
      t_eyeOpen = 34; t_pupilOffset = 8; t_mouthCurve = -10; t_eyesClosed = false;
      break;
    case EMOTION_REMINDER:
      t_eyeOpen = 34; t_pupilOffset = 6; t_mouthCurve = -8; t_eyesClosed = false;
      break;
  }
}

// ========================== FACE ANIMATION (Flicker-Free) ==========================
void animateFace() {
  if (millis() - lastFrame < 50) return;
  lastFrame = millis();
  
  // Store old values
  int old_eyeOpen     = eyeOpen;
  int old_pupilOffset = pupilOffset;
  int old_mouthCurve  = mouthCurve;
  bool old_eyesClosed = eyesClosed;
  
  // Smoothly interpolate toward targets
  eyeOpen     += constrain(t_eyeOpen - eyeOpen, -2, 2);
  pupilOffset += constrain(t_pupilOffset - pupilOffset, -1, 1);
  mouthCurve  += constrain(t_mouthCurve - mouthCurve, -1, 1);
  eyesClosed   = t_eyesClosed;
  
  // Only redraw what changed
  bool eyesChanged  = (eyeOpen != old_eyeOpen || pupilOffset != old_pupilOffset || eyesClosed != old_eyesClosed);
  bool mouthChanged = (mouthCurve != old_mouthCurve);
  
  if (eyesChanged) {
    redrawEyes();
  }
  
  if (mouthChanged) {
    redrawMouth();
  }
}

void handleBlink() {
  if (eyesClosed) return;
  
  if (millis() - lastBlink > 3500) {
    lastBlink = millis();
    quickBlink();
  }
}

void quickBlink() {
  clearEyeArea();
  drawClosedEye(centerX - EYE_SPACING, centerY + EYE_Y_OFFSET);
  drawClosedEye(centerX + EYE_SPACING, centerY + EYE_Y_OFFSET);
  delay(100);
  clearEyeArea();
  drawEye(centerX - EYE_SPACING, centerY + EYE_Y_OFFSET);
  drawEye(centerX + EYE_SPACING, centerY + EYE_Y_OFFSET);
}

// ========================== DRAW FACE COMPLETE ==========================
void drawFaceComplete() {
  // Cheeks (blush)
  drawCheeks();
  
  // Eyes
  if (eyesClosed) {
    drawClosedEye(centerX - EYE_SPACING, centerY + EYE_Y_OFFSET);
    drawClosedEye(centerX + EYE_SPACING, centerY + EYE_Y_OFFSET);
  } else {
    drawEye(centerX - EYE_SPACING, centerY + EYE_Y_OFFSET);
    drawEye(centerX + EYE_SPACING, centerY + EYE_Y_OFFSET);
  }
  
  // Mouth
  drawMouth();
}

void drawCheeks() {
  uint16_t pinkColor = tft.color565(255, 182, 193);
  tft.fillCircle(centerX - CHEEK_X_OFFSET, centerY + CHEEK_Y_OFFSET, CHEEK_RADIUS, pinkColor);
  tft.fillCircle(centerX + CHEEK_X_OFFSET, centerY + CHEEK_Y_OFFSET, CHEEK_RADIUS, pinkColor);
}

void redrawEyes() {
  clearEyeArea();
  
  if (eyesClosed) {
    drawClosedEye(centerX - EYE_SPACING, centerY + EYE_Y_OFFSET);
    drawClosedEye(centerX + EYE_SPACING, centerY + EYE_Y_OFFSET);
  } else {
    drawEye(centerX - EYE_SPACING, centerY + EYE_Y_OFFSET);
    drawEye(centerX + EYE_SPACING, centerY + EYE_Y_OFFSET);
  }
}

void redrawMouth() {
  clearMouth();
  drawMouth();
}

void clearEyeArea() {
  tft.fillRect(centerX - EYE_SPACING - EYE_RADIUS - 5, 
               centerY + EYE_Y_OFFSET - EYE_RADIUS - 5, 
               (EYE_RADIUS + 5) * 2, 
               (EYE_RADIUS + 5) * 2, BLUE);
  
  tft.fillRect(centerX + EYE_SPACING - EYE_RADIUS - 5, 
               centerY + EYE_Y_OFFSET - EYE_RADIUS - 5, 
               (EYE_RADIUS + 5) * 2, 
               (EYE_RADIUS + 5) * 2, BLUE);
  
  drawCheeks();
}

void drawEye(int x, int y) {
  // White outer
  tft.fillCircle(x, y, EYE_RADIUS, WHITE);
  tft.drawCircle(x, y, EYE_RADIUS, tft.color565(200, 200, 200));
  
  // Black pupil
  int pupilY = y + pupilOffset;
  tft.fillCircle(x, pupilY, PUPIL_RADIUS, BLACK);
  
  // Highlights
  tft.fillCircle(x - 6, pupilY - 6, 6, WHITE);
  tft.fillCircle(x - 9, pupilY - 9, 2, WHITE);
}

void drawClosedEye(int x, int y) {
  int halfWidth = 30;
  for (int thickness = -3; thickness <= 3; thickness++) {
    for (int i = -halfWidth; i <= halfWidth; i++) {
      float normalized = (float)i / halfWidth;
      int curve = (int)(normalized * normalized * 18);
      tft.drawPixel(x + i, y + curve + thickness, BLACK);
    }
  }
}

void clearMouth() {
  tft.fillRect(centerX - MOUTH_WIDTH - 30, centerY + MOUTH_Y - 30, 
               (MOUTH_WIDTH + 30) * 2, 80, BLUE);
}

void drawMouth() {
  int cx    = centerX;
  int cy    = centerY + MOUTH_Y;
  int width = MOUTH_WIDTH;
  
  if (mouthCurve > 0) {
    // ===== SMILE =====
    int curve = mouthCurve * 5;
    
    for (int thick = -3; thick <= 3; thick++) {
      for (int i = -width; i <= width; i++) {
        float normalized = (float)i / width;
        int y = cy + (int)(normalized * normalized * curve) + thick;
        tft.drawPixel(cx + i, y, BLACK);
      }
    }
    
    // Very happy: open mouth
    if (mouthCurve > 10) {
      int mouthOpenY = cy + curve + 10;
      tft.fillCircle(cx, mouthOpenY, 20, BLACK);
      uint16_t redColor = tft.color565(255, 100, 100);
      tft.fillCircle(cx, mouthOpenY + 6, 14, redColor);
    }
    
  } else if (mouthCurve < 0) {
    // ===== FROWN =====
    int curve = abs(mouthCurve) * 4;
    
    for (int thick = -3; thick <= 3; thick++) {
      for (int i = -width; i <= width; i++) {
        float normalized = (float)i / width;
        int y = cy - (int)(normalized * normalized * curve) + thick;
        tft.drawPixel(cx + i, y, BLACK);
      }
    }
    
    if (mouthCurve < -8) {
      int frowY = cy - curve - 5;
      tft.fillCircle(cx, frowY, 10, BLACK);
    }
    
  } else {
    // ===== NEUTRAL: straight line =====
    for (int thick = -2; thick <= 2; thick++) {
      tft.drawFastHLine(cx - width, cy + thick, width * 2, BLACK);
    }
  }
}

// ========================== MAIN LOOP ==========================
void loop() {
  // Handle web server
  if (wifiConnected) {
    ws.cleanupClients();
  }
  
  // Update systems
  updateBookStates();
  handleTouchSensor();
  calculateEmotion();
  
  // Handle emotion changes
  if (currentEmotion != lastEmotion) {
    Serial.print("Emotion changed: ");
    Serial.println(getEmotionName());
    setEmotionTargets(currentEmotion);
    playEmotionAudio();
    lastEmotion = currentEmotion;
  }
  
  // Animate face (only if no overlay)
  if (!overlayActive) {
    animateFace();
    handleBlink();
  }
  
  delay(30);
}

// ========================== WEBSOCKET SETUP ==========================
void setupWebSocket() {
  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);
  Serial.println("WebSocket ready at /ws");
}

void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, 
                      AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.print("WS client connected: #");
    Serial.println(client->id());
    sendWebSocketUpdate();
    
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.print("WS client disconnected: #");
    Serial.println(client->id());
  }
}

void sendWebSocketUpdate() {
  if (ws.count() == 0) return; // No clients connected
  
  String json = "{";
  json += "\"status\":\"live\",";
  json += "\"uptime\":" + String(millis() / 1000) + ",";
  json += "\"emotion\":\"" + getEmotionName() + "\",";
  json += "\"booksPlaced\":" + String(currentBookCount) + ",";
  
  if (rtcReady) {
    DateTime now = rtc.now();
    char timeBuf[30];
    sprintf(timeBuf, "%04d-%02d-%02dT%02d:%02d:%02d", 
            now.year(), now.month(), now.day(),
            now.hour(), now.minute(), now.second());
    json += "\"time\":\"" + String(timeBuf) + "\",";
  } else {
    json += "\"time\":\"N/A\",";
  }
  
  json += "\"books\":[";
  for (int i = 0; i < NUM_BOOKS; i++) {
    json += "{";
    json += "\"name\":\"" + String(BOOK_NAMES[i]) + "\",";
    json += "\"present\":" + String(books[i].present ? "true" : "false") + ",";
    json += "\"totalSeconds\":" + String(getBookTotalSeconds(i));
    json += "}";
    if (i < NUM_BOOKS - 1) json += ",";
  }
  json += "]";
  
  uint32_t totalSecs = 0;
  for (int i = 0; i < NUM_BOOKS; i++) {
    totalSecs += getBookTotalSeconds(i);
  }
  json += ",\"totalStudySeconds\":" + String(totalSecs);
  
  // Extra: degrade info
  json += ",\"degradeStage\":" + String(currentDegradeStage);
  json += ",\"bootCalm\":" + String(inBootCalmPeriod ? "true" : "false");
  json += ",\"calmHold\":" + String(calmHoldActive ? "true" : "false");
  json += ",\"degradeActive\":" + String(degradeActive ? "true" : "false");
  
  json += "}";
  
  ws.textAll(json);
}

String getEmotionName() {
  switch (currentEmotion) {
    case EMOTION_NEUTRAL:    return "Neutral";
    case EMOTION_CALM:       return "Calm";
    case EMOTION_HAPPY:      return "Happy";
    case EMOTION_PROUD:      return "Proud";
    case EMOTION_VERY_PROUD: return "Very Proud";
    case EMOTION_SAD:        return "Sad";
    case EMOTION_VERY_SAD:   return "Very Sad";
    case EMOTION_REMINDER:   return "Reminder";
    default:                 return "Unknown";
  }
}

// ========================== WEB SERVER SETUP ==========================
void setupWebServer() {
  // CORS headers
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");
  
  // ─── API: Status ───
  server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{";
    json += "\"status\":\"live\",";
    json += "\"uptime\":" + String(millis() / 1000) + ",";
    json += "\"emotion\":\"" + getEmotionName() + "\",";
    json += "\"booksPlaced\":" + String(currentBookCount) + ",";
    
    if (rtcReady) {
      DateTime now = rtc.now();
      char timeBuf[30];
      sprintf(timeBuf, "%04d-%02d-%02dT%02d:%02d:%02d", 
              now.year(), now.month(), now.day(),
              now.hour(), now.minute(), now.second());
      json += "\"time\":\"" + String(timeBuf) + "\",";
    } else {
      json += "\"time\":\"N/A\",";
    }
    
    json += "\"books\":[";
    for (int i = 0; i < NUM_BOOKS; i++) {
      json += "{";
      json += "\"name\":\"" + String(BOOK_NAMES[i]) + "\",";
      json += "\"present\":" + String(books[i].present ? "true" : "false") + ",";
      json += "\"totalSeconds\":" + String(getBookTotalSeconds(i));
      json += "}";
      if (i < NUM_BOOKS - 1) json += ",";
    }
    json += "]";
    
    uint32_t totalSecs = 0;
    for (int i = 0; i < NUM_BOOKS; i++) {
      totalSecs += getBookTotalSeconds(i);
    }
    json += ",\"totalStudySeconds\":" + String(totalSecs);
    json += ",\"degradeStage\":" + String(currentDegradeStage);
    json += ",\"bootCalm\":" + String(inBootCalmPeriod ? "true" : "false");
    json += "}";
    
    request->send(200, "application/json", json);
  });
  
  // ─── API: Sync Time ───
  server.on("/api/synctime", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
      if (!rtcReady) {
        request->send(500, "application/json", "{\"error\":\"RTC not available\"}");
        return;
      }
      
      String body = "";
      for (size_t i = 0; i < len; i++) {
        body += (char)data[i];
      }
      
      int idx = body.indexOf("\"epoch\":");
      if (idx >= 0) {
        String epochStr = body.substring(idx + 8);
        epochStr.trim();
        int endIdx = epochStr.indexOf('}');
        if (endIdx >= 0) epochStr = epochStr.substring(0, endIdx);
        epochStr.trim();
        
        uint32_t epoch = epochStr.toInt();
        if (epoch > 1000000000) {
          rtc.adjust(DateTime(epoch));
          
          DateTime now = rtc.now();
          char timeBuf[30];
          sprintf(timeBuf, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
          
          request->send(200, "application/json", 
                        "{\"success\":true,\"newTime\":\"" + String(timeBuf) + "\"}");
          
          Serial.print("RTC synced to: ");
          Serial.println(timeBuf);
          return;
        }
      }
      
      request->send(400, "application/json", "{\"error\":\"Invalid request\"}");
    }
  );
  
  // ─── Landing page ───
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<!DOCTYPE html><html><head>";
    html += "<title>BookBuddy ESP32</title>";
    html += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
    html += "<style>body{font-family:Arial,sans-serif;text-align:center;padding:40px;";
    html += "background:#1e293b;color:white;} h1{font-size:2em;} ";
    html += ".status{display:inline-block;width:12px;height:12px;border-radius:50%;";
    html += "background:#22c55e;margin-right:8px;animation:pulse 2s infinite;}";
    html += "@keyframes pulse{0%,100%{opacity:1}50%{opacity:0.5}} ";
    html += ".card{background:#334155;padding:20px;border-radius:16px;margin:15px auto;";
    html += "max-width:400px;} a{color:#60a5fa;}</style></head><body>";
    html += "<h1>📚 BookBuddy ESP32</h1>";
    html += "<div class='card'><span class='status'></span><strong>LIVE</strong></div>";
    html += "<div class='card'>";
    html += "<p><strong>WebSocket:</strong> ws://" + WiFi.localIP().toString() + "/ws</p>";
    html += "<p><strong>API:</strong> http://" + WiFi.localIP().toString() + "/api/status</p>";
    html += "<p><strong>Sync Time:</strong> POST /api/synctime</p>";
    html += "</div>";
    html += "<div class='card'>";
    html += "<p>Open the BookBuddy Dashboard and enter this IP:</p>";
    html += "<p style='font-size:1.5em;font-weight:bold;color:#60a5fa;'>" + WiFi.localIP().toString() + "</p>";
    html += "</div>";
    html += "</body></html>";
    
    request->send(200, "text/html", html);
  });
  
  server.begin();
  Serial.println("Web server started");
  Serial.print("Local: http://");
  Serial.println(WiFi.localIP());
}
