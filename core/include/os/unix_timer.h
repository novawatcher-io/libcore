#pragma once

#include <ctime>

namespace Core::OS {

class UnixTimer {
public:
  UnixTimer() = default;

  /**
   * @brief 初始化unix定时器，时间单位是毫秒
   *
   * @param second
   */
  explicit UnixTimer(time_t unit);

  int getTimerFd() const { return timerFd; }

private:
  int timerFd = -1;
};
} // namespace Core::OS
