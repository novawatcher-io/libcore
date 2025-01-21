/**
******************************************************************************
* @file           : unix_sigset.h
* @author         : zhanglei
* @brief          : None
* @attention      : None
* @date           : 2024/2/7
******************************************************************************
*/
//
// Created by zhanglei on 2024/2/7.
//

#pragma once
extern "C" {
#include <errno.h>
#include <signal.h>
}

#include <iostream>
#include <spdlog/spdlog.h>

namespace Core {
namespace OS {
class UnixSigSet {
public:
  UnixSigSet() { sigfillset(&set); }

  void remove(int sig) {
    int ret = sigdelset(&set, sig);
    if (ret == -1) {
      SPDLOG_ERROR("sigdelset has error");
    }
  }

  void block() {
    int ret = sigprocmask(SIG_BLOCK, &set, nullptr);
    if (ret == -1) {
      SPDLOG_ERROR("sigprocmask has error");
    }
    return;
  }

  void unblock() { sigprocmask(SIG_UNBLOCK, &set, &oldSet); }

private:
  sigset_t set = {};
  sigset_t oldSet = {};
};
} // namespace OS
} // namespace Core
