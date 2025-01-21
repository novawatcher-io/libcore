#pragma once

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "http_attributes.h"
#include "non_copyable.h"
namespace Core {
namespace Http {
class HttpAction;
}
}  // namespace Core

namespace Core {
namespace Http {

class HttpRouter :public std::enable_shared_from_this<HttpRouter>, public Core::Noncopyable {
public:

    HttpRouter()
    :groupAttributes(std::make_shared<HttpAttributes>()) ,
    emptyAction(std::shared_ptr<HttpAction>()) {

    }

    /**
     * @brief 添加get路由
     *
     * @param uri
     * @param action
     * @return std::shared_ptr<HttpRouter>
     */
    std::shared_ptr<HttpRouter> getRequest(const std::string &uri, const std::shared_ptr<HttpAction> &action);

    /**
     * @brief 添加post路由
     *
     * @param uri
     * @param action
     * @return std::shared_ptr<HttpRouter>
     */
    std::shared_ptr<HttpRouter>
    postRequest(const std::string &uri, std::shared_ptr<HttpAction> &action);

    /**
     * @brief 添加put路由
     *
     * @param uri
     * @param action
     * @return std::shared_ptr<HttpRouter>
     */
    std::shared_ptr<HttpRouter> putRequest(const std::string &uri, std::shared_ptr<HttpAction> &action);

    /**
     * @brief 添加put路由
     *
     * @param uri
     * @param action
     * @return std::shared_ptr<HttpRouter>
     */
    std::shared_ptr<HttpRouter>
    patchRequest(const std::string &uri, std::shared_ptr<HttpAction> &action);

    /**
     * @brief 添加delete路由
     *
     * @param uri
     * @param action
     * @return std::shared_ptr<HttpRouter>
     */
    std::shared_ptr<HttpRouter>
    deleteRequest(const std::string &uri, std::shared_ptr<HttpAction> &action);

    /**
     * @brief 添加option路由
     *
     * @param uri
     * @param action
     * @return std::shared_ptr<HttpRouter>
     */
    std::shared_ptr<HttpRouter>
    optionsRequest(const std::string &uri, std::shared_ptr<HttpAction> &action);

    /**
     * @brief 添加 any路由
     *
     * @param uri
     * @param action
     * @return std::shared_ptr<HttpRouter>
     */
    std::shared_ptr<HttpRouter> anyRequest(const std::string &uri, std::shared_ptr<HttpAction> &action);

    /**
     * @brief 分组
     *
     * @param attributes
     * @param callback
     */
    void group(const std::shared_ptr<HttpAttributes> &attributes,
                const std::function<void(std::shared_ptr<HttpRouter>)> &callback);

    /**
     * @brief 添加路由
     *
     * @param method
     * @param uri
     * @param action
     * @return std::shared_ptr<HttpRouter>
     */
    std::shared_ptr<HttpRouter>
    addRoute(const std::vector<std::string> &method, const std::string &uri,
                const std::shared_ptr<HttpAction> &action);

    /**
     * @brief 派遣服务
     *
     * @param uri
     * @param method
     * @return std::shared_ptr<HttpAction>&
     */
    std::shared_ptr<HttpAction>& dispatch(const std::string &uri, const std::string_view& method);

    ~HttpRouter() {};

private:

    void addStaticRoute(const std::vector<std::string> &method, const std::string &uri,
                        const std::shared_ptr<HttpAction> &action);


    /**
     * @brief 分组属性
     *
     */
    std::shared_ptr<HttpAttributes> groupAttributes;

    /**
     * @brief 空的action，用来作为空的情况下的返回值避免了拷贝
     *
     */
    std::shared_ptr<HttpAction> emptyAction;

    //静态集合
    std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<HttpAction>>> staticRoutes;
};
}
}
