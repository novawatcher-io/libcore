#include "event/event_buffer_channel.h"
#include <errno.h>
#include <spdlog/spdlog.h>
#include <string.h>
#include <iostream>
#include "build_expect.h"

namespace Core { namespace Event { class EventChannel; } }

namespace Core {
namespace Event {

void EventBufferChannel::update() {
//    tie(shared_from_this());
    loop->updateChannel(shared_from_this());
}


void EventBufferChannel::onRead(bufferevent *evClient, void *arg) {
    //管道事件触发
    auto channel = static_cast<EventBufferChannel *>(arg);
    if (!channel) {
        return;
    }
    channel->doReadCallable(evClient);
}


void EventBufferChannel::onWrite(bufferevent *evClient, void *arg) {
    //管道事件触发
    auto channel = static_cast<EventBufferChannel *>(arg);
    if (!channel) {
        return;
    }
    channel->doWriteCallable(evClient);
}

int closeCount;

void EventBufferChannel::onEvent(bufferevent *ev, short flag, void *arg) {
    //管道不能是空的
    if (build_unlikely(!arg)) {
#ifdef USE_DEBUG
        std::cout << "arg null" << std::endl;
#endif
        SPDLOG_ERROR("arg null");
        return;
    }

    //管道事件触发
    auto channel = static_cast<EventBufferChannel *>(arg);

    if (bufferevent_getfd(ev) != channel->getChannelFd()) {
        return;
    }

    if ((flag & BEV_EVENT_EOF)) {
        channel->close(ev, channel);
        return;
    }

    if (flag & BEV_EVENT_ERROR) {
        std::cerr << "BEV_EVENT_ERROR:" << strerror(errno) << std::endl;
        return;
    }

    if (flag & BEV_EVENT_READING) {
//        std::cerr << "BEV_EVENT_READING:" << strerror(errno) << std::endl;
    }

    if (flag & BEV_EVENT_WRITING) {
        std::cerr << "BEV_EVENT_WRITING" << std::endl;
        return;
    }

    if (flag & BEV_EVENT_TIMEOUT) {
        channel->close(ev, channel);
        return;
    }

    if (flag & BEV_EVENT_CONNECTED) {
//        std::cerr << "BEV_EVENT_CONNECTED" << std::endl;
    }

//    channel->handelEvent(flag);
}

void EventBufferChannel::close(bufferevent *evClient, EventChannel *arg) {
    if (eventOnClose)
        eventOnClose(evClient, arg);

    loop->deleteChannel(shared_from_this());
}

bool EventBufferChannel::eventSet(const std::shared_ptr<EventLoop> &/*loop*/) {
    //添加新事件
    int ret = 0;
    if (this->getTimer().tv_usec > 0) {
        struct timeval time;

        time.tv_sec = this->getTimer().tv_sec;
        time.tv_usec = this->getTimer().tv_usec;
        bufferevent_set_timeouts(bufptr.get(), &time, nullptr);
    }
    errno = 0;
    bufferevent_setcb(bufptr.get(), onRead, onWrite, onEvent, static_cast<void *>(this));
    bufferevent_enable(bufptr.get(), this->getEvents());
    return !ret;
}

// bool EventBufferChannel::handelEvent(short events) {
//    /**
//     * EPOLLIN 判断刻度
//     * EPOLLPRI 判断外带数据
//     * EPOLLRDHUP 对端关闭或者写一方关闭
//     */
// //    if (events & (EV_READ)) {
// //        if (eventOnRead) eventOnRead();
// //    }

// //    if (events & EV_WRITE) {
// //        if (eventOnWrite) eventOnWrite();
// //    }

// //    if (events & EV_CLOSED) {
// //        if (eventOnClose) eventOnClose();
// //    }

//     return true;
// }
}
}
