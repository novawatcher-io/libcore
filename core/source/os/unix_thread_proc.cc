#include "os/unix_thread_proc.h"
#include <spdlog/spdlog.h>
#include <functional>
#include <list>
#include "event/event_loop.h"
#include "os/unix_current_thread.h"
#include "os/unix_thread.h"

namespace Core::OS {
/**
 * @brief
 * 线程运行的具体处理入口
 */
void UnixThreadProc:: runThread() {
  UnixCurrentThread::currentLoop = loop;
  UnixCurrentThread::currentThread = thread;
  SPDLOG_INFO("thread {} start", UnixCurrentThread::tid());
  for (auto &data : thread->initList) {
    data();
  }
  loop->loop();
  for (auto &data : thread->finishList) {
    printf("111111111\n");
    SPDLOG_INFO("thread {} finish callable", UnixCurrentThread::tid());
    data();
  }
}
} // namespace Core::OS
