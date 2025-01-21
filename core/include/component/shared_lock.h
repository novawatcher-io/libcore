#pragma once
#include <shared_mutex>

namespace Core::Component {

[[deprecated("SharedLock is deprecated. Use std::shared_mutex directly.")]]
class SharedLock {
public:
  void shared_lock() { mutex_.lock_shared(); }

  void shared_unlock() { mutex_.unlock_shared(); }

  void mutex_lock() { mutex_.lock(); }

  void mutex_unlock() { mutex_.unlock(); }

private:
  std::shared_mutex mutex_;
};
} // namespace Core::Component
