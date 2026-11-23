#pragma once
#include <algorithm>
#include <cstdint>
using std::min;
using std::max;
unsigned long millis();
long random(long low, long high);
template <typename T> T constrain(T value, T low, T high) { return min(max(value, low), high); }
