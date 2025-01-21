#pragma once

#include <cstdint>
#include <event2/event.h>
#include <memory>
#include <spdlog/spdlog.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
namespace Core {
namespace Event {
class EventLoop;
}
} // namespace Core

namespace Core::Event {

class EventChannel : public std::enable_shared_from_this<EventChannel> {
public:
  EventChannel(const std::shared_ptr<EventLoop> &loop_, int fd_, bool auto_close = true)
      : auto_close_(auto_close), loop(loop_), fd(fd_) {}

  virtual bool eventSet(const std::shared_ptr<EventLoop> & /*loop*/) { return true; }

  /**
   * @brief 获取channel的fd
   *
   * @return int
   */
  int getChannelFd() const { return fd; }

  void close();

  void shutdownWrite();

  void shutdownRead();

  unsigned char getEvents() const { return events; }

  virtual bool enable(double /*unused*/) { return true; };

  virtual auto enable(unsigned char /*unused*/, __time_t /*unused*/) -> bool { return true; }

  virtual bool disableReading() { return true; };

  virtual bool disableWriting() { return true; }

  virtual bool enableWriting(double /*unused*/) { return true; }

  virtual bool enableReading(double /*unused*/) { return true; }

  /**
   * @brief Set the Events object
   *
   * @param _events
   * @return true
   * @return false
   */
  void setEvents(uint32_t _events) { events = _events; }

  /**
   * @brief Get the Timer object
   *
   * @return struct timeval&
   */
  struct timeval &getTimer() { return time; }

  virtual ~EventChannel() {
    SPDLOG_INFO("~EventChannel:{}", fd);
    if (fd >= 0 && auto_close_) {
      ::close(fd);
      fd = -1;
    }
  }

protected:
  void tie(const std::shared_ptr<void> &obj) { tie_ = obj; }

  bool auto_close_ = true;

  /**
   * @brief 事件循环
   *
   */
  std::shared_ptr<EventLoop> loop;

  /**
   * 这是为了保护channel 存活的
   */
  std::weak_ptr<void> tie_;

  // 管道的超时时间
  struct timeval time {};

  /**
   * @brief 事件标志位
   *
   */
  unsigned char events = EV_PERSIST;

private:
  /**
   * @brief 描述符
   *
   */
  int fd = -1;

  int closeType = SHUT_RDWR;
};

} // namespace Core::Event
