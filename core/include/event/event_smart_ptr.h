#pragma once

extern "C" {
#include <event.h>
#include <event2/bufferevent.h>
#include <event2/http.h>
#include <event2/listener.h>
#include <evhttp.h>
}
#include <memory>

namespace Core::Event {

/**
 * @brief 重写unique_ptr为了更好的管理libevent指针
 *
 * @tparam T
 * @tparam (*deleter)(T*)
 */
template <class T, void (*deleter)(T *)> class EventSmartPtr {
public:
  EventSmartPtr() : ptr_(nullptr, deleter){};

  explicit EventSmartPtr(T *object) : ptr_(object, deleter) {}
  void Reset(T *object) { ptr_.reset(object); }
  auto get() -> T * { return ptr_.get(); }
  explicit operator bool() { return ptr_.operator bool(); }

private:
  std::unique_ptr<T, decltype(deleter)> ptr_;
};

/**
 * @brief 定义libevent的智能指针类型
 *
 */
using EventBasePtr = EventSmartPtr<event_base, event_base_free>;
using EventBufferPtr = EventSmartPtr<evbuffer, evbuffer_free>;
using EventHttpPtr = EventSmartPtr<evhttp, evhttp_free>;
using EventPtr = EventSmartPtr<event, event_free>;
using EventBufferEventPtr = EventSmartPtr<::bufferevent, bufferevent_free>;
using EventListenPtr = EventSmartPtr<evconnlistener, evconnlistener_free>;
using EventHttpUriPtr = EventSmartPtr<evhttp_uri, evhttp_uri_free>;
using EventConnPtr = EventSmartPtr<evhttp_connection, evhttp_connection_free>;
using EventConnPtr = EventSmartPtr<evhttp_connection, evhttp_connection_free>;
} // namespace Core::Event
