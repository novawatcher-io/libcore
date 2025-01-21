#include "event/event_loop.h"
#include "event/event_timer.h"
#include "gtest/gtest.h"
#include <functional>
#include <string>

TEST(TimerTest, Normal) {
  Core::Event::EventLoop loop;
  constexpr int timeout = 500;
  bool timer_called = false;
  Core::Event::RepeatedTimer const timer(&loop, timeout, [&]() {
    timer_called = true;
    loop.quit();
  });

  loop.loop();
  EXPECT_TRUE(timer_called);
}

TEST(TimerTest, Disable) {
  Core::Event::EventLoop loop;
  bool timer_called = false;
  constexpr int timeout = 500;
  Core::Event::RepeatedTimer timer(&loop, timeout, [&]() {
    timer_called = true;
    loop.quit();
  });
  timer.Disable();
  loop.loop();
  EXPECT_FALSE(timer_called);
}

TEST(TimerTest, Null) {
  bool timer_called = false;
  constexpr int timeout = 500;
  Core::Event::RepeatedTimer const timer(nullptr, timeout, [&]() { timer_called = true; });
  EXPECT_FALSE(timer_called);
}
