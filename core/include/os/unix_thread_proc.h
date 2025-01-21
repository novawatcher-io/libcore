#pragma once

#include <memory>

namespace Core {

namespace Event {
class EventLoop;
}

namespace OS {

class UnixThread;

/**
 * @brief
 * 线程处理的上下文
 */
class UnixThreadProc {

public:
  explicit UnixThreadProc(const std::shared_ptr<Event::EventLoop> &loop_,
                          const std::shared_ptr<OS::UnixThread> &thread_)
      : loop(loop_), thread(thread_) {}

  /**
   * @brief
   * 线程运行的具体处理入口
   */
  void runThread();

private:
  std::shared_ptr<Event::EventLoop> loop;
  std::shared_ptr<OS::UnixThread> thread;
};
} // namespace OS
} // namespace Core