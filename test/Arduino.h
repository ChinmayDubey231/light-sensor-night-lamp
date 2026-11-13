// Minimal fake of the Arduino API so sketch.ino can be compiled and
// exercised on a normal computer (see test/run_tests.sh).
#pragma once
#include <cstdint>
#include <cstdio>

#define A0 14
#define OUTPUT 1
#define F(s) (s)

inline long constrain(long x, long a, long b) { return x < a ? a : (x > b ? b : x); }
inline long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Simulation state, driven by the test.
extern unsigned long fake_millis;
extern int fake_analog;
extern int fake_pwm;
extern int fake_reads;

inline unsigned long millis() { return fake_millis; }
inline int analogRead(uint8_t) { fake_reads++; return fake_analog; }
inline void analogWrite(uint8_t, int v) { fake_pwm = v; }
inline void pinMode(uint8_t, uint8_t) {}

struct FakeSerial {
  bool quiet = true;
  void begin(long) {}
  template <typename T> void print(T v) { if (!quiet) out(v); }
  template <typename T> void println(T v) { if (!quiet) { out(v); std::printf("\n"); } }
  void out(const char* s) { std::printf("%s", s); }
  void out(int v) { std::printf("%d", v); }
  void out(unsigned v) { std::printf("%u", v); }
  void out(long v) { std::printf("%ld", v); }
};
extern FakeSerial Serial;
