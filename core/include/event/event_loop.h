#pragma once

#include "event/event_channel.h"
#include "event/event_smart_ptr.h"
#include <event2/event.h>
#include <event2/event_compat.h>
#include <memory>
#include <spdlog/spdlog.h>
#include <unordered_map>
#include <utility>
struct bufferevent; // IWYU pragma: keep

/**
 * 信号派遣的函数类型
 */
using dispatcherSignal = void (*)(int, short, void *);

namespace Core::Event {

// 管道的map
using ChannelMap = std::unordered_map<int, std::shared_ptr<EventChannel>>;

using SignalSlot = std::unordered_map<int, EventPtr>;

/**
 * @brief
 *
 */
class EventLoop : public std::enable_shared_from_this<EventLoop> {
public:
  EventLoop() : base(event_init()) {}

  /**
   * 获取基址
   * @return
   */
  struct event_base *getEventBase() { return base.get(); }

  // 添加信号
  bool sigAdd(int sigNo, dispatcherSignal handle, void *userData) {
    EventPtr signale(evsignal_new(base.get(), sigNo, handle, userData));
    if (event_add(signale.get(), nullptr) != 0) {
      SPDLOG_ERROR("sig add error");
      return false;
    }
    slot[sigNo] = std::move(signale);
    return true;
  }

  // 退出
  void quit();

  // 事件循环
  void loop();

  /**
   * 更新管道
   * @param channel
   * @return
   */
  bool updateChannel(const std::shared_ptr<EventChannel> &channel);

  /**
   * 删除管道
   * @param channel
   * @return
   */
  bool deleteChannel(const std::shared_ptr<EventChannel> &channel);

  /**
   * @brief 删除管道通过描述符
   *
   * @param fd
   * @return true
   * @return false
   */
  bool deleteChannelByFd(int fd);

private:
  // libevent的基址
  Event::EventBasePtr base;

  /**
   * 管道集合
   */
  ChannelMap channels;

  /**
   * 信号槽，放在这里是为了防止 信号对象被析构
   */
  SignalSlot slot;
};
} // namespace Core::Event
