#include "event/event_loop.h"
#include <event2/event.h>
#include <memory>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <unordered_map>

namespace Core::Event {

void EventLoop::loop() {
  if (!base) {
    return;
  }
  // 循环、分发事件
  int ret = event_base_dispatch(base.get());
  if (ret != 0) {
    SPDLOG_ERROR("create event loop error; ret={}", ret);
    //    exit(-1);
  }
}

void EventLoop::quit() {
  if (!base) {
    SPDLOG_ERROR("base error!");
    throw std::runtime_error("base error!");
  }

  // 循环、分发事件
  int ret = event_base_loopexit(base.get(), nullptr);
  if (ret != 0) {
    SPDLOG_WARN("event_base_loopexit failed, ret={}", ret);
  }
}

// 更新管道
bool EventLoop::updateChannel(const std::shared_ptr<EventChannel> &channel) {
  // 这里不要加锁，不要发生竞争关系
  int fd = channel->getChannelFd();
  auto it = channels.find(fd);
  if (it == channels.end()) {
    channels.insert({fd, channel});
  } else {
    channels[fd] = channel;
  }
  channel->eventSet(shared_from_this());
  return true;
}

bool EventLoop::deleteChannel(const std::shared_ptr<EventChannel> &channel) {
  // 这里不要加锁，不要发生竞争关系
  int fd = channel->getChannelFd();
  if (fd == -1) {
    SPDLOG_WARN("channel fd has closed");
    return false;
  }
  auto it = channels.find(fd);

  if (it != channels.end()) {
    channels.erase(it);
    return true;
  }
  return false;
}

bool EventLoop::deleteChannelByFd(int fd) {
  // 这里不要加锁，不要发生竞争关系
  auto it = channels.find(fd);

  if (it != channels.end()) {
    channels.erase(it);
    return true;
  }
  return false;
}
} // namespace Core::Event
