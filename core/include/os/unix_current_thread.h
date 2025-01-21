#pragma once

#include <sys/syscall.h>   // for SYS_gettid
#include <unistd.h>        // for syscall
#include <memory>          // for shared_ptr
#include "build_expect.h"  // for build_unlikely
namespace Core { namespace Event { class EventLoop; } }
namespace Core { namespace OS { class UnixThread; } }

namespace Core {
namespace OS {

namespace UnixCurrentThread {

extern thread_local int t_cachedTid;

extern thread_local std::shared_ptr<Core::Event::EventLoop> currentLoop;

extern thread_local std::shared_ptr<UnixThread> currentThread;


inline int tid() {
    if (build_unlikely((t_cachedTid == 0))) {
        t_cachedTid = syscall(SYS_gettid);
    }
    return t_cachedTid;
}

inline Event::EventLoop *loop() {
    return currentLoop.get();
}

inline std::shared_ptr<Event::EventLoop> &loopSmartPtr() {
    return currentLoop;
}

void setCurrentThread(std::shared_ptr<UnixThread> &thread_);


inline std::shared_ptr<UnixThread> &getThread() {
    return currentThread;
}
}
}
}
