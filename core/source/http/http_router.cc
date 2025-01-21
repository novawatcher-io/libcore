#include "http/http_router.h"
#include <stddef.h>                // for size_t
#include "http/http_attributes.h"  // for HttpAttributes
namespace Core { namespace Http { class HttpAction; } }

namespace Core {
namespace Http {
std::shared_ptr<HttpRouter> HttpRouter::getRequest(const std::string &uri, const std::shared_ptr<HttpAction> &action) {
    return this->addRoute({"GET", "HEAD"}, uri, action);
}


std::shared_ptr<HttpRouter>
HttpRouter::postRequest(const std::string &uri, std::shared_ptr<HttpAction> &action) {
    return this->addRoute({"POST"}, uri, action);
}


std::shared_ptr<HttpRouter>
HttpRouter::putRequest(const std::string &uri, std::shared_ptr<HttpAction> &action) {
    return this->addRoute({"PUT"}, uri, action);
}


std::shared_ptr<HttpRouter>
HttpRouter::patchRequest(const std::string &uri, std::shared_ptr<HttpAction> &action) {
    return this->addRoute({"PATCH"}, uri, action);
}


std::shared_ptr<HttpRouter>
HttpRouter::deleteRequest(const std::string &uri, std::shared_ptr<HttpAction> &action) {
    return this->addRoute({"DELETE"}, uri, action);
}


std::shared_ptr<HttpRouter>
HttpRouter::optionsRequest(const std::string &uri, std::shared_ptr<HttpAction> &action) {
    this->addRoute({"OPTIONS"}, uri, action);
    return shared_from_this();
}


std::shared_ptr<HttpRouter>
HttpRouter::anyRequest(const std::string &uri, std::shared_ptr<HttpAction> &action) {
    return this->addRoute({"GET", "HEAD", "POST", "PUT", "PATCH", "DELETE"}, uri, action);
}

void HttpRouter::group(const std::shared_ptr<HttpAttributes> &attributes,
                           const std::function<void(std::shared_ptr<HttpRouter>)> &callback) {
    std::shared_ptr<HttpAttributes> parentGroupAttributes = groupAttributes;

    groupAttributes->prefixStr = groupAttributes->prefixStr + "/" + attributes->prefixStr;

    groupAttributes->middlewareStr = groupAttributes->middlewareStr + "|" + attributes->middlewareStr;

    groupAttributes->namespaceStr = groupAttributes->namespaceStr + "\\" + attributes->namespaceStr;

    callback(shared_from_this());

    groupAttributes = parentGroupAttributes;
}

std::shared_ptr<HttpRouter>
HttpRouter::addRoute(const std::vector<std::string> &method, const std::string &uri,
                         const std::shared_ptr<HttpAction> &action) {
    std::string urlPath;
    urlPath.assign(uri);
    // stringTool->strTolower(urlPath);

    if (!groupAttributes->prefixStr.empty()) {
        // urlPath += stringTool->trimString(groupAttributes->prefixStr, "/") + "/" + stringTool->trimString(urlPath, "/");
    }

    addStaticRoute(method, uri, action);
    return shared_from_this();
}

void HttpRouter::addStaticRoute(const std::vector<std::string> &methodVector, const std::string &uri,
                    const std::shared_ptr<HttpAction> &action) {
    for (size_t i = 0; i < methodVector.size(); i++) {
        std::string method = methodVector[i];
        if (staticRoutes.find(method) == staticRoutes.end()) {
            staticRoutes[method][uri] = action;
            continue;
        }

        if (staticRoutes[method].find(uri) == staticRoutes[method].end()) {
            staticRoutes[method][uri] = action;
            continue;
        }
    }
}


std::shared_ptr<HttpAction>& HttpRouter::dispatch(const std::string &method, const std::string_view& uri) {
    if (staticRoutes.empty()) {
        return emptyAction;
    }

    if (staticRoutes.find(method) == staticRoutes.end()) {
        return emptyAction;
    }

    if (staticRoutes[method].find(uri.data()) == staticRoutes[method].end()) {
        return emptyAction;
    }
    return staticRoutes[method][uri.data()];
}

}
}
