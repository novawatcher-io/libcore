/**
******************************************************************************
* @file           : component.cc
* @author         : zhanglei
* @brief          : None
* @attention      : None
* @date           : 2024/2/11
******************************************************************************
*/
//
// Created by zhanglei on 2024/2/11.
//

#pragma once
#include <utility>

#include "component/api.h"
#include "event/event_buffer_channel.h"
#include "event/event_channel.h"

#include "watcher.h"

namespace Core::Component::Discovery {
class Component : public Core::Component::Component {
public:
  explicit Component(const std::string &config_path) : watcher_(std::make_unique<Watcher>()) {
    if (!watcher_->Reg(config_path, IN_CREATE | IN_MODIFY | IN_DELETE)) {
      SPDLOG_ERROR("watcher->reg failed({}):{}", errno, strerror(errno));
      return;
    }
    SPDLOG_INFO("watch file: {}", config_path);
  }

  void init() override {};

  void start() override {};

  void stop() override { watcher_ = nullptr; };

  void finish() override {};

  std::string name() override { return "discovery"; }

  void setListener(std::shared_ptr<ConfigListener> listener) { listener_ = std::move(listener); }

  std::shared_ptr<Event::EventChannel> channel(const std::shared_ptr<Event::EventLoop> &loop) {
    auto channel = std::make_shared<Event::EventBufferChannel>(loop, watcher_->GetFd());
    channel->bindOnReadCallable(
        [&](struct bufferevent *bev, Core::Event::EventChannel *) {
          struct evbuffer *input = bufferevent_get_input(bev);
          size_t length = evbuffer_get_length(input);
          size_t i = 0;
          while (i < length) {
            auto p = evbuffer_pullup(input, sizeof(struct inotify_event));
            if (p == nullptr) {
              SPDLOG_ERROR("rest of data not enough, rest={}", length - i);
              return;
            }
            auto *event = (struct inotify_event *)p;
            if (event->mask & IN_CREATE) {
              if (listener_) listener_->onCreate();
            }
            if (event->mask & IN_MODIFY) {
              if (listener_) listener_->onUpdate();
            }
            if (event->mask & IN_DELETE) {
              if (listener_) listener_->onDelete();
            }
            auto used = sizeof(struct inotify_event) + event->len;
            evbuffer_drain(input, used);
            i += used;
          }
        },
        -1);
    return channel;
  }

private:
  std::unique_ptr<Watcher> watcher_;
  std::shared_ptr<ConfigListener> listener_;
};
} // namespace Core::Component::Discovery
