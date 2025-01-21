#pragma once
extern "C" {
#include <limits.h>
}
#include <string>

namespace Core::OS {

// 定义一个cpu occupy的结构体，用来存放CPU的信息

typedef struct _SysData {
  int pid;                      /** 进程 id. **/
  char exName[_POSIX_PATH_MAX]; /** 可执行文件名**/
  char state; /** 1 **/ /** R 代表正在运行, S 代表正在休眠, D 代表正处于不可中断的等待状态下的休眠，Z 代表进程未响应, T
                           代表进程已停止**/
  unsigned euid,        /** 有效的用户 id **/
      egid;             /** 有效的组 id */
  int ppid;             /** 父进程的 pid. **/
  int pgrp;             /** The pgrp of the process. **/
  int session;          /** 该进程的 session **/
  int tty;              /** The tty the process uses **/
  int tpgid;            /** (too long) **/
  unsigned int flags;   /** The flags of the process. **/
  unsigned int minflt;  /** 该任务不需要从硬盘拷数据而发生的缺页（次缺页）的次数 **/
  unsigned int cminflt; /**  累计的该任务的所有的waited-for进程曾经发生的次缺页的次数目 **/
  unsigned int majflt;  /** 该任务需要从硬盘拷数据而发生的缺页（主缺页）的次数 **/
  unsigned int cmajflt; /** 累计的该任务的所有的waited-for进程曾经发生的主缺页的次数目 **/
  int utime;            /** 该任务在用户态运行的时间，单位为jiffies **/
  int stime;            /** 内核态运行时间 **/
  int cutime;           /** user mode jiffies with childs **/
  int cstime;           /** kernel mode jiffies with childs **/
  int counter;          /** process's next timeslice **/
  int priority;         /** the standard nice value, plus fifteen **/
  unsigned int timeout; /** The time in jiffies of the next timeout **/
  unsigned int itrealvalue;      /** The time before the next SIGALRM is sent to the process **/
  int starttime; /** 20 **/      /** Time the process started after system boot **/
  unsigned int vsize;            /** Virtual memory size **/
  unsigned int rss;              /** Resident Set Size **/
  unsigned int rlim;             /** Current limit in bytes on the rss **/
  unsigned int startcode;        /** The address above which program text can run **/
  unsigned int endcode;          /** The address below which program text can run **/
  unsigned int startstack;       /** The address of the start of the stack **/
  unsigned int kstkesp;          /** The current value of ESP **/
  unsigned int kstkeip;          /** The current value of EIP **/
  int signal;                    /** The bitmap of pending signals **/
  int blocked; /** 30 **/        /** The bitmap of blocked signals **/
  int sigignore;                 /** The bitmap of ignored signals **/
  int sigcatch;                  /** The bitmap of catched signals **/
  unsigned int wchan; /** 33 **/ /** (too long) **/
  int sched,                     /** scheduler **/
      sched_priority;            /** scheduler priority **/
} SysData;

typedef struct _StatusData {
  uint64_t rss = 0;
  uint64_t vmSize = 0;
  float cpu = 0.0;
} StatusData;

/**
 * 这个类是从/proc/pid/stat下去读取值，之所以没用sysinfo去获取是因为需要用k8s，sysinfo不能拿到容器的
 */
class [[deprecated("use new proc reader")]] UnixSysInfo {
public:
  UnixSysInfo(pid_t pid) {
    procPath = "/proc/" + std::to_string(pid) + "/stat";
    procStatusPath = "/proc/" + std::to_string(pid) + "/status";
  }

  /**
   * 加载文件，从而获取到应用的系统信息
   * @return
   */
  bool load(SysData &data, StatusData &status);

private:
  std::string procPath;
  std::string procStatusPath;
  std::string statPath = "/proc/stat";
  uint64_t sysCpuRecordTime = 0;
  uint64_t procCpuRecordTime = 0;
};
} // namespace Core::OS
