/*
 * Task 01 — Sensor and actuator: automatic night lamp
 *
 * Sensor  : LDR (photoresistor) voltage divider  -> A0
 * Actuator: LED + 220 ohm resistor               -> D9 (PWM)
 *
 * Every SAMPLE_INTERVAL_MS the sketch reads the light level, smooths it,
 * and sets the LED brightness: the darker the room, the brighter the LED.
 * In bright light the LED switches fully off, with hysteresis so it does
 * not flicker when the light sits right at the threshold.
 *
 * Wiring (divider): 5V -- 10k -- A0 -- LDR -- GND
 * More light -> lower LDR resistance -> lower reading on A0.
 */

const uint8_t SENSOR_PIN = A0;
const uint8_t LED_PIN    = 9;     // must be a PWM pin (3, 5, 6, 9, 10, 11 on Uno)

const unsigned long SAMPLE_INTERVAL_MS = 100;   // 10 readings per second
const unsigned long LOG_INTERVAL_MS    = 1000;  // serial log once per second

// Readings above DARK_ON turn the lamp on; it only turns off again once the
// reading falls below DARK_OFF. The gap between them is the hysteresis band.
const int DARK_ON  = 300;
const int DARK_OFF = 250;
const int FULL_DARK = 900;        // at or above this the LED is at full brightness

const uint8_t AVG_SAMPLES = 8;    // moving average over the last 0.8 s

int  samples[AVG_SAMPLES];
uint8_t sampleIndex = 0;
long sampleSum = 0;

bool lampOn = false;
uint8_t brightness = 0;

unsigned long lastSampleAt = 0;
unsigned long lastLogAt = 0;

int readSmoothed() {
  int raw = analogRead(SENSOR_PIN);
  sampleSum -= samples[sampleIndex];
  samples[sampleIndex] = raw;
  sampleSum += raw;
  sampleIndex = (sampleIndex + 1) % AVG_SAMPLES;
  return (int)(sampleSum / AVG_SAMPLES);
}

uint8_t brightnessFor(int level) {
  // Hysteresis: decide on/off first.
  if (!lampOn && level >= DARK_ON)  lampOn = true;
  if (lampOn  && level <  DARK_OFF) lampOn = false;
  if (!lampOn) return 0;

  // Scale the darkness to a PWM value. Start at a visible minimum (20)
  // rather than 0 so "on" is always noticeably on.
  int clamped = constrain(level, DARK_OFF, FULL_DARK);
  return (uint8_t)map(clamped, DARK_OFF, FULL_DARK, 20, 255);
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(9600);

  // Pre-fill the averaging buffer so the first outputs are not dragged
  // towards zero.
  int first = analogRead(SENSOR_PIN);
  for (uint8_t i = 0; i < AVG_SAMPLES; i++) samples[i] = first;
  sampleSum = (long)first * AVG_SAMPLES;

  Serial.println(F("Night lamp ready. level = 0 (bright) .. 1023 (dark)"));
}

void loop() {
  unsigned long now = millis();

  // Non-blocking timing: no delay(), so the loop stays free for other work.
  if (now - lastSampleAt >= SAMPLE_INTERVAL_MS) {
    lastSampleAt = now;
    int level = readSmoothed();
    brightness = brightnessFor(level);
    analogWrite(LED_PIN, brightness);

    if (now - lastLogAt >= LOG_INTERVAL_MS) {
      lastLogAt = now;
      Serial.print(F("level="));
      Serial.print(level);
      Serial.print(F("  lamp="));
      Serial.print(lampOn ? F("ON ") : F("OFF"));
      Serial.print(F("  pwm="));
      Serial.println(brightness);
    }
  }
}
