#pragma once
#include <fcntl.h>         // for flock
#include <sys/types.h>     // for pid_t
#include <cerrno>          // for errno
#include <cstring>         // for strerror
#include <string>          // for basic_string, string
#include "non_copyable.h"  // for Noncopyable

namespace Core {
namespace OS {
/**
* 这个类是禁止复制的，因为这个类含有描述符，为了避免被复制出现副本close 关闭不掉
*/
class UnixPidFile : public Core::Noncopyable {
public:
    UnixPidFile(const std::string &pidFile, int flag);

    /**
     * 锁住文件
     * @param fileDescription
     * @param type
     * @return
     */
    bool tryWriteLock();

    /**
     * 打开pid文件获取句柄
     * @param pidFile
     * @param flag
     * @return
     */
    int open();

    /**
     * @brief 获取错误码
     *
     * @return int
     */
    int getErrno() {
        return errno;
    }

    /**
     * @brief 获取错误信息
     *
     * @return std::string
     */
    std::string getErrorMsg() {
        return strerror(errno);
    }

    pid_t setPid();

    pid_t getPid();

    ~UnixPidFile();

private:
    //文件锁
    struct flock fileLock{};

    //要守护的文件描述符
    int pidFd = -1;

    //pid的号码
    pid_t pid = 0;
};
}
}
