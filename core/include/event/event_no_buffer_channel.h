#pragma once

#include <event2/event.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/types.h>
#include <functional>
#include <iostream>
#include <memory>
#include "event_channel.h"
#include "event_smart_ptr.h"
#include "non_copyable.h"
namespace Core { namespace Event { class EventLoop; } }
struct bufferevent;

namespace Core {
namespace Event {

typedef std::function<void(EventChannel *channel)> EventNoBufferCallable;

class EventNoBufferChannel : public EventChannel, public Core::Noncopyable {
public:

    EventNoBufferChannel(const std::shared_ptr<EventLoop> &loop_, int fd, bool auto_close_ = true);

    /**
     * @brief Set the On Read Callable object
     *
     * @param callable
     * @return true
     * @return false
     */
    bool setOnReadCallable(const EventNoBufferCallable &callable) {
        eventOnRead = callable;
        return true;
    }


    /**
     * @brief 绑定可读事件
     *
     * @param callable
     * 事件循环超时事件
     * @param second
     * @return true
     * @return false
     */
    bool bindOnReadCallable(const EventNoBufferCallable &callable, int second) {
        eventOnRead = callable;
        enableReading(second);
        return true;
    }

    /**
     * @brief Get the On Read Callable object
     *
     * @return EventCallable
     */
    EventNoBufferCallable &getOnReadCallable() {
        return eventOnRead;
    }

    /**
     * @brief Get the On Close Callable object
     *
     * @return EventCallable
     */
    EventNoBufferCallable getOnCloseCallable() {
        return eventOnClose;
    }

    /**
     * @brief event 事件循环
     *
     * @param flag
     * @return true
     * @return false
     */
    bool handelEvent(short flag);

    /**
     * @brief Set the On Close Callable object
     *
     * @param callable
     * @return true
     * @return false
     */
    bool setOnCloseCallable(const EventNoBufferCallable &callable) {
        eventOnClose = callable;
        return true;
    }

    /**
     * @brief 绑定关闭事件
     *
     * 事件
     * @param callable
     * 超时事件
     * @param second
     * @return true
     * @return false
     */
    bool bindOnCloseCallable(const EventNoBufferCallable &callable, int second) {
        eventOnClose = callable;
        enableReading(second);
        return true;
    }

    /**
     * @brief Set the On Error Callable object
     *
     * @param callable
     * @return true
     * @return false
     */
    bool setOnErrorCallable(const EventNoBufferCallable &callable) {
        eventOnError = callable;
        return true;
    }

    /**
     * @brief 绑定出错事件
     *
     * 事件
     * @param callable
     * 超时事件
     * @param second
     * @return true
     * @return false
     */
    bool bindOnErrorCallable(const EventNoBufferCallable &callable, int second) {
        eventOnError = callable;
        enableReading(second);
        return true;
    }

    /**
     * @brief Set the On Write Callable object
     *
     * @param callable
     * @return true
     * @return false
     */
    bool setOnWriteCallable(const EventNoBufferCallable &callable) {
        eventOnWrite = callable;
        return true;
    }

    /**
     * @brief 绑定写入事件
     *
     * @param callable
     * @param second
     * @return true
     * @return false
     */
    bool bindOnWriteCallable(const EventNoBufferCallable &callable, int second) {
        eventOnWrite = callable;
        enableWriting(second);
        return true;
    }

    /**
     * @brief Get the On Write Callable object
     *
     * @return EventCallable
     */
    EventNoBufferCallable &getOnWriteCallable() {
        return eventOnWrite;
    }


    /**
     * @brief Get the Events object
     *
     * @return uint32_t
     */
    uint32_t getEvents() {
        return events;
    }

    /**
     * @brief 开启可读事件单位是秒, -1是不超时
     *
     * @param second
     * @return true
     * @return false
     */
    bool enableReading(double second) {
        events = EV_PERSIST;
        time.tv_sec = 0;
        time.tv_usec = second * 1000 * 1000;
        events |= EV_READ;
        events |= EV_CLOSED;
        update();
        return true;
    }

    /**
     * @brief 开启可读事件单位是秒, -1是不超时
     *
     * @param second
     * @return true
     * @return false
     */
    bool enableWriting(double second) {
        events = EV_PERSIST;
        time.tv_sec = 0;
        time.tv_usec = second * 1000 * 1000;
        events |= EV_WRITE;
        events |= EV_CLOSED;
        update();
        return true;
    }

    /**
    * @brief 开启可读事件单位是秒, -1是不超时
    *
    * @param second
    * @return true
    * @return false
    */
    bool enable(double second) {
        time.tv_sec = 0;
        time.tv_usec = second * 1000 * 1000;
        events |= EV_READ;
        events |= EV_WRITE;
        update();
        return true;
    }

    /**
   * @brief 开启可读事件单位是秒, -1是不超时
   *
   * @param second
   * @return true
   * @return false
   */
    bool enable(unsigned char events_, __time_t second) {
        time.tv_sec = second;
        time.tv_usec = 0;
        events = events_;
        update();
        return true;
    }

    //禁用可读事件
    bool disableReading() {
        events &= ~EV_READ;
        update();
        return true;
    }

    /**
     * @brief 禁用写入事件
     *
     * @return true
     * @return false
     */
    bool disableWriting() {
        events &= ~EV_WRITE;
        update();
        return true;
    }


    /**
     * @brief 更新管道
     *
     */
    void update();

    void die(const std::shared_ptr<EventChannel> &channel) {
        tie = channel;
    }

//    /**
//     * @brief Get the Timer object
//     *
//     * @return struct timeval&
//     */
//    struct timeval& getTimer() {
//        return time;
//    }

    bool eventSet(const std::shared_ptr<EventLoop> &loop) override;

    virtual ~EventNoBufferChannel() override {
        int ret = event_del(ptr.get());
        if (ret == 0) {
            std::cout << "event_del success" << std::endl;
        }
        std::cout << "~EventNoBufferChannel" << std::endl;
    }

private:

//    /**
//     * @brief
//     * 可读回调
//     * @param evClient
//     * @param arg
//     */
//    static void onRead(bufferevent* evClient, void* arg);

    /**
     * @brief
     * 可写回调
     */
    static void onWrite(bufferevent *evClient, void *arg);

    static void onEvent(int fd, short events, void *arg);


    /**
     * @brief 事件
     *
     */
    EventNoBufferCallable eventOnRead;
    EventNoBufferCallable eventOnClose;
    EventNoBufferCallable eventOnWrite;
    EventNoBufferCallable eventOnError;

    //保证生命周期
    std::shared_ptr<EventChannel> tie;

    /**
     * @brief 智能指针，防止内存泄漏
     *
     */
    Event::EventPtr ptr;
};
}
}
