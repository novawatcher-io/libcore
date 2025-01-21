#pragma once

#include <condition_variable> // std::condition_variable
#include <cstdint>
#include <mutex> // std::mutex, std::unique_lock

namespace Core {
namespace OS {

class UnixCountDownLatch {
public:
  UnixCountDownLatch(uint16_t count_) : count(count_) {}

  /**
   * 等待完成
   */
  void wait();

  /**
   * 完成
   */
  void down();

private:
  /**
   * 计数器
   */
  uint16_t count = 0;
  /**
   * 互斥锁
   */
  std::mutex lck;
  /**
   * 条件变量
   */
  std::condition_variable condition;
};
} // namespace OS
} // namespace Core