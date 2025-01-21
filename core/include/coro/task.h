#pragma once

#include <ucontext.h>
namespace Core {
namespace Coroutine {

/**
 * 协程上下文处理,这个类使用起来不是线程安全的
 */
class Task {
public:

    bool makeTaskContext() {
        sigset_t zero;
        sigemptyset(&zero);
        sigprocmask(SIG_BLOCK, &zero, &task_context.uc_sigmask);

        if (getcontext(&task_context) < 0)
        {
            return false;
        }

        task_context.uc_stack.ss_sp = (unsigned char*)(this + 1);
        task_context.uc_stack.ss_size = defaultStackSize;
        task_context.uc_link = &schedule_context;

        return true;
    }

    bool makeScheduleContext() {
        sigset_t zero;
        sigemptyset(&zero);
        sigprocmask(SIG_BLOCK, &zero, &task_context.uc_sigmask);

        if (getcontext(&task_context) < 0)
        {
            return false;
        }

//        makecontext(&task_context);
        return true;
    }



    void yield(Task *t)
    {
        swapcontext(&task_context, &schedule_context);
    }

    /**
     * 恢复上下文
     */
    void resume() {
        swapcontext(&schedule_context, &task_context);
    }

private:
    /**
     * 检查协程调度的task是否是在活跃状态，不在活跃状态要激活
     */
    bool active = false;

    /**
     * 默认栈大小
     */
    uint32_t defaultStackSize = 1024 * 1024;

    /**
     * 调度上下文
     */
    ucontext_t schedule_context;

    /**
     * 任务上下文
     */
    ucontext_t task_context;
};

}
}
