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
  explicit UnixThreadProc(Event::EventLoop* loop_,
                         OS::UnixThread* thread_)
      : loop(loop_), thread(thread_) {}

  /**
   * @brief
   * 线程运行的具体处理入口
   */
  void runThread();

private:
 Event::EventLoop* loop;
 OS::UnixThread* thread;
};
} // namespace OS
} // namespace Core