#include "component/thread_container.h"
#include <spdlog/spdlog.h>
#include <utility>
#include "os/unix_countdown_latch.h"


namespace Core::Component {

void UnixThreadContainer::start() {
    std::shared_ptr<OS::UnixCountDownLatch> latch = std::make_shared<OS::UnixCountDownLatch>(size());
    for(auto &item : container) {
      item.second->addInitCallable([latch] {
            latch->down();
        });
        if (!item.second->start()) {
            SPDLOG_ERROR("create thread failed!");
        }
    }
    latch->wait();
}

void UnixThreadContainer::stop() {
    for(auto &item : container) {
      item.second->stop();
    }
}

bool UnixThreadContainer::reg(pid_t index, const std::shared_ptr<OS::UnixThread>& thread) {
    auto iter = container.find(index);
    if (iter != container.end()) {
        SPDLOG_ERROR("Thread has been registered!");
        return false;
    }

    container[index] = thread;
    return true;
}

bool UnixThreadContainer::task(pid_t index, const Event::Task& task) {
    auto iter = container.find(index);
    if (iter == container.end()) {
        SPDLOG_ERROR("No thread index found!");
        return false;
    }

    iter->second->addTask(task);
    return true;
}

void UnixThreadContainer::broadcastEvent(const Event::Task& task) {
    for(auto & iter : container) {
        iter.second->addTask(task);
    }
}

std::shared_ptr<OS::UnixThread>& UnixThreadContainer::getThread(int index) {
     auto iter = container.find(index);
     if (iter == container.end()) {
         return empty;
     }
     return iter->second;
}
}

