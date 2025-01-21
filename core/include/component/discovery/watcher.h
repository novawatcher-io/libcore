#pragma once

extern "C" {
#include <sys/inotify.h>
#include <unistd.h>
#include <fcntl.h>
}
#include <spdlog/spdlog.h>
#include <string>

namespace Core::Component::Discovery {
class Watcher {
public:
  Watcher() : fd_(inotify_init()) {
    int flags = fcntl(fd_, F_GETFL, 0);
    int ret = fcntl(fd_, F_SETFL, flags | O_NONBLOCK | O_CLOEXEC);
    if (ret == -1) {
      SPDLOG_ERROR("Failed to set inotify fd to non-blocking: {}", strerror(errno));
    }
  };

  bool Reg(const std::string &path, uint32_t mask) {
    wd_ = inotify_add_watch(fd_, path.c_str(), mask);
    if (wd_ == -1) {
      SPDLOG_ERROR("inotify_add_watch failed: errno={}, err message={}", errno, strerror(errno));
      return false;
    }
    return true;
  }

  int GetFd() const { return fd_; }

  ~Watcher() {
    if (wd_ != -1) {
      inotify_rm_watch(fd_, wd_);
    }
    if (fd_ != -1) {
      close(fd_);
    }
  };

private:
  int fd_ = -1;
  int wd_ = -1;
};
} // namespace Core::Component::Discovery
