#pragma once

#include <string>

#include "component/api.h"

#ifndef HTTP_MANAGER_NAME
#define HTTP_MANAGER_NAME "http"
#endif

namespace Core {
namespace Http {

class HttpConfig :public Core::Component::BaseConfig {

public:
    HttpConfig() = default;

    ~HttpConfig() = default;


    uint16_t getPort() {
        return port;
    }

    std::string getIp() {
        return ip;
    }

    int getTimeout() {
        return timeout;
    }

    void onUpdate() {};
    void onCreate() {};
    void onDelete() {};

    void setPort(uint16_t port_) {
        port = port_;
    }

    void setIp(const std::string& ip_) {
        ip = ip_;
    }

    void setTimeout(int timeout_) {
        timeout = timeout_;
    }


private:
    /**
     * @brief 端口
     * 
     */
    uint16_t port = 11900;

    /**
     * @brief ip
     * 
     */
    std::string ip = "0.0.0.0";

    /**
     * @brief 超时时间
     * 
     */
    int timeout = 5;

    //配置中的模块名字
    std::string name = "http";
};
}
}
