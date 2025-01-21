#pragma once

extern "C" {
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
}
#include <string>
#include "build_expect.h"
#include "os/unix_util.h"
#include <spdlog/spdlog.h>

namespace Core {
namespace OS {

/**
* @brief UDP 服务器
*
*/
class UnixUdpServer {
public:
    UnixUdpServer(const std::string &ip, uint16_t port) {
        fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (build_unlikely(fd <= 0)) {
            SPDLOG_ERROR(strerror(errno));
            return;
        }

        /**
         * @brief 设置套接字选项，可以复用ip和端口
         *
         */
        int opt = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
        set_noblock(fd);
        startListen(ip, port);
    }

    /**
     * @brief 监听ip和端口
     *
     * @param ip
     * @param port
     * @return true
     * @return false
     */
    bool startListen(const std::string &ip, uint16_t port) {
        struct sockaddr_in servaddr;
        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(port);
        int ret = inet_pton(AF_INET, ip.c_str(), (void *) &servaddr.sin_addr);
        if (build_unlikely(ret != 1)) {
            SPDLOG_ERROR(strerror(errno));
            return false;
        }

        ret = bind(fd, (struct sockaddr *) &servaddr, sizeof(servaddr));
        if (build_unlikely(ret != 0)) {
            SPDLOG_ERROR(strerror(errno));
            return false;
        }

        return true;
    }

    /**
     * @brief 获取描述符
     *
     * @return int
     */
    int getFd() {
        return fd;
    }

    /**
     * @brief 不要在析构函数中去close fd，因为他们最后会在channel中被关闭
     *
     */
    virtual ~UnixUdpServer() {

    }

private:
    /**
     * @brief 描述符
     *
     */
    int fd = -1;
};
}
}
