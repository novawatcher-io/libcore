#pragma once

#include <cstddef>        // for size_t
#include <map>             // for map
#include <string>          // for basic_string, string
#include <string_view>     // for string_view, hash
#include <unordered_map>   // for unordered_map
#include <vector>          // for vector
#include "non_copyable.h"  // for Noncopyable
struct evhttp_request;  // IWYU pragma: keep

using param = std::map<std::string, std::string>;


namespace Core::Http {

class HttpRequest :public Core::Noncopyable {

public:
    HttpRequest(struct evhttp_request *request_)
            :request(request_) {
        init();
    };

    /**
     * 获取uri
     * @return
     */
    std::string& uri() {
        return uri_ ;
    }

    std::string_view& getHeader(std::string_view key);

    /**
     * 获取get value
     * @return
     */
    std::string get(const std::string& key);

    /**
     * 获取path
     * @return
     */
    std::string& path();

    /**
     * 获取获取原始报文
     * @return
     */
    std::vector<char>& raw();

    unsigned char* rawCstr();

    /**
     * 暂时没有研发因为用不到
     * @return
     */
    std::string post() {
        return "";
    }

    size_t rawLength();

private:
    void init();
    /**
     * @brief 请求句柄
     *
     */
    struct evhttp_request *request;

    std::string path_ = "/";

    std::string query_;

    std::string uri_;

    std::vector<char> raw_;

    std::unordered_map<std::string_view, std::string_view> headersMap;

    std::string_view emptyString;
};
}
