#include "event/event_queue_handler.h"
#include "build_expect.h"
#include "event/event_queue.h"
#include <cstdio>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <vector>

namespace Core::Event {
void EventQueueHandler::dispatchTask(struct bufferevent *bev, Event::EventChannel * /*channel*/) {
  // 清空缓冲区，因为应用程序并不关注内核层面缓冲区里有什么
  size_t length = evbuffer_get_length(::bufferevent_get_input(bev));
  if (build_likely(length < BUFSIZ)) {
    std::vector<char> buf(length);
    bufferevent_read(bev, buf.data(), length);
  } else {
    char buffer[BUFSIZ];
    while ((evbuffer_remove(bufferevent_get_input(bev), buffer, sizeof(buffer))) > 0) {
    }
  }
  queue->dispatchTask();
}
} // namespace Core::Event
