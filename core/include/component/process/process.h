#pragma once

#include <signal.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "component/timer_channel.h"
#include "non_copyable.h"
#include "non_moveable.h"
#include "process_event.h"
namespace Core { namespace Component { namespace Process { class Manager; } } }
namespace Core { namespace Event { class EventLoop; } }
namespace Core { namespace OS { class CGroup; } }

namespace Core {
namespace Component {
namespace Process {

enum STATUS {
    UNKNOWN = 0,
    RUN = 1,
    RUNNING = 2,
    STOPPED = 3,
    STOPPING = 4,
    RELOAD = 5,
    RELOADING = 6,
    EXITED = 7,
    DELETING = 8,
    DELETED = 9
};


class Process :public Nonmoveable, public Noncopyable, public ProcessEvent {
public:
    Process(const std::string& command, const std::shared_ptr<Event::EventLoop>& loop);
    Process(const std::string& command, Event::EventLoop* loop);

    ~Process() override = default;

    bool execute();

    void onCreate() override {};
    void onStart() override {
      start_time_ = time(nullptr);
    };
    void onStop() override {};
    void onReload() override {};
    void onDestroy() override {};
    void onExit(Manager* m) override;

    bool stop() {
        if (pid_ <= 0) {
            return false;
        }
        status = STOPPING;
        auto ret = kill(pid_, SIGTERM);
        if (ret == -1) {
            return false;
        }
        return true;
    }

    bool syncStop() {
        if (pid_ <= 0) {
            return false;
        }
        status = STOPPING;
        auto ret = kill(pid_, SIGTERM);
        if (ret == -1) {
            return false;
        }
        int stat;
        pid_t pid = waitpid(-1, &stat, 0);
        if (pid <= 0) {
            return false;
        }
        return true;
    }


    bool reload() {
        if (pid_ <= 0) {
            return false;
        }
        status = RELOADING;
        auto ret = kill(pid_, SIGTERM);
        if (ret == -1) {
            return false;
        }
        return true;
    }

    bool remove() {
        if (pid_ <= 0) {
            return false;
        }
        status = DELETING;
        auto ret = kill(pid_, SIGTERM);
        if (ret == -1) {
            return false;
        }
        return true;
    }

    void setName(const std::string& name) {
        name_ = name;
    }

    std::string name() {
        return name_;
    }

    void setCGroup(std::shared_ptr<OS::CGroup> cgroup) {
        cgroup_ = std::move(cgroup);
    }

    STATUS getStatus() {
        return status;
    }

    void setStatus(STATUS s) {
        status = s;
    }

    pid_t getPid() {
        return pid_;
    }
    auto getStartTime() const {return start_time_;}
protected:
    std::unique_ptr<TimerChannel> timer;
private:

    void onRestore();

    std::string binary;
    std::vector<std::string> args;
    pid_t pid_ = 0;
    STATUS status = UNKNOWN;
    std::shared_ptr<OS::CGroup> cgroup_;
    std::string name_;
    uint8_t restoreCount = 0;
    Manager* m = nullptr;
    std::string command_;
    Event::EventLoop* loop_;
    time_t start_time_;
};
}
}
}
