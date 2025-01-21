#include "os/unix_thread.h"
#include <event2/event.h>                // for EV_PERSIST
#include <spdlog/spdlog.h>               // for SPDLOG_ERROR
#include <stdint.h>                      // for uint64_t
#include <stdlib.h>                      // for exit
#include <sys/syscall.h>                 // for SYS_gettid
#include <unistd.h>                      // for syscall, write
#include <iostream>                      // for char_traits, basic_ostream
#include <utility>                       // for forward, move
#include "build_expect.h"                // for build_unlikely
#include "event/event_buffer_channel.h"  // for EventBufferChannel
#include "event/event_loop.h"            // for EventLoop
#include "os/unix_thread_proc.h"         // for UnixThreadProc
#include "os/unix_util.h"                // for set_thread_name

namespace Core {
namespace OS {

UnixThread::UnixThread() :
loop((std::make_shared<Event::EventLoop>())),
daemonize(false),
mTerminated(false),
isSuspend(false),
mRunStatus(false),
wakeupChannelFd(createChannelFd()) {}

int UnixThread::ThreadProc(std::unique_ptr<UnixThreadProc> proc, const std::shared_ptr<UnixThread>& object) {
    if (!object->getName().empty()) {
        OS::set_thread_name(object->getName().c_str());
    }
    object->setTid(syscall(SYS_gettid));
    proc->runThread();
    object->destroy();
    return 0;
}

void UnixThread::stop() {
    queue->pushTask(std::bind(&Event::EventLoop::quit, loop));
    if ((mRunStatus) && ((!daemonize))) {
        join();
        mRunStatus = false;
    }
}

ssize_t UnixThread::wakeUp() {
    uint64_t notify = 1;
    ssize_t n = ::write(wakeupChannelFd, &notify, sizeof(notify));
    if (n != sizeof(notify)) {
        std::string error("thread ");
        error += std::to_string(getTid()) + " wake up " + std::to_string(wakeupChannelFd) + " failed !";
        SPDLOG_ERROR(error);
    }
    return n;
}

bool UnixThread::start() {
    // //构建队列
    queue = std::make_shared<Event::EventQueueHandler>(shared_from_this());

    if (build_unlikely(wakeupChannelFd == -1)) {
        SPDLOG_ERROR("wakeupChannelFd create failed!");
        exit(-1);
    }

    std::unique_ptr<UnixThreadProc> proc = std::make_unique<UnixThreadProc>(loop, shared_from_this());
    channel = std::make_shared<Event::EventBufferChannel>(loop, wakeupChannelFd);
    channel->setEvents(EV_PERSIST);
    channel->setOnReadCallable(([this](auto && PH1, auto && PH2)
    {
        std::cout << "onTask" << std::endl;
        queue->dispatchTask(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); }));
    // //开启读取事件
    channel->enableReading(-1);
    threadHandle = std::make_unique<std::thread>(ThreadProc, std::move(proc), shared_from_this());

    mRunStatus = true;
    return true;
}
}
}
