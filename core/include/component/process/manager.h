#pragma once

#include "component/api.h"    // for Component
#include "event/event_loop.h" // for EventLoop
#include "non_copyable.h"     // for Noncopyable
#include "non_moveable.h"     // for Nonmoveable
#include "os/unix_sigset.h"   // for UnixSigSet
#include "process.h"          // for Process
#include <event2/util.h>      // for evutil_socket_t
#include <memory>             // for unique_ptr, shared_ptr, __shared_ptr_a...
#include <signal.h>           // for SIGCHLD
#include <string>             // for basic_string, operator==, string
#include <sys/types.h>        // for pid_t
#include <unordered_map>      // for unordered_map, operator!=, _Node_iterator
#include <utility>            // for move, pair

namespace Core {
namespace Component {
namespace Process {
class Manager : public Component, public Noncopyable, public Nonmoveable {
public:
  Manager() : loop(std::make_shared<Event::EventLoop>()), sigset(std::make_unique<OS::UnixSigSet>()) {}
  explicit Manager(std::shared_ptr<Event::EventLoop> loop)
      : loop(std::move(loop)), sigset(std::make_unique<OS::UnixSigSet>()) {}

  void init() override {}

  void start() override {
    sigset->remove(SIGCHLD);
    sigset->block();
    loop->sigAdd(SIGCHLD, onExit, this);
    loop->loop();
  }

  void stop() override {
    for (auto &item : maps) {
      item.second->syncStop();
    }
  }

  static void onExit(evutil_socket_t sig, short events, void *param);

  void onCreate() {}
  void onStart() {}
  void onStop() {}
  void onDestroy() {}

  bool addProcess(std::unique_ptr<Process> process) {
    if (!process) {
      return false;
    }
    auto iter = maps.find(process->getPid());
    if (iter != maps.end()) {
      return false;
    }
    maps[process->getPid()] = std::move(process);
    return true;
  }

  std::unordered_map<pid_t, std::unique_ptr<Process>> &all() { return maps; }

  void stopProcess(const std::string &name) {
    for (auto &[pid, process] : maps) {
      if (process->name() == name) {
        process->stop();
      }
    }
  }

  Event::EventLoop *getLoop() const { return loop.get(); }

protected:
  friend class Process;
  std::shared_ptr<Event::EventLoop> loop;
  std::shared_ptr<OS::UnixSigSet> sigset;
  std::unordered_map<pid_t, std::unique_ptr<Process>> maps;
};
} // namespace Process
} // namespace Component
} // namespace Core
