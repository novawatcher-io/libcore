#pragma once
#include "build_expect.h"              // for build_likely
#include "event/event_queue.h"         // for Task
#include "event/event_queue_handler.h" // for EventQueueHandler
#include "non_copyable.h"              // for Noncopyable
#include <functional>                  // for function
#include <list>                        // for list
#include <memory>                      // for shared_ptr, unique_ptr, __sha...
#include <pthread.h>                   // for pthread_cond_t
#include <string>                      // for string, basic_string
#include <sys/eventfd.h>               // for eventfd, EFD_CLOEXEC, EFD_NON...
#include <sys/types.h>                 // for pid_t, ssize_t
#include <thread>                      // for thread
namespace Core {
namespace Event {
class EventBufferChannel;
}
} // namespace Core
namespace Core {
namespace Event {
class EventLoop;
}
} // namespace Core
namespace Core {
namespace OS {
class UnixThreadProc;
}
} // namespace Core

namespace Core::OS {

using InitCallable = std::function<void()>;

/**
 * @brief
 * 基本的线程单元，在线程之上的封装
 */
class UnixThread : public Core::Noncopyable, public std::enable_shared_from_this<UnixThread> {
public:
  UnixThread();

  virtual ~UnixThread() {
    if (build_likely(!daemonize) && build_likely(mRunStatus)) {
      threadHandle->join();
      mRunStatus = false;
    }
  }

  /**
   * @brief 启动线程
   * 做的事情:
   * 1.传入unique_ptr指针
   * 2.创建事件队列
   * 3.创建wakeup子线程的channel
   * 4.启动线程上下文
   *
   * @param object
   * @return true
   * @return false
   */
  virtual bool start();

  /**
   * @brief 停止线程，释放掉资源
   *
   */
  virtual void stop();

  /**
   * @brief 销毁释放组件
   *
   */
  virtual void destroy() {
    queue.reset();
    channel.reset();
    loop.reset();
    shared_from_this().reset();
  }

  virtual void resume() { return; }

  virtual bool status() { return true; }

  /**
   * @brief 暂时不支持守护线程
   *
   */
  virtual bool setDaemonize() { return false; }

  virtual void addInitCallable(const InitCallable &callable) { initList.push_back(callable); }

  /**
   * @brief 获取当前线程id
   *
   * @return pid_t
   */
  virtual pid_t getTid() { return tid; }

  // 获取线程内部的事件循环
  virtual const std::shared_ptr<Event::EventLoop> &getEventLoop() { return loop; }

  /**
   * 事件唤醒
   */
  virtual ssize_t wakeUp();

  /**
   * @brief 处理任务的回调
   *
   */
  virtual bool addTask(const Core::Event::Task &task) { return queue->pushTask(task); }

  void setName(const std::string &name_) { name = name_; }

  std::string &getName() { return name; }

protected:
  /**
   * @brief event loop 循环
   * 这里采用shared_ptr,而不是采用unique_ptr,event loop 主要是因为
   * stop 会往子线程发送消息，会触发闭包，std::bind 所以要用shared_ptr
   */
  std::shared_ptr<Event::EventLoop> loop;

private:
  /**
   * 设置线程id
   * @param tid_
   */
  void setTid(pid_t tid_) { tid = tid_; }

  /**
   * @brief 同步回收线程
   *
   */
  void join() { threadHandle->join(); }

  /**
   * 线程运行的主程序
   * @param arg
   * @return
   */
  static int ThreadProc(std::unique_ptr<UnixThreadProc> proc, const std::shared_ptr<UnixThread> &object);

  /**
   * @brief 创建管道的通讯描述符
   *
   * @return int
   */
  int createChannelFd() {
    // EFD_CLOEXEC 被exec 后会自动关闭，不会泄漏给exec的新进程空间
    // EFD_NONBLOCK 非阻塞模式
    return eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
  }

  /**
   * 条件锁
   */
  pthread_cond_t mCondLock;

  /**
   * 是否是守护线程
   */
  int daemonize = 0;

  /**
   * 是否要终止线程运行
   */
  bool mTerminated = false;

  /**
   * 是否要将线程挂起
   */
  bool isSuspend = false;

  // 线程运行状态
  bool mRunStatus = false;

  // 互斥原语
  //  std::shared_ptr<UnixCountDownLatch> latch;
  std::list<InitCallable> initList;

  // 管道
  std::shared_ptr<Event::EventBufferChannel> channel;

  // 消息队列
  std::shared_ptr<Event::EventQueueHandler> queue;

  // 线程句柄
  std::unique_ptr<std::thread> threadHandle;

  // 线程id
  pid_t tid = 0;

  // 名字
  std::string name;

  // 管道的fd
  int wakeupChannelFd = -1;

  friend class UnixThreadProc;
};
} // namespace Core::OS
