#pragma once

#include <queue>
#include <mutex>
#include <memory>
#include <functional>
#include "non_copyable.h"

namespace Core {
namespace OS {
    class UnixThread;
}

namespace Event {

typedef std::function<void()> Task;

class EventQueue : public Core::Noncopyable {
public:
    explicit EventQueue(const std::shared_ptr<OS::UnixThread> &threadObject);

    /**
     * @brief 投递任务
     *
     * @param task
     */
    bool pushTask(const Task &task);

    /**
     * @brief 派遣任务处理
     *
     */
    void dispatchTask();

    ~EventQueue() = default;

private:
    /**
     * @brief 任务队列
     *
     */
    std::queue<Task> taskQueue;

    /**
     * @brief 锁
     *
     */
    std::mutex lock;

    /**
     * @brief 线程
     *
     */
    std::shared_ptr<OS::UnixThread> thread;
};
}
}