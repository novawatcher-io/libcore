#pragma once
#include "pure.h"

namespace Core::Component::Process {
class Manager;
class ProcessEvent {
public:
  virtual void onCreate() PURE;
  virtual void onStart() PURE;
  virtual void onStop() PURE;
  virtual void onDestroy() PURE;
  virtual void onReload() PURE;
  virtual void onExit(Manager *manager) PURE;
  virtual ~ProcessEvent() = default;
};
} // namespace Core::Component::Process
