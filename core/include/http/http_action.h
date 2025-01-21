#pragma once
extern "C" {
#include "evhttp.h"
}

#include <memory>
#include <functional>
#include <string>
#include <spdlog/spdlog.h>

#include "build_expect.h"


namespace Core {
namespace Http {

class HttpResponse;
class HttpRequest;

typedef std::function<void(HttpRequest &request,
        HttpResponse &response)> usesClosure;

/**
 * 注册到http路由中的动作实体
 */
class HttpAction {
public:

    std::string middleware;

    void setUsers(const usesClosure &callable) {
        uses = callable;
    }

    usesClosure& getUsers() {
        return uses;
    }

    bool doUsers(HttpRequest &request, HttpResponse &response) {
        if (build_unlikely(!uses)) {
            SPDLOG_ERROR("doUsers nullptr");
            return false;
        }

        uses(std::ref(request), std::ref(response));
        return true;
    }

private:
    usesClosure uses;
};
}
}
