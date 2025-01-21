#pragma once

#include "component/api.h"
#include "http_config.h"
#include "non_copyable.h"
#include <memory>
#include <string>
namespace Core::Component {
class UnixThreadContainer; // IWYU pragma: keep
} // namespace Core::Component
namespace Core::Event {
class EventLoop; // IWYU pragma: keep
} // namespace Core::Event
namespace Core::Http {
class HttpRouter; // IWYU pragma: keep
class HttpWorker; // IWYU pragma: keep
} // namespace Core::Http
struct evhttp_request; // IWYU pragma: keep

namespace Core::Http {
/**
 * @brief 这个类提供了http的功能，设计主要是为了提供查询接口，所以只会在主线程中使用，不会利用多核心
 *
 */
class HttpManager : public Component::Component, public Core::Noncopyable {
public:
  HttpManager(const std::shared_ptr<Event::EventLoop> &loop_,
              const std::shared_ptr<Core::Component::UnixThreadContainer> &manager,
              const std::shared_ptr<HttpConfig> &config);

  std::string name() override { return HTTP_MANAGER_NAME; }

  void init() override;

  void start() override {};

  void stop() override;

  void finish() override {};

  /**
   * @brief http请求的派遣,这里要注意性能，作为处理的主入口，压测结果单线程4w5
   *
   */
  static void httpRequestHandle(struct evhttp_request *, void *);

  std::shared_ptr<HttpRouter> &getRouter() { return routerHandle; }

private:
  /**
   * @brief 获取http请求类型
   *
   * @param type
   * @return std::string
   */
  std::string getHttpRequestType(short type);

  /**
   * @brief http的ip
   *
   */
  std::string httpIp;

  /**
   * @brief http的端口
   *
   */
  short httpPort;

  /**
   * @brief http的超时时间
   *
   */
  int httpTimeout;
  // http的基础事例
  // 路由
  std::shared_ptr<HttpRouter> routerHandle;

  /**
   * @brief 要绑定的事件循环
   *
   */
  Event::EventLoop *loop;

  /**
   * 线程管理
   */
  std::shared_ptr<Core::Component::UnixThreadContainer> manager_;

  // 配置
  std::shared_ptr<HttpConfig> httpConfig;

  // 没有子线程的时候会使用这个变量存储worker
  std::shared_ptr<HttpWorker> current;
};
} // namespace Core::Http
