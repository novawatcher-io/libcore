#include <cerrno>
#include <fcntl.h>
#include <spdlog/spdlog.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <strings.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include "os/unix_sysinfo.h"
#include "os/unix_util.h"

namespace Core::OS {
bool UnixSysInfo::load(SysData& data, StatusData& status) {

    memset(&data, 0, sizeof(data));
    if (!OS::isFile(procPath)) {
        SPDLOG_WARN("{} is not file", procPath);
        return false;
    }

    int fd = open(procPath.c_str(), O_RDONLY|O_NONBLOCK);
    if (fd < 0) {
        SPDLOG_WARN( "open {} failed", procPath);
        return false;
    }

    std::string context;

    /**
     * 读取文件，这不会执行任何非阻塞, 除非被中断才会重新读
     */
    while(true) {
        char buf[BUFSIZ];
        ssize_t readSize = read(fd, buf, BUFSIZ);
        if (readSize == 0) {
            if (errno == EINTR) {
                continue;
            } else {
                break;
            }
        } else if (readSize < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                break;
            }
        }
        buf[readSize] = '\0';
        context.append(buf);
    }

    int ret = close(fd);
    if (ret != 0) {
        SPDLOG_WARN( "close {} failed", procPath);
    }
    //解析系统指标到内存
    ret = sscanf(context.c_str(),
                 "%d %s %c %d %d %d %d %d %u %u %u %u %u %d %d %d %d %d %d %u %u %d %u %u %u %u %u %u %u %u %d %d %d %d %u",
                 &(data.pid),
                 &(data.exName[0]),
                 &(data.state),
                 &(data.ppid),
                 &(data.pgrp),
                 &(data.session),
                 &(data.tty),
                 &(data.tpgid),
                 &(data.flags),
                 &(data.minflt),
                 &(data.cminflt),
                 &(data.majflt),
                 &(data.cmajflt),
                 &(data.utime),
                 &(data.stime),
                 &(data.cutime),
                 &(data.cstime),
                 &(data.counter),
                 &(data.priority),
                 &(data.timeout),
                 &(data.itrealvalue),
                 &(data.starttime),
                 &(data.vsize),
                 &(data.rss),
                 &(data.rlim),
                 &(data.startcode),
                 &(data.endcode),
                 &(data.startstack),
                 &(data.kstkesp),
                 &(data.kstkeip),
                 &(data.signal),
                 &(data.blocked),
                 &(data.sigignore),
                 &(data.sigcatch),
                 &(data.wchan)
          );
    if (ret != 35) {
        SPDLOG_WARN( "sscanf failed");
        return false;
    }
    fd = open(procStatusPath.c_str(), O_RDONLY|O_NONBLOCK);
    if (fd < 0) {
        SPDLOG_WARN( "open {} failed", procStatusPath);
        return false;
    }

    char lineBuf[BUFSIZ];
    char name[64];
    uint64_t memSize = 0;

    while(OS::readline(fd, lineBuf, BUFSIZ)) {
        sscanf(lineBuf,"%s %lu",name,&memSize);
        if (strcasecmp("VmRSS:", name) == 0) {
            status.rss = memSize;
        }

        if (strcasecmp("VmSize:", name) == 0) {
            status.vmSize = memSize;
        }
    }

    ret = close(fd);
    if (ret != 0) {
        SPDLOG_WARN( "close {} failed", procStatusPath);
    }

    //获取总cpu使用情况
    fd = open(statPath.c_str(), O_RDONLY|O_NONBLOCK);
    if (fd < 0) {
        SPDLOG_WARN( "open {} failed", statPath);
        return false;
    }

    std::string statContext;

    /**
     * 读取文件，这不会执行任何非阻塞, 除非被中断才会重新读
     */
    while(true) {
        char buf[BUFSIZ];
        ssize_t readSize = read(fd, buf, BUFSIZ);
        if (readSize == 0) {
            if (errno == EINTR) {
                continue;
            } else {
                break;
            }
        } else if (readSize < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                break;
            }
        }
        buf[readSize] = '\0';
        statContext.append(buf);
    }

    unsigned long user;
    unsigned long nice;
    unsigned long system;
    unsigned long idle;
    sscanf(statContext.c_str(), "%s %ld %ld %ld %ld",name,&user, &nice, &system,&idle);

    unsigned long procCpuTime = data.utime + data.stime + data.cutime + data.cstime;
    unsigned long sysTotalTime = (user + nice + system + idle);
    if (!procCpuRecordTime && !sysCpuRecordTime) {
        status.cpu = 0;
    } else {
        status.cpu = static_cast<float>((((static_cast<float>(procCpuTime - procCpuRecordTime))
                / static_cast<float >(sysTotalTime - sysCpuRecordTime))  * 100.0));
    }


    ret = close(fd);
    if (ret != 0) {
        SPDLOG_WARN( "close {} failed", procStatusPath);
    }

    procCpuRecordTime = procCpuTime;
    sysCpuRecordTime = sysTotalTime;
    return true;
}
}

