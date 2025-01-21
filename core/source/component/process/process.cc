#include "component/process/process.h"
#include "component/process/manager.h"
#include "os/unix_cgroup.h"
#include <bits/chrono.h>
#include <cerrno>
#include <spdlog/spdlog.h>
#include <sstream>
#include <sys/prctl.h>
#include <unistd.h>
#include <unordered_map>

namespace Core::Event {
class EventLoop;
}

namespace Core::Component::Process {

Process::Process(const std::string &command, const std::shared_ptr<Event::EventLoop> &loop)
    : Process(command, loop.get()) {}
Process::Process(const std::string &command, Event::EventLoop *loop)
    : timer(std::make_unique<TimerChannel>(loop,
                                           [this]() {
                                             onRestore();
                                             return;
                                           })),
      command_(command), loop_(loop) {
  std::stringstream ss(command);
  std::string token;
  while (ss >> token) {
    if (binary.empty()) {
      binary = token;
      args.push_back(token);
      continue;
    }
    args.push_back(token);
  }
}

bool Process::execute() {
  this->onCreate();
  pid_t pid = fork();
  status = RUNNING;
  if (pid == -1) {
    // fork失败
    return false;
  } else if (pid == 0) {
    std::vector<char *> argv(args.size() + 1);
    for (size_t i = 0; i < args.size(); i++) {
      // std::cout << args[i].c_str() << '\n';
      argv[i] = (char *)args[i].c_str();
    }
    argv[args.size()] = nullptr;
#if defined(__linux__)
    prctl(PR_SET_PDEATHSIG, SIGTERM);
#elif defined(__FreeBSD__)
    int sigid = SIGTERM;
    procctl(P_PID, 0, PROC_PDEATHSIG_CTL, &sigid);
#endif
    // 子进程
    int ret = execvp(binary.c_str(), argv.data());
    if (ret == -1) {
      SPDLOG_ERROR("execvp {} failed", binary);
      _exit(errno);
    }
    _exit(0);
  } else {
    // 父进程
    pid_ = pid;
    status = RUN;
    start_time_ = time(nullptr);
    if (cgroup_) {
      cgroup_->run();
      cgroup_->attach(pid);
    }
    this->onStart();
    return true;
  }
}

void Process::onExit(Manager *manager) {
  auto rCnt = restoreCount % 6;
  if (rCnt == 6) {
    restoreCount = 0;
    rCnt = 0;
  }
  restoreCount++;
  this->m = manager;
  auto timeout = (1 << rCnt);
  SPDLOG_INFO("process {} restart,after {}s", binary, timeout);
  timer->enable(std::chrono::milliseconds(timeout * 1000));
}

void Process::onRestore() {
  if (m == nullptr) {
    return;
  }
  std::unique_ptr<Process> process = std::make_unique<Process>(command_, loop_);
  process->restoreCount = restoreCount;
  process->setCGroup(cgroup_);
  auto iter = m->maps.find(getPid());
  if (iter != m->maps.end()) {
    m->maps.erase(iter);
  }

  if (!process->execute()) {
    return;
  }
  m->maps[process->getPid()] = std::move(process);
}
} // namespace Core::Component::Process
