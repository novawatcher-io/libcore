#pragma once

#include <event2/bufferevent.h>
#include <event2/bufferevent_struct.h>
#include <event2/event.h>
#include <sys/time.h>

#include <functional>
#include <memory>

#include "event/event_loop.h"
#include "event/event_smart_ptr.h"
#include "event_channel.h"
#include "non_copyable.h"

namespace Core::Event {

typedef std::function<void(struct bufferevent *bev, EventChannel *ctx)> EventCallable;

class EventBufferChannel : public EventChannel, public Core::Noncopyable {
 public:
  EventBufferChannel(const std::shared_ptr<EventLoop> &loop_, int fd)
      : EventChannel(loop_, fd), bufptr(bufferevent_socket_new(loop->getEventBase(), this->getChannelFd(), 0)) {}

  /**
   * @brief Set the On Read Callable object
   *
   * @param callable
   * @return true
   * @return false
   */
  bool setOnReadCallable(const EventCallable &callable) {
    eventOnRead = callable;
    return true;
  }

  /**
   * @brief 绑定可读事件
   *
   * @param callable
   * 事件循环超时事件
   * @param second
   * @return true
   * @return false
   */
  bool bindOnReadCallable(const EventCallable &callable, int second) {
    eventOnRead = callable;
    enableReading(second);
    return true;
  }

  /**
   * @brief Get the On Read Callable object
   *
   * @return EventCallable
   */
  EventCallable &getOnReadCallable() { return eventOnRead; }

  void doReadCallable(bufferevent *evClient) {
    //        auto guard = tie_.lock();
    //        if (!guard) {
    //            SPDLOG_WARN("object tie has released");
    //            return;
    //        }
    eventOnRead(evClient, this);
  }

  /**
   * @brief Get the On Close Callable object
   *
   * @return EventCallable
   */
  EventCallable getOnCloseCallable() { return eventOnClose; }

  //    /**
  //     * @brief event 事件循环
  //     *
  //     * @param flag
  //     * @return true
  //     * @return false
  //     */
  //    bool handelEvent(short flag);

  /**
   * @brief Set the On Close Callable object
   * 不要去手动关闭连接，触发事件后会自己关闭连接
   *
   * @param callable
   * @return true
   * @return false
   */
  bool setOnCloseCallable(const EventCallable &callable) {
    eventOnClose = callable;
    return true;
  }

  /**
   * @brief 绑定关闭事件
   *
   * 事件
   * @param callable
   * 超时事件
   * @param second
   * @return true
   * @return false
   */
  bool bindOnCloseCallable(const EventCallable &callable, int second) {
    eventOnClose = callable;
    enableReading(second);
    return true;
  }

  /**
   * @brief Set the On Error Callable object
   *
   * @param callable
   * @return true
   * @return false
   */
  bool setOnErrorCallable(const EventCallable &callable) {
    eventOnError = callable;
    return true;
  }

  /**
   * @brief 绑定出错事件
   *
   * 事件
   * @param callable
   * 超时事件
   * @param second
   * @return true
   * @return false
   */
  bool bindOnErrorCallable(const EventCallable &callable, int second) {
    eventOnError = callable;
    enableReading(second);
    return true;
  }

  /**
   * @brief Set the On Write Callable object
   *
   * @param callable
   * @return true
   * @return false
   */
  bool setOnWriteCallable(const EventCallable &callable) {
    eventOnWrite = callable;
    return true;
  }

  void doWriteCallable(bufferevent *evClient) {
    //        auto guard = tie_.lock();
    //        if (!guard) {
    //            SPDLOG_WARN("object tie has released");
    //            return;
    //        }
    if (!eventOnWrite) {
      return;
    }
    eventOnWrite(evClient, this);
  }

  /**
   * @brief 绑定写入事件
   *
   * @param callable
   * @param second
   * @return true
   * @return false
   */
  bool bindOnWriteCallable(const EventCallable &callable, int second) {
    eventOnWrite = callable;
    enableWriting(second);
    return true;
  }

  /**
   * @brief Get the On Write Callable object
   *
   * @return EventCallable
   */
  EventCallable getOnWriteCallable() { return eventOnWrite; }

  //    /**
  //     * @brief Get the Channel Fd object
  //     *
  //     * @return int
  //     */
  //    int getChannelFd() {
  //        return channelFd;
  //    }

  /**
   * @brief 开启可读事件单位是秒, -1是不超时
   *
   * @param second
   * @return true
   * @return false
   */
  bool enableReading(double second) override {
    time.tv_sec = 0;
    time.tv_usec = second * 1000 * 1000;
    events |= EV_READ;
    update();
    return true;
  }

  /**
   * @brief 开启可读事件单位是秒, -1是不超时
   *
   * @param second
   * @return true
   * @return false
   */
  bool enable(double second) override {
    time.tv_sec = 0;
    time.tv_usec = second * 1000 * 1000;
    events |= EV_READ | EV_WRITE;
    update();
    return true;
  }

  // 禁用可读事件
  bool disableReading() override {
    events &= ~EV_READ;
    update();
    return true;
  }

  /**
   * @brief 开启写入事件
   *
   * @param second
   * @return true
   * @return false
   */
  bool enableWriting(double second) override {
    time.tv_sec = 0;
    time.tv_usec = second * 1000 * 1000;
    events |= EV_WRITE;
    update();
    return true;
  }

  /**
   * @brief 禁用写入事件
   *
   * @return true
   * @return false
   */
  bool disableWriting() override {
    events &= ~EV_WRITE;
    update();
    return true;
  }

  /**
   * @brief 更新管道
   *
   */
  void update();

  bool eventSet(const std::shared_ptr<EventLoop> &loop) override;

  //    /**
  //     * @brief Get the Timer object
  //     *
  //     * @return struct timeval&
  //     */
  //    struct timeval& getTimer() {
  //        return time;
  //    }

  virtual ~EventBufferChannel() {
#ifdef USE_DEBUG
    SPDLOG_DEBUG("~EventBufferChannel");
#endif
  }

 private:
  static void onEvent(bufferevent *ev, short flag, void *arg);

  /**
   * @brief 关闭管道
   *
   * @param evClient
   * @param arg
   */
  void close(bufferevent *evClient, EventChannel *arg);

  /**
   * @brief
   * 可读回调
   * @param evClient
   * @param arg
   */
  static void onRead(bufferevent *evClient, void *arg);

  /**
   * @brief
   * 可写回调
   */
  static void onWrite(bufferevent *evClient, void *arg);

  //    /**
  //     * @brief 事件循环
  //     *
  //     */
  //    const std::shared_ptr<EventLoop> &loop;

  /**
   * @brief 事件
   *
   */
  EventCallable eventOnRead;
  EventCallable eventOnClose;
  EventCallable eventOnWrite;
  EventCallable eventOnError;

  EventBufferEventPtr bufptr;
};
}  // namespace Core::Event
