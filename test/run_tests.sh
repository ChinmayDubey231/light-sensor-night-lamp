#!/usr/bin/env sh
# Compiles sketch.ino against a fake Arduino API and runs the tests.
set -e
cd "$(dirname "$0")"
g++ -std=c++17 -Wall -Wextra -I. -x c++ test_sketch.cpp -o /tmp/night_lamp_tests
/tmp/night_lamp_tests
