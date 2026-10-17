/*

  TouchForge — Experiment 4: F1 Reaction Finder (v2 + calibration)

  No enum used — plain int state machine, to rule out enum ordering issues.

 */

 #include <Wire.h>

 #include <Adafruit_GFX.h>

 #include <Adafruit_SSD1306.h>

 #define SCREEN_W  128

 #define SCREEN_H  64

 #define OLED_ADDR 0x3C

 Adafruit_SSD1306 display(SCREEN_W, SCREEN_H, &Wire, -1);

 const int TOUCH_PIN  = 4;

 const int LED_PIN    = 2;

 const int BUZZER_PIN = 18;

 const int PWM_RES_BITS = 8;

 const int COUNT_FREQ  = 3000;

 const int START_FREQ  = 5000;

 const int FALSE_FREQ  = 1200;

 const int BEST_FREQ   = 4000;

 void buzzerOn(int freq)     { ledcWriteTone(BUZZER_PIN, freq); }

 void buzzerOff()            { ledcWriteTone(BUZZER_PIN, 0); }

 void beep(int freq, int ms) { buzzerOn(freq); delay(ms); buzzerOff(); }

 const unsigned long COUNT_STEP_MS  = 700;

 const unsigned long HOLD_MIN_MS    = 1200;

 const unsigned long HOLD_MAX_MS    = 4000;

 const unsigned long GO_TIMEOUT_MS  = 3000;

 const unsigned long RESULT_HOLD_MS = 800;

 const unsigned long FALSESTART_HOLD_MS = 1500;

 const unsigned long TIMEOUT_HOLD_MS    = 800;

 const unsigned long CAL_DURATION_MS = 3000;

 unsigned long touchThresholdMs = 15;

 // ---- State machine using plain int constants instead of enum ----

 #define ST_IDLE        0

 #define ST_COUNT3      1

 #define ST_COUNT2      2

 #define ST_COUNT1      3

 #define ST_COUNTSTART  4

 #define ST_HOLD        5

 #define ST_GO          6

 #define ST_RESULT      7

 #define ST_FALSESTART  8

 #define ST_TIMEOUT     9

 int state = ST_IDLE;

 unsigned long stateEnteredAt = 0;

 unsigned long holdDelay = 0;

 unsigned long goStartMicros = 0;

 float currentTime = -1;

 float bestTime = -1;

 int   attempts = 0;

 int  lastTouch = LOW;

 bool tapPending = false;

 void enterState(int s) {

  state = s;

  stateEnteredAt = millis();

 }

 void startCountdown() {

  enterState(ST_COUNT3);

  beep(COUNT_FREQ, 100);

 }

 void calibrateTouchSensor() {

  display.clearDisplay();

  display.setTextSize(1);

  display.setCursor(10, 10);

  display.print("CALIBRATING...");

  display.setCursor(10, 28);

  display.print("Do NOT touch sensor");

  display.setCursor(30, 44);

  display.print("Hold still");

  display.display();

  Serial.println("Calibrating touch sensor -- do not touch...");

  unsigned long start = millis();

  unsigned long longestBlip = 0;

  unsigned long blipStart = 0;

  bool inBlip = false;

  int blipCount = 0;

  while (millis() - start < CAL_DURATION_MS) {

    int reading = digitalRead(TOUCH_PIN);

    if (reading == HIGH && !inBlip) {

      inBlip = true;

      blipStart = millis();

      blipCount++;

    }

    if (reading == LOW && inBlip) {

      inBlip = false;

      unsigned long blipLen = millis() - blipStart;

      if (blipLen > longestBlip) longestBlip = blipLen;

    }

  }

  touchThresholdMs = constrain(longestBlip + 10, 10, 80);

  Serial.print("Calibration done. Spurious blips: ");

  Serial.print(blipCount);

  Serial.print(", longest: ");

  Serial.print(longestBlip);

  Serial.print(" ms -> threshold set to ");

  Serial.print(touchThresholdMs);

  Serial.println(" ms");

  if (blipCount > 0) {

    Serial.println("WARNING: sensor is noisy. Consider turning the onboard");

    Serial.println("sensitivity trimmer, moving the pad away from wires,");

    Serial.println("or shielding it from nearby metal.");

  }

  display.clearDisplay();

  display.setTextSize(1);

  display.setCursor(10, 15);

  display.print("Calibration done");

  display.setCursor(10, 32);

  display.print("Threshold: ");

  display.print(touchThresholdMs);

  display.print("ms");

  if (blipCount > 0) {

    display.setCursor(10, 46);

    display.print("Noise detected!");

  }

  display.display();

  delay(1200);

 }

 void drawIdle() {

  display.setTextSize(2);

  display.setCursor(6, 6);

  display.print("F1 REACTION");

  display.setTextSize(1);

  display.setCursor(14, 32);

  display.print("Touch to start");

  display.setCursor(10, 50);

  display.print("BEST: ");

  if (bestTime > 0) { display.print(bestTime, 3); display.print(" s"); }

  else display.print("--");

 }

 void drawBigCenter(const char* text, int textSize) {

  display.setTextSize(textSize);

  int16_t x1, y1; uint16_t w, h;

  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

  display.setCursor((SCREEN_W - w) / 2, (SCREEN_H - h) / 2);

  display.print(text);

 }

 void drawResult() {

  display.setTextSize(2);

  display.setCursor(10, 2);

  display.print("REACTION");

  display.setTextSize(2);

  display.setCursor(10, 24);

  display.print(currentTime, 3);

  display.print("s");

  display.setTextSize(1);

  display.setCursor(4, 48);

  display.print("CUR: "); display.print(currentTime, 3);

  display.setCursor(70, 48);

  display.print("BEST: "); display.print(bestTime, 3);

  if (millis() - stateEnteredAt > RESULT_HOLD_MS) {

    display.setCursor(20, 58);

    display.print("Touch for next run");

  }

 }

 void drawFalseStart() {

  display.setTextSize(2);

  display.setCursor(4, 10);

  display.print("FALSE");

  display.setCursor(4, 30);

  display.print("START!");

  display.setTextSize(1);

  display.setCursor(2, 52);

  display.print("Wait for lights out!");

 }

 void drawTimeout() {

  display.setTextSize(2);

  display.setCursor(10, 14);

  display.print("TOO SLOW");

  display.setTextSize(1);

  display.setCursor(20, 40);

  display.print("No touch detected");

  display.setCursor(30, 52);

  display.print("Touch to retry");

 }

 void setup() {

  Serial.begin(115200);

  pinMode(TOUCH_PIN, INPUT_PULLDOWN);

  pinMode(LED_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);

  ledcAttach(BUZZER_PIN, START_FREQ, PWM_RES_BITS);

  buzzerOff();

  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {

    Serial.println("OLED not found! Check wiring / address.");

    while (true) { digitalWrite(LED_PIN, !digitalRead(LED_PIN)); delay(100); }

  }

  display.setTextColor(WHITE);

  calibrateTouchSensor();

  randomSeed(esp_random());

  Serial.println("=== TouchForge | Experiment 4: F1 Reaction Finder (v2) ===");

  enterState(ST_IDLE);

 }

 void loop() {

  int t = digitalRead(TOUCH_PIN);

  if (t == HIGH && lastTouch == LOW) tapPending = true;

  lastTouch = t;

  unsigned long now = millis();

  if (state == ST_IDLE) {

    display.clearDisplay(); drawIdle(); display.display();

    if (tapPending) { attempts = 0; startCountdown(); }

  }

  else if (state == ST_COUNT3) {

    if (tapPending) { buzzerOn(FALSE_FREQ); enterState(ST_FALSESTART); }

    else {

      display.clearDisplay(); drawBigCenter("3", 4); display.display();

      if (now - stateEnteredAt >= COUNT_STEP_MS) { enterState(ST_COUNT2); beep(COUNT_FREQ, 100); }

    }

  }

  else if (state == ST_COUNT2) {

    if (tapPending) { buzzerOn(FALSE_FREQ); enterState(ST_FALSESTART); }

    else {

      display.clearDisplay(); drawBigCenter("2", 4); display.display();

      if (now - stateEnteredAt >= COUNT_STEP_MS) { enterState(ST_COUNT1); beep(COUNT_FREQ, 100); }

    }

  }

  else if (state == ST_COUNT1) {

    if (tapPending) { buzzerOn(FALSE_FREQ); enterState(ST_FALSESTART); }

    else {

      display.clearDisplay(); drawBigCenter("1", 4); display.display();

      if (now - stateEnteredAt >= COUNT_STEP_MS) { enterState(ST_COUNTSTART); beep(START_FREQ, 120); }

    }

  }

  else if (state == ST_COUNTSTART) {

    if (tapPending) { buzzerOn(FALSE_FREQ); enterState(ST_FALSESTART); }

    else {

      display.clearDisplay(); drawBigCenter("START", 2); display.display();

      digitalWrite(LED_PIN, HIGH);

      if (now - stateEnteredAt >= COUNT_STEP_MS) {

        holdDelay = random(HOLD_MIN_MS, HOLD_MAX_MS + 1);

        enterState(ST_HOLD);

      }

    }

  }

  else if (state == ST_HOLD) {

    if (tapPending) {

      digitalWrite(LED_PIN, LOW);

      buzzerOn(FALSE_FREQ);

      enterState(ST_FALSESTART);

    } else {

      display.clearDisplay(); drawBigCenter("READY", 2); display.display();

      if (now - stateEnteredAt >= holdDelay) {

        digitalWrite(LED_PIN, LOW);

        display.clearDisplay();

        display.display();

        goStartMicros = micros();

        enterState(ST_GO);

      }

    }

  }

  else if (state == ST_GO) {

    if (tapPending) {

      unsigned long reactMicros = micros() - goStartMicros;

      currentTime = reactMicros / 1000000.0;

      attempts++;

      bool newBest = (bestTime < 0 || currentTime < bestTime);

      if (newBest) bestTime = currentTime;

      Serial.print("Reaction #"); Serial.print(attempts);

      Serial.print(": "); Serial.print(currentTime, 3);

      Serial.println(newBest ? " s  <-- NEW BEST" : " s");

      enterState(ST_RESULT);

      if (newBest) beep(BEST_FREQ, 100);

    }

    else if (now - stateEnteredAt > GO_TIMEOUT_MS) {

      enterState(ST_TIMEOUT);

    }

  }

  else if (state == ST_RESULT) {

    display.clearDisplay(); drawResult(); display.display();

    if (tapPending && now - stateEnteredAt > RESULT_HOLD_MS) startCountdown();

  }

  else if (state == ST_FALSESTART) {

    display.clearDisplay(); drawFalseStart(); display.display();

    if (now - stateEnteredAt > 300) buzzerOff();

    if (tapPending && now - stateEnteredAt > FALSESTART_HOLD_MS) startCountdown();

  }

  else if (state == ST_TIMEOUT) {

    display.clearDisplay(); drawTimeout(); display.display();

    if (tapPending && now - stateEnteredAt > TIMEOUT_HOLD_MS) startCountdown();

  }

  tapPending = false;

  if (state != ST_GO) delay(15);

 }
