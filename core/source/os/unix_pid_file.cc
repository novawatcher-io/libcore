#include "os/unix_pid_file.h"
#include <fcntl.h>
#include <spdlog/spdlog.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sstream>

namespace Core::OS {
UnixPidFile::UnixPidFile(const std::string &pidFile, int flag) {
    //检查pid文件
    pidFd = ::open(pidFile.c_str(), flag, 0777);
    if (pidFd == -1) {
        SPDLOG_ERROR("pid file({}) error:{}", pidFile, strerror(errno));
        exit(-1);
    }
}

bool UnixPidFile::tryWriteLock() {
    if (pidFd == -1) {
        SPDLOG_WARN("pidFile error!");
        exit(-1);
        return false;
    }
    fileLock.l_type = F_WRLCK;
    fileLock.l_whence = SEEK_SET;
    fileLock.l_start = 0;
    fileLock.l_len = 0;
    int res = fcntl(pidFd, F_SETLK, &fileLock);
    return !res;
}

pid_t UnixPidFile::getPid() {
    if (pidFd <= 0) {
        SPDLOG_ERROR("pidFile error!");
        exit(-1);
    }
    //持久化pid进程文件锁
    char buf[64];
    ssize_t res = ::read(pidFd, buf, sizeof(buf));
    if (res == -1) {
        SPDLOG_ERROR(strerror(errno));
        exit(-1);
    }
    buf[res] = '\0';
    pid_t  process_pid = atoi(buf);
    return process_pid;
}

pid_t UnixPidFile::setPid() {
    std::stringstream pidStr;
    pid = getpid();
    pidStr << pid;
    lseek(pidFd, 0, SEEK_SET);
    int ret = ftruncate(pidFd, 0);
    if (ret == -1) {
        SPDLOG_ERROR(strerror(errno));
    }

    //重新写入pid
    int res = write(pidFd, pidStr.str().c_str(), (pidStr.str().length()));

    if (res <= 0) {
        SPDLOG_ERROR(strerror(errno));
        exit(-1);
    }

    return pid;
}

UnixPidFile::~UnixPidFile() {
    if (pidFd > 0) {
        ::close(pidFd);
    }
}
}

