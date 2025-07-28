#include <iostream>
#include <cmath>
#include "scheduler.h"

struct AddNumber {
  float add(float a) const { return a + number; }
  float number;
};

int main() {
  float a = 1;
  float b = -2;
  float c = 0;

  AddNumber add{3.0f};

  TTaskScheduler scheduler;

  auto id1 = scheduler.add(
      [](float aa, float cc) { return -4 * aa * cc; },
      a, c);

  auto id2 = scheduler.add(
      [](float bb, float v) { return bb * bb + v; },
      b, scheduler.getFutureResult<float>(id1));

  auto id3 = scheduler.add(
      [](float bb, float d) { return -bb + std::sqrt(d); },
      b, scheduler.getFutureResult<float>(id2));

  auto id4 = scheduler.add(
      [](float bb, float d) { return -bb - std::sqrt(d); },
      b, scheduler.getFutureResult<float>(id2));

  auto id5 = scheduler.add(
      [](float aa, float v) { return v / (2 * aa); },
      a, scheduler.getFutureResult<float>(id3));

  auto id6 = scheduler.add(
      [](float aa, float v) { return v / (2 * aa); },
      a, scheduler.getFutureResult<float>(id4));

  auto id7 = scheduler.add(
      [add](float x) { return add.add(x); },
      scheduler.getFutureResult<float>(id6));

  scheduler.executeAll();

  std::cout << "x1 = " << scheduler.getResult<float>(id5) << '\n';
  std::cout << "x2 = " << scheduler.getResult<float>(id6) << '\n';
  std::cout << "x3 = " << scheduler.getResult<float>(id7) << '\n';
  return 0;
}