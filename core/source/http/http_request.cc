#include "http/http_request.h"
#include <event2/buffer.h>          // for evbuffer_get_length, evbuffer_pullup
#include <event2/http.h>            // for evhttp_find_header, evhttp_parse_...
#include <event2/http_struct.h>     // for evhttp_request
#include <event2/keyvalq_struct.h>  // for evkeyval, evkeyvalq
#include <cstring>                 // for size_t, memcpy
#include <utility>                  // for pair
#include "common/helper.h"          // for cStrTolower
#include "event/event_smart_ptr.h"  // for EventHttpUriPtr


namespace Core::Http {

void HttpRequest::init() {
    if (!request) {
        return;
    }

    const char *eventUri = evhttp_request_get_uri(request);//获取请求uri
    if (!eventUri) {
        return;
    }

    uri_ = eventUri;

    //解码uri
    Event::EventHttpUriPtr decoded( evhttp_uri_parse(eventUri));
    if (!decoded)
    {
        evhttp_send_error(request, HTTP_BADREQUEST, "Bad Request");
        return;
    }

    //获取uri中的path部分
    const char * eventPath = evhttp_uri_get_path(decoded.get());

    if (eventPath)
        path_ = eventPath;

    struct evkeyval *header = nullptr;
    //解析header 头信息
    struct evkeyvalq * headers = evhttp_request_get_input_headers(request);
    for (header = headers->tqh_first; header;
         header = header->next.tqe_next) {

        /**
         * 这里出于性能考虑，我自己用c直接在原有字符串基础上转小写，因为我们并不需要去std::string拷贝出来后再去修改它
         * 因为原有字符串我不认为我们需要去关注他的大小写
         * 并且这里使用std::string_view 而不是std::string 避免了深拷贝
         */

        std::string_view header_key(Common::cStrTolower(header->key));

        std::string_view header_value(Common::cStrTolower(header->value));
        headersMap[header_key] = header_value;
    }

    //获取uri中的参数部分
    const char * query = (char*)evhttp_uri_get_query(decoded.get());
    if(!query)
    {
        return;
    }

    query_ = query;

}

std::string HttpRequest::get(const std::string& key) {
    if (query_.empty()) {
        return "";
    }

    const evkeyvalq *params = {0};
    //查询指定参数的值
    evhttp_parse_query_str(query_.c_str(), (evkeyvalq *) &params);
    return (char*)evhttp_find_header(params, key.c_str());
}

std::string& HttpRequest::path() {
    return path_;
}

std::vector<char>& HttpRequest::raw() {
    if (!raw_.empty()) {
        return raw_;
    }

    size_t post_size = 0;
    post_size = evbuffer_get_length(request->input_buffer);//获取数据长度

    if (post_size > 0)
    {
        raw_.resize(post_size);
        memcpy(raw_.data(), evbuffer_pullup(request->input_buffer,-1), post_size);
    }

    return raw_;
}

unsigned char* HttpRequest::rawCstr() {
    size_t post_size = 0;
    post_size = evbuffer_get_length(request->input_buffer);//获取数据长度

    if (post_size > 0)
    {
        unsigned char* buf = evbuffer_pullup(request->input_buffer,-1);
        buf[post_size] = '\0';
        return buf;
    }

    return nullptr;
}

size_t HttpRequest::rawLength() {
    size_t post_size = 0;
    post_size = evbuffer_get_length(request->input_buffer);//获取数据长度
    return post_size;
}

std::string_view& HttpRequest::getHeader(std::string_view key) {
    auto iter = headersMap.find(key);
    if (iter == headersMap.end()) {
        return emptyString;
    }

    return iter->second;
}
}

