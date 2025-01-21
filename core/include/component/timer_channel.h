#pragma once

#include "os/unix_timer.h"

#include <chrono>

#include "event/event_buffer_channel.h"

namespace Core::Component {

/**
 * Utility helper functions for Timer implementation.
 */
class TimerUtils {
public:
    /**
     * Intended for consumption by enable(HR)Timer, this method is templated method to avoid implicit
     * duration conversions for its input arguments. This lets us have an opportunity to check bounds
     * before doing any conversions. When the passed in duration exceeds INT32_MAX max seconds, the
     * output will be clipped to yield INT32_MAX seconds and 0 microseconds for the
     * output argument. We clip to INT32_MAX to guard against overflowing the timeval structure.
     * Throws an EnvoyException on negative duration input.
     * @tparam Duration std::chrono duration type, e.g. seconds, milliseconds, ...
     * @param d duration value
     * @param tv output parameter that will be updated
     */
    template <typename Duration> static void durationToTimeval(const Duration& d, timeval& tv) {
        if (d.count() < 0) {
            throw fmt::format("Negative duration passed to durationToTimeval(): {}", d.count());
        };
        constexpr int64_t clip_to = INT32_MAX; // 136.102208 years
        auto secs = std::chrono::duration_cast<std::chrono::seconds>(d);
        if (secs.count() > clip_to) {
            tv.tv_sec = clip_to;
            tv.tv_usec = 0;
            return;
        }

        auto usecs = std::chrono::duration_cast<std::chrono::microseconds>(d - secs);
        tv.tv_sec = secs.count();
        tv.tv_usec = usecs.count();
    }
};

class TimerChannel : public OS::UnixTimer,
                     public Core::Noncopyable {
public:
    /**
     * @brief 定时器管道，单位是秒
     *
     * 主循环
     * @param loop_
     * 秒
     * @param millisecond
     */
    TimerChannel(const std::shared_ptr<Event::EventLoop>& loop, const std::function<void()>& cb)
        : TimerChannel(loop.get(), cb) {
    }
    TimerChannel(Event::EventLoop* loop, const std::function<void()>& cb) : cb_(cb), loop_(loop) {
        evtimer_assign(
            &raw_event_, loop_->getEventBase(),
            [](evutil_socket_t, short, void* arg) -> void {
                auto timer = static_cast<TimerChannel*>(arg);
                if (!timer) {
                    return;
                }

                if (!timer->cb_) {
                    return;
                }
                timer->cb_();
            },
            this);
    }

    void enable(const std::chrono::milliseconds duration) {
        struct timeval tv{};
        TimerUtils::durationToTimeval(duration, tv);

        event_add(&raw_event_, &tv);
    }

    bool enabled() {
        return 0 != evtimer_pending(&raw_event_, nullptr);
    }

    void disable() {
        event_del(&raw_event_);
    }

    ~TimerChannel() {
        event_del(&raw_event_);
    }

private:
    std::function<void()> cb_;
    Event::EventLoop* loop_;
    std::shared_ptr<Event::EventBufferChannel> channel_;
    struct event raw_event_;
};
} // namespace Core::Component
