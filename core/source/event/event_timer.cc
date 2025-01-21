#include "event/event_timer.h"

#include <event2/event_compat.h>
#include <spdlog/spdlog.h>
#include <sys/time.h>

#include <functional>
#include <utility>

#include "event/event_loop.h"
#include "event2/event.h"

namespace Core::Event {
RepeatedTimer::RepeatedTimer(const std::shared_ptr<EventLoop> &loop, time_t millisecond, std::function<void()> callable)
    : RepeatedTimer(loop.get(), millisecond, std::move(callable)) {}

RepeatedTimer::RepeatedTimer(EventLoop *loop, time_t millisecond, std::function<void()> callable)
    : loop_(loop), callable_(std::move(callable)) {
  if (loop_ == nullptr) {
    return;
  }
  auto *event = evtimer_new(loop_->getEventBase(), TimerCallable, this);
  if (event == nullptr) {
    SPDLOG_ERROR("RepeatedTimer evtimer_new failed");
    return;
  }
  event_set(event, 0, EV_PERSIST, TimerCallable, this);

  ptr_.Reset(event);

  struct timeval time_val {};
  constexpr time_t kMillisecond = 1000;
  time_val.tv_sec = (millisecond / kMillisecond);
  time_val.tv_usec = (millisecond % kMillisecond);
  int ret = evtimer_add(ptr_.get(), &time_val);
  if (ret != 0) {
    SPDLOG_WARN("RepeatedTimer evtimer_add failed, ret: {}", ret);
  }
}

void RepeatedTimer::Disable() {
  if (ptr_) {
    evtimer_del(ptr_.get());
  }
}

void RepeatedTimer::TimerCallable(evutil_socket_t /*fd*/, short /*what*/, void *arg) {
  auto *helper = static_cast<RepeatedTimer *>(arg);
  if (helper->callable_) {
    helper->callable_();
  }
}

}  // namespace Core::Event
