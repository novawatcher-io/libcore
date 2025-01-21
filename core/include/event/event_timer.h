#pragma once

#include "event_smart_ptr.h"
#include "non_copyable.h"
#include <ctime>
#include <event2/util.h>
#include <functional>
#include <memory>

namespace Core::Event {
class EventLoop;

class RepeatedTimer : public Core::Noncopyable {
public:
  explicit RepeatedTimer(const std::shared_ptr<EventLoop> &loop, time_t millisecond, std::function<void()> callable);
  explicit RepeatedTimer(EventLoop *loop, time_t millisecond, std::function<void()> callable);

  void Disable();

private:
  static void TimerCallable(evutil_socket_t /*fd*/, short /*what*/, void *arg);

  EventLoop *loop_;
  EventPtr ptr_;
  std::function<void()> callable_;
};
} // namespace Core::Event
