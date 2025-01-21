#include "event/event_no_buffer_channel.h"
#include "event/event_channel.h"
#include "event/event_loop.h"
#include <event2/event_compat.h>
#include <spdlog/spdlog.h>
#include <string>
struct bufferevent;

namespace Core::Event {

EventNoBufferChannel::EventNoBufferChannel(const std::shared_ptr<EventLoop> &loop_, int fd, bool auto_close_)
    : EventChannel(loop_, fd, auto_close_){};

void EventNoBufferChannel::update() { loop->updateChannel(shared_from_this()); }

void EventNoBufferChannel::onWrite(bufferevent * /*evClient*/, void * /*arg*/) {}

void EventNoBufferChannel::onEvent(int /*fd*/, short events, void *arg) {
  // 管道不能是空的
  if (!arg) return;

  // 管道事件触发
  auto channel = static_cast<EventNoBufferChannel *>(arg);
  channel->handelEvent(events);
}

bool EventNoBufferChannel::eventSet(const std::shared_ptr<EventLoop> &loop) {
  int ret = 0;
  if (!events) {
    SPDLOG_WARN("events is nullptr");
    return false;
  }

  if ((events & EV_READ) && (events & EV_WRITE)) {
    SPDLOG_WARN("events is error!");
    return false;
  }
  ptr.Reset(event_new(loop->getEventBase(), getChannelFd(), events, onEvent, this));
  event_del(ptr.get());
  event_set(ptr.get(), getChannelFd(), events, onEvent, this);
  if (getTimer().tv_sec > 0) {
    struct timeval time;

    time.tv_sec = this->getTimer().tv_sec;
    time.tv_usec = this->getTimer().tv_usec;
    ret = event_add(ptr.get(), &time);
  } else {
    ret = event_add(ptr.get(), nullptr);
  }
  if (ret != 0) {
    SPDLOG_WARN("event_add failed,fd:" + std::to_string(getChannelFd()));
    return false;
  }
  return true;
  ;
}

bool EventNoBufferChannel::handelEvent(short events) {

  if ((events & EV_CLOSED) != 0) {
    if (eventOnClose) eventOnClose(this);
    return true;
  }
  /**
   * EPOLLIN 判断刻度
   * EPOLLPRI 判断外带数据
   * EPOLLRDHUP 对端关闭或者写一方关闭
   */
  if (events & (EV_READ)) {
    if (eventOnRead) eventOnRead(this);
  }

  if (events & EV_WRITE) {
    if (eventOnWrite) eventOnWrite(this);
  }

  if (events & EV_TIMEOUT) {
    std::cout << "EV_TIMEOUT" << std::endl;
  }

  return true;
}
} // namespace Core::Event
