//
// Created by zhanglei on 10/25/21.
//
#include "non_copyable.h"
namespace Core {
namespace Http {

class HttpClientResponse :public Core::Noncopyable {
public:
    HttpClientResponse(struct evhttp_request *req_,
            void *arg_,
            const char *host_,
            const char *path_,
            std::string& method_)
            :host(host_), path(path_), method(method_), req(req_), arg(arg_){
        if (!req) {
            return;
        }

        /**
         * 解析body
         */
        int nread;
        char buffer[BUFSIZ];
        while ((nread = evbuffer_remove(evhttp_request_get_input_buffer(req),
                                        buffer, BUFSIZ))
               > 0) {
            /* These are just arbitrary chunks of 256 bytes.
             * They are not lines, so we can't treat them as such. */
            buffer[nread] = '\0';
            body.append(buffer);
        }

        /**
         * 解析header头
         */
        struct evkeyvalq *headersKv;
        struct evkeyval *header;
        headersKv = evhttp_request_get_input_headers(req);
        for (header = headersKv->tqh_first; header;
             header = header->next.tqe_next) {
            headers[header->key] = header->value;
        }

        code = evhttp_request_get_response_code(req);
    }

    const char *getHost() {
        return host;
    }

    const char *getPath() {
        return path;
    }

    std::string& getMethod() {
        return method;
    }

    std::string& getBody() {
        return body;
    }

    int getCode() {
        return code;
    }

    struct evhttp_request *getRequest() {
        return req;
    }

    void *getArg() {
        return arg;
    }

    size_t bodySize() {
        return body.size();
    }

    std::unordered_map<std::string, std::string>& header() {
        return headers;
    }

private:
    int code = 0;
    std::string body;
    const char *host = nullptr;
    const char *path = nullptr;
    std::string& method;
    struct evhttp_request *req = nullptr;
    void *arg = nullptr;
    std::unordered_map<std::string, std::string> headers;
};
}
}
