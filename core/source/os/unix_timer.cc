#include "os/unix_timer.h"
#include <spdlog/spdlog.h>
#include <sys/timerfd.h>
#include <sys/types.h>
#include <string>
#include "build_expect.h"

namespace Core::OS {

UnixTimer::UnixTimer(time_t unit) {
  timerFd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  if (build_unlikely(timerFd <= 0)) {
    SPDLOG_WARN("create timer fd error");
    return;
  }

  // 参数new_value指定定时器的超时时间以及超时间隔时间
  struct itimerspec new_value {};
  __syscall_slong_t sec = (unit / 1000);
  __syscall_slong_t nsec = (unit % 1000) * 1000 * 1000;
  new_value.it_interval.tv_sec = sec;
  // （定时间隔周期）
  new_value.it_interval.tv_nsec = nsec;
  //(第一次超时时间)
  new_value.it_value.tv_sec = sec;
  new_value.it_value.tv_nsec = nsec;

  int res = timerfd_settime(timerFd, 0, &new_value, nullptr);
  if (res == -1) {
    SPDLOG_WARN("timerfd_settime error unit:" + std::to_string(nsec));
    return;
  }
}

} // namespace Core::OS
