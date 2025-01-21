#pragma once

#include <event2/buffer.h>          // for evbuffer_new
#include <event2/http.h>            // for evhttp_request_get_output_headers
#include <cstddef>                  // for size_t
#include <string>                   // for string
#include "event/event_smart_ptr.h"  // for EventBufferPtr
#include "non_copyable.h"           // for Noncopyable
struct evhttp_request;

namespace Core::Http {

class HttpResponse :public Core::Noncopyable {

public:
    explicit HttpResponse(struct evhttp_request *request_)
    :
    request(request_),
    httpBuffer(evbuffer_new()),
    output_headers(evhttp_request_get_output_headers(request)) {};

    bool response(short code, const std::string& response);

    bool header(const std::string &key, const std::string &value);

    /**
     * 获取响应体积
     * @return
     */
    size_t getResponseSize() {
        return responseSize;
    }


private:
    /**
     * @brief 请求句柄
     *
     */
    evhttp_request *request;

    /**
     * @brief buffer
     *
     */
    Event::EventBufferPtr httpBuffer;

    size_t responseSize = 0;

    //http响应头
    struct evkeyvalq *output_headers;
};
}

