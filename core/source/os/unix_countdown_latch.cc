#include "os/unix_countdown_latch.h"

namespace Core::OS {
/**
 * 等待完成
 */
void UnixCountDownLatch::down() {
  std::lock_guard<std::mutex> guard(lck);
  count--;
  if (count == 0) {
    condition.notify_all();
  }
}

/**
 * 等待完成
 */
void UnixCountDownLatch::wait() {
  std::unique_lock<std::mutex> uniqueLock(lck);
  while (count > 0) {
    condition.wait(uniqueLock);
  }
}
}  // namespace Core::OS
