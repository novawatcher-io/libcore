#pragma once
#include <openssl/ssl.h>            // for SSL_CTX_free, SSL_free
#include <openssl/types.h>          // for SSL, SSL_CTX, X509_STORE
#include <stddef.h>                 // for size_t
#include <stdint.h>                 // for uint32_t, int64_t
#include <map>                      // for map
#include <memory>                   // for unique_ptr
#include <string>                   // for basic_string, string
#include <unordered_map>            // for unordered_map
#include "event/event_smart_ptr.h"  // for EventSmartPtr, EventConnPtr, Even...
#include "non_copyable.h"           // for Noncopyable
namespace Core { namespace Http { class HttpClientResponse; } }
struct bufferevent;



namespace Core::Http {

class HttpClient;
/**
 * 请求要传递的参数
 */
struct CallbackArgs {
    std::string method = "get";
    /**
     * 回调地址
     */
    void (*cb_)(HttpClientResponse& response) = nullptr;
    /**
     * 记录开始时间
     */
    int64_t beginTime = 0;

    /**
     * 请求路径
     */
    const char* path = nullptr;

    /**
     * 请求主机
     */
    const char *host = nullptr;

    /**
     * 请求索引
     */
    uint32_t index = 0;

    HttpClient* obj = nullptr;

    /**
     * 自定义要传入的参数
     */
    void *args = nullptr;

    void loadRequestMetric(size_t length);

};

enum HttpClientStatus {
    HTTP_CLIENT_NO_REQUESTING = 0,
    HTTP_CLIENT_REQUESTING = 1,
    HTTP_CLIENT_FINISH = 2,
    HTTP_CLIENT_ERROR = 3,
    HTTP_CLIENT_CLOSE = 4
};

using SslPtr = Event::EventSmartPtr<SSL, SSL_free>;
using SslCTXPtr = Event::EventSmartPtr<SSL_CTX, SSL_CTX_free>;
/**
 * Http客户端,这个类不是线程安全的，不要去在多线程中调用
 */
class HttpClient :public  Core::Noncopyable {
public:
    HttpClient(const std::string& requestUrl_, void (*cb_)(HttpClientResponse&), int timeout_ = 60);

    void setHeader(std::map<std::string, std::string>& header) {
        headers = header;
    }

    /**
     * 发送get请求
     */
    bool get(void* arg = nullptr);

    /**
     * 发送post 请求
     * @return
     */
    bool post(unsigned char* data, size_t length, void* arg);

    /**
     * 删除回调参数，这个方法仅限 HttpClient 自己用
     * @param index
     */
    void releaseArgs(uint32_t index);

    void setStatus(HttpClientStatus status_) {
        status = status_;
    }

    HttpClientStatus getStatus() {
        return status;
    }

    /**
     * 释放掉的时候要清空 obj,
     * 因为释放后会触发 connection_close_cb， 在这里如果obj是空的，那么会释放掉closeArgs参数
     */
    ~HttpClient();

private:

    /**
     * 客户端初始化
     */
    void init();

    /**
     * http请求的url
     */
    std::string requestUrl;

    /**
     * 请求完成的回调
     */
    void (*cb)(HttpClientResponse&) = nullptr;

    /**
     * header请求
     */
    std::map<std::string, std::string> headers;

    std::string uri;

    std::string http_host;
    /**
     * http connection
     */
    Event::EventConnPtr conn;

    Event::EventHttpUriPtr http_uri;

    std::string contentType = "text/plain";

    int port;

    const char *scheme = nullptr;

    const char* host = nullptr;

    int timeout = 0;

    const char* query = nullptr;

    enum HttpClientStatus status = HTTP_CLIENT_NO_REQUESTING;

    //请求的参数
    uint32_t requestIndex = 0;

    //参数列表，用来存放参数的池子
    std::unordered_map<uint32_t, std::unique_ptr<CallbackArgs>> argsPool;

    CallbackArgs closeArgs;

    X509_STORE *store = nullptr;

    bufferevent* bev = nullptr;

    SslPtr ssl;

    SslCTXPtr ssl_ctx;
};
}
