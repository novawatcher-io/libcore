#pragma once

namespace Core {
class Noncopyable {
public:
  Noncopyable(const Noncopyable &) = delete;
  Noncopyable(Noncopyable &) = delete;
  Noncopyable &operator=(const Noncopyable &) = delete;

  Noncopyable(Noncopyable &&) = default;
  Noncopyable &operator=(Noncopyable &&) = default;

protected:
  Noncopyable() = default;
  ~Noncopyable() = default;
};
} // namespace Core