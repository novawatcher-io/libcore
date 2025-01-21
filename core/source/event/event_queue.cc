#include "event/event_queue.h"
#include <spdlog/spdlog.h>
#include <iostream>
#include "build_expect.h"
#include "os/unix_current_thread.h"
#include "os/unix_thread.h"


namespace Core::Event {

    EventQueue::EventQueue(const std::shared_ptr<OS::UnixThread> &threadObject) : thread(threadObject) {};

    bool EventQueue::pushTask(const Task &task) {
        if (build_unlikely((OS::UnixCurrentThread::tid() == thread->getTid()))) {
            task();
            return true;
        } else {
            {
                std::lock_guard<std::mutex> guard(lock);
                if (taskQueue.size() > 500) {
                    SPDLOG_ERROR("taskQueue is full");
                    std::cerr << "taskQueue is full, queue size:" << taskQueue.size() << std::endl;
                    return false;
                }
                taskQueue.push(task);
            }

            //唤醒线程
            thread->wakeUp();
            return true;
        }
    }

    void EventQueue::dispatchTask() {
        //交换地址
        std::queue<Task> mainTaskQueue;
        {
            std::lock_guard<std::mutex> guard(lock);
            mainTaskQueue.swap(taskQueue);
        }

        //循环遍历
        while (!mainTaskQueue.empty()) {
            auto cb = mainTaskQueue.front();
            cb();
            mainTaskQueue.pop();
        }
    }
}

