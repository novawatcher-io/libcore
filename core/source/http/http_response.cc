#include "http/http_response.h"
#include <event2/http_struct.h>


namespace Core::Http {

bool HttpResponse::response(short code, const std::string& response) {
    evbuffer_add_printf(httpBuffer.get(), "%s", response.c_str());
    responseSize = response.size();
    evhttp_send_reply(request, code, "", httpBuffer.get());
    return true;
};

bool HttpResponse::header(const std::string &key, const std::string &value) {
    evhttp_add_header(request->output_headers, key.c_str(), value.c_str());
    return true;
};
}

