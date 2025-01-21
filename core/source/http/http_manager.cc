#include "http/http_manager.h"
#include "build_expect.h"
#include "component/thread_container.h"
#include "http/http_action.h"
#include "http/http_config.h"
#include "http/http_request.h"
#include "http/http_response.h"
#include "http/http_router.h"
#include "http/http_worker.h"
#include "os/unix_thread.h"
#include <cstddef>
#include <event2/http.h>
#include <event2/http_struct.h>
#include <event2/thread.h>
#include <functional>

namespace Core::Event {
class EventLoop;
}

namespace Core::Http {

/**
 * 记录普罗米修斯的值
 */
HttpManager::HttpManager(const std::shared_ptr<Event::EventLoop> &loop_,
                         const std::shared_ptr<Core::Component::UnixThreadContainer> &manager,
                         const std::shared_ptr<HttpConfig> &config)
    : routerHandle(std::make_shared<HttpRouter>()), loop(loop_.get()), manager_(manager), httpConfig(config) {}

void HttpManager::init() {
  if (!httpConfig) {
    return;
  }
  /**
   * @brief 初始化配置
   *
   */
  httpIp = httpConfig->getIp();
  httpPort = static_cast<short>(httpConfig->getPort());
  httpTimeout = httpConfig->getTimeout();

  evthread_use_pthreads();

  if (!manager_ || manager_->getContainer().empty()) {
    current = std::make_shared<HttpWorker>(httpIp, httpPort);
    current->setTimeout(httpTimeout);
    current->setCallable(httpRequestHandle, this);
    current->init(loop);
    return;
  }
  for (size_t i = 0; i < manager_->getContainer().size(); i++) {
    std::shared_ptr<HttpWorker> worker = std::make_shared<HttpWorker>(httpIp, httpPort);
    worker->setTimeout(httpTimeout);
    worker->setCallable(httpRequestHandle, this);
    manager_->getThread(i)->addInitCallable([worker] { worker->init(nullptr); });
  }
}

std::string HttpManager::getHttpRequestType(short type) {
  switch (type) {
  case evhttp_cmd_type::EVHTTP_REQ_GET:
    return "GET";
  case evhttp_cmd_type::EVHTTP_REQ_POST:
    return "POST";
  case evhttp_cmd_type::EVHTTP_REQ_HEAD:
    return "HEAD";
  case evhttp_cmd_type::EVHTTP_REQ_PUT:
    return "PUT";
  case evhttp_cmd_type::EVHTTP_REQ_DELETE:
    return "DELETE";
  case evhttp_cmd_type::EVHTTP_REQ_OPTIONS:
    return "OPTIONS";
  case evhttp_cmd_type::EVHTTP_REQ_TRACE:
    return "TRACE";
  case evhttp_cmd_type::EVHTTP_REQ_CONNECT:
    return "CONNECT";
  case evhttp_cmd_type::EVHTTP_REQ_PATCH:
    return "PATCH";
  }
  return "GET";
}

/**
 * @brief 这个部分是可能会调用频率比较高的函数接口，不支持多线程，因为http只是提供简单的查询
 * 压测结果为单线程单机4w5
 *
 * @param request
 * @param args
 */
void HttpManager::httpRequestHandle(struct evhttp_request *request, void *args) {
  auto httpDispatcher = static_cast<HttpManager *>(args);
  // 请求
  HttpRequest req(request);

  // 初始化响应体
  HttpResponse response(request);

  // 初始化路由
  std::shared_ptr<HttpRouter> &router = httpDispatcher->getRouter();

  auto &path = req.path();

  // 查找路由
  std::shared_ptr<HttpAction> &action =
      router->dispatch(httpDispatcher->getHttpRequestType(request->type), path.c_str());

  if (build_unlikely(!action)) {
    //        THREAD_WARNING_LOG_TRACE("action not found");
    response.response(HTTP_NOTFOUND, "Not found");
    return;
  }

  // 获取要处理的函数
  if (build_unlikely(!action->doUsers(std::ref(req), std::ref(response)))) {
    //        THREAD_WARNING_LOG_TRACE("action->doUsers failed");
    response.response(HTTP_BADREQUEST, "Bad Request");
    return;
  }
}

void HttpManager::stop() {
  routerHandle = std::make_shared<HttpRouter>();
  current->stop();
};
} // namespace Core::Http
