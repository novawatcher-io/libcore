#pragma once
#include "event_queue.h"
#include <memory>
struct bufferevent; // IWYU pragma: keep
namespace Core::Event {
class EventChannel; // IWYU pragma: keep
}
namespace Core::OS {
class UnixThread; // IWYU pragma: keep
}

namespace Core::Event {

class EventQueueHandler {
public:
  explicit EventQueueHandler(const std::shared_ptr<OS::UnixThread> &threadObject)
      : queue(std::make_unique<EventQueue>(threadObject)){};

  /**
   * @brief 投递任务
   *
   * @param task
   */
  bool pushTask(const Task &task) { return queue->pushTask(task); }

  /**
   * @brief 派遣任务处理
   *
   */
  void dispatchTask(struct bufferevent *bev, Event::EventChannel *channel);

private:
  std::unique_ptr<EventQueue> queue;
};
} // namespace Core::Event
