// Host-side tests for sketch.ino. Build and run with test/run_tests.sh.
#include "Arduino.h"

unsigned long fake_millis = 0;
int fake_analog = 0;
int fake_pwm = -1;
int fake_reads = 0;
FakeSerial Serial;

#include "../sketch.ino"

static int failures = 0;
#define CHECK(cond, msg)                                              \
  do {                                                                \
    if (cond) std::printf("  ok   %s\n", msg);                       \
    else { std::printf("  FAIL %s\n", msg); failures++; }            \
  } while (0)

// Advance simulated time in 1 ms steps, calling loop() each step.
static void run_for(unsigned long ms) {
  for (unsigned long i = 0; i < ms; i++) { fake_millis++; loop(); }
}

int main() {
  std::printf("sketch.ino tests\n");

  fake_analog = 100;  // bright room
  setup();

  int readsBefore = fake_reads;
  run_for(1000);
  CHECK(fake_reads - readsBefore == 10, "samples every 100 ms (10 reads in 1 s)");
  CHECK(fake_pwm == 0 && !lampOn, "bright room: LED off");

  fake_analog = 1000;  // lights out
  run_for(1000);
  CHECK(lampOn && fake_pwm == 255, "full dark: LED at full brightness");

  fake_analog = 600;  // dim
  run_for(1000);
  CHECK(lampOn && fake_pwm > 20 && fake_pwm < 255, "dim room: partial brightness");

  fake_analog = 275;  // inside the hysteresis band while on
  run_for(1000);
  CHECK(lampOn && fake_pwm > 0, "inside hysteresis band: stays ON");

  fake_analog = 200;  // brighter than DARK_OFF
  run_for(1000);
  CHECK(!lampOn && fake_pwm == 0, "below DARK_OFF: turns OFF");

  fake_analog = 275;  // inside band again, but now off
  run_for(1000);
  CHECK(!lampOn && fake_pwm == 0, "inside hysteresis band: stays OFF");

  // Smoothing: a single 100 ms spike to full dark must not switch the lamp on.
  fake_analog = 1000;
  run_for(100);
  fake_analog = 100;
  run_for(1000);
  CHECK(!lampOn, "one-sample spike is filtered out by the moving average");

  // millis() overflow (~49.7 days): sampling must keep working.
  fake_millis = 0xFFFFFFFFUL - 50;
  lastSampleAt = fake_millis;
  lastLogAt = fake_millis;
  readsBefore = fake_reads;
  run_for(1000);
  CHECK(fake_reads - readsBefore == 10, "timing survives millis() overflow");

  std::printf(failures ? "\n%d test(s) failed\n" : "\nall tests passed\n", failures);
  return failures ? 1 : 0;
}
