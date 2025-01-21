#include "http/http_client.h"
#include <event2/buffer.h>                         // for evbuffer_add
#include <event2/bufferevent.h>                    // for bufferevent_socket...
#include <event2/bufferevent_ssl.h>                // for bufferevent_openss...
#include <event2/http.h>                           // for evhttp_add_header
#include <event2/http_struct.h>                    // for evhttp_request
#include <openssl/rand.h>                          // for RAND_poll
#include <openssl/ssl.h>                           // for SSL_CTX_get_cert_s...
#include <openssl/x509.h>                          // for X509_NAME_oneline
#include <spdlog/spdlog.h>                         // for SPDLOG_ERROR, SPDL...
#include <stdio.h>                                 // for printf, NULL, size_t
#include <strings.h>                               // for strcasecmp
#include <stdexcept>                               // for runtime_error
#include <utility>                                 // for move, pair
#include "build_expect.h"                          // for build_unlikely
#include "event/event_loop.h"                      // for EventLoop
#include "http/http_client_response.h"             // for HttpClientResponse
#include "http/ssl/openssl_hostname_validation.h"  // for Error, MatchFound
#include "os/unix_current_thread.h"                // for currentLoop, loopS...
#include "os/unix_util.h"                          // for getNowTime

namespace Core {
namespace Http {

static int ignore_cert = 1;
/**
 * 这是响应相关的metrics
 */

/**
 * 这是请求相关的metrics
 */
void CallbackArgs::loadRequestMetric(size_t length) {
    //上报记录信息
#if USE_DEBUG
    SPDLOG_DEBUG("loadRequestMetric: {}", length);
#else
    (void)length;
#endif
}


static void http_request_done(struct evhttp_request *req, void *arg) {
    auto params = static_cast<CallbackArgs*>(arg);

    if (build_unlikely(!req)) {
        SPDLOG_ERROR("req is null; path:{};host:{}",  params->host);
        /**
         * 这里已经出错了，在error里已经删除掉了，不要再次删除了
         */
         if (build_likely(params)) {
             params->obj->releaseArgs(params->index);
             params->obj->setStatus(HTTP_CLIENT_ERROR);
         }
        //delete params;
//        params->obj->setStatus(HTTP_CLIENT_FINISH);
        return;
    }

    params->obj->setStatus(HTTP_CLIENT_FINISH);

    if (build_unlikely(!params->cb_)) {
        SPDLOG_ERROR("params->cb_ is null");
        params->obj->releaseArgs(params->index);
        return;
    }

#ifdef USE_DEBUG
    if (params->method == "post" && !req) {
        SPDLOG_WARN("post req is null" );
    }
//    std::cout << "nowtime:" << OS::getNowTime() << std::endl;
//    std::cout << "beginTime:" << params->beginTime << std::endl;
//    std::cout << "duration:" << duration << std::endl;
#endif
    HttpClientResponse response(req, params->args, params->host, params->path, params->method);

    params->cb_(response);
    params->obj->releaseArgs(params->index);
    return;
}

static void http_error_cb(enum evhttp_request_error error_code, void *arg) {
    switch (error_code) {
        case EVREQ_HTTP_TIMEOUT: {
            SPDLOG_ERROR("Timeout reached, also @see evhttp_connection_set_timeout()");
        }
            break;
        case EVREQ_HTTP_EOF: {
            SPDLOG_ERROR("EOF reached");
        }
            break;
        case EVREQ_HTTP_INVALID_HEADER: {
            SPDLOG_ERROR("Error while reading header, or invalid header");
        }
            break;
        case EVREQ_HTTP_BUFFER_ERROR: {
            SPDLOG_ERROR("Error encountered while reading or writing");
        }
            break;
        case EVREQ_HTTP_REQUEST_CANCEL: {
            SPDLOG_ERROR("The evhttp_cancel_request() called on this request.");
        }
            break;
        case EVREQ_HTTP_DATA_TOO_LONG: {
            SPDLOG_ERROR("Body is greater then evhttp_connection_set_max_body_size()");
        }
            break;
    }

    auto params = static_cast<CallbackArgs*>(arg);
    HttpClientResponse response(nullptr, params->args, params->host, params->path, params->method);

//    params->loadErrorMetrics(error_code);
    params->obj->releaseArgs(params->index);
    params->obj->setStatus(HTTP_CLIENT_ERROR);
}

HttpClient::HttpClient(const std::string& requestUrl_, void (*cb_)(HttpClientResponse&), int timeout_)
:requestUrl((requestUrl_)), cb(cb_) ,timeout(timeout_) {
    init();
};

/**
 * 这个函数是在http客户端之后去执行的，当执行这个函数后httpclient存在已经析构的风险，所以不要在这里使用httpclient的指针
 * 这个函数并不会去触发http_error_cb
 */
static void connection_close_cb(struct evhttp_connection *, void */*params*/) {
    SPDLOG_WARN("connection_close_cb");
}

/* See http://archives.seul.org/libevent/users/Jan-2013/msg00039.html */
static int cert_verify_callback(X509_STORE_CTX *x509_ctx, void *arg)
{
    char cert_str[256];
    const char *host = (const char *) arg;
    const char *res_str = "X509_verify_cert failed";
    HostnameValidationResult res = Error;

    /* This is the function that OpenSSL would call if we hadn't called
     * SSL_CTX_set_cert_verify_callback().  Therefore, we are "wrapping"
     * the default functionality, rather than replacing it. */
    int ok_so_far = 0;

    X509 *server_cert = NULL;

    if (ignore_cert) {
        return 1;
    }

    ok_so_far = X509_verify_cert(x509_ctx);

    server_cert = X509_STORE_CTX_get_current_cert(x509_ctx);

    if (ok_so_far) {
        res = validate_hostname(host, server_cert);

        switch (res) {
            case MatchFound:
                res_str = "MatchFound";
                break;
            case MatchNotFound:
                res_str = "MatchNotFound";
                break;
            case NoSANPresent:
                res_str = "NoSANPresent";
                break;
            case MalformedCertificate:
                res_str = "MalformedCertificate";
                break;
            case Error:
                res_str = "Error";
                break;
            default:
                res_str = "WTF!";
                break;
        }
    }

    X509_NAME_oneline(X509_get_subject_name (server_cert),
                      cert_str, sizeof (cert_str));

    if (res == MatchFound) {
        printf("https server '%s' has this certificate, "
               "which looks good to me:\n%s\n",
               host, cert_str);
        return 1;
    } else {
        printf("Got '%s' for hostname '%s' and certificate:\n%s\n",
               res_str, host, cert_str);
        return 0;
    }
}

void HttpClient::init() {
#ifdef USE_DEBUG
    SPDLOG_DEBUG("HttpClient::init({})", requestUrl);
#endif

    if (!OS::UnixCurrentThread::currentLoop) {
        SPDLOG_ERROR("OS::UnixCurrentThread::currentLoop is nullptr");
        return;
    }

    if (!build_unlikely(OS::UnixCurrentThread::currentLoop->getEventBase())) {
        SPDLOG_ERROR("OS::UnixCurrentThread::currentLoop->getEventBase() is nullptr");
        return;
    }

    http_uri .Reset( evhttp_uri_parse(requestUrl.c_str()));

    scheme = evhttp_uri_get_scheme(http_uri.get());
    if (scheme == nullptr || (strcasecmp(scheme, "https") != 0 &&
                           strcasecmp(scheme, "http") != 0)) {
        SPDLOG_ERROR("scheme error");
        return;
    }

    uri = evhttp_uri_get_path(http_uri.get());

    if (http_uri.get() == nullptr) {
        SPDLOG_ERROR("malformed url");
        return;
    }

    host = evhttp_uri_get_host(http_uri.get());
    if (host == nullptr) {
        SPDLOG_ERROR("url must have a host");
        return;
    }

    port = evhttp_uri_get_port(http_uri.get());
    if (build_unlikely(port == -1)) {
        port = (strcasecmp(scheme, "http") == 0) ? 80 : 443;
    }

    if (port != 80 && port != 443) {
        std::string tmp_host(host);
        http_host = tmp_host + ":" + std::to_string(port);
    } else {
        http_host.assign(host);
    }

    query = evhttp_uri_get_query(http_uri.get());
    if (query) {
        uri += "?";
        uri +=  (query);
    }

    if (strcasecmp(scheme, "https") == 0) {
        int r = RAND_poll();
        if (r == 0) {
            throw std::runtime_error("RAND_poll:" + requestUrl);
        }

        ssl_ctx .Reset( SSL_CTX_new(SSLv23_client_method()));
        if (!ssl_ctx.get()) {
            throw std::runtime_error(requestUrl + " SSL_CTX_new failed");
            return;
        }

        SSL_CTX_load_verify_locations (ssl_ctx.get(), host, NULL);

        /* Attempt to use the system's trusted root certificates. */
        store = SSL_CTX_get_cert_store(ssl_ctx.get());
        if (!store) {
            throw std::runtime_error("store is nullptr:" + requestUrl);
        }

        if (X509_STORE_set_default_paths(store) != 1) {
            throw std::runtime_error("X509_STORE_set_default_paths");
        }



        /* Ask OpenSSL to verify the server certificate.  Note that this
         * does NOT include verifying that the hostname is correct.
         * So, by itself, this means anyone with any legitimate
         * CA-issued certificate for any website, can impersonate any
         * other website in the world.  This is not good.  See "The
         * Most Dangerous Code in the World" article at
         * https://crypto.stanford.edu/~dabo/pubs/abstracts/ssl-client-bugs.html
         */
        SSL_CTX_set_verify(ssl_ctx.get(), SSL_VERIFY_PEER, nullptr);
        /* This is how we solve the problem mentioned in the previous
         * comment.  We "wrap" OpenSSL's validation routine in our
         * own routine, which also validates the hostname by calling
         * the code provided by iSECPartners.  Note that even though
         * the "Everything You've Always Wanted to Know About
         * Certificate Validation With OpenSSL (But Were Afraid to
         * Ask)" paper from iSECPartners says very explicitly not to
         * call SSL_CTX_set_cert_verify_callback (at the bottom of
         * page 2), what we're doing here is safe because our
         * cert_verify_callback() calls X509_verify_cert(), which is
         * OpenSSL's built-in routine which would have been called if
         * we hadn't set the callback.  Therefore, we're just
         * "wrapping" OpenSSL's routine, not replacing it. */
        SSL_CTX_set_cert_verify_callback(ssl_ctx.get(), cert_verify_callback,
                                         (void *) host);

        // Create OpenSSL bufferevent and stack evhttp on top of it
        ssl.Reset( SSL_new(ssl_ctx.get()));
        if (build_unlikely(!ssl)) {
            throw std::runtime_error("SSL_new(" + requestUrl + ") error");
        }

        bev = bufferevent_openssl_socket_new(OS::UnixCurrentThread::loopSmartPtr()->getEventBase(), -1, ssl.get(),
                                             BUFFEREVENT_SSL_CONNECTING,
                                             BEV_OPT_DEFER_CALLBACKS);

        if (build_unlikely(!bev)) {
            throw std::runtime_error("bev get nullptr(" +requestUrl+ ")");
        }

        bufferevent_openssl_set_allow_dirty_shutdown(bev, 1);

    } else {
        bev = bufferevent_socket_new(OS::UnixCurrentThread::loopSmartPtr()->getEventBase(), -1, 0);

        if (build_unlikely(!bev)) {
            throw std::runtime_error("bev get nullptr(" +requestUrl+ ")");
        }
    }




    conn .Reset(evhttp_connection_base_bufferevent_new(OS::UnixCurrentThread::loopSmartPtr()->getEventBase(), nullptr,
          bev, host, port));

    if (build_unlikely(!conn.get())) {
        SPDLOG_ERROR("conn.get() is null");
        return;
    }

    evhttp_connection_set_max_body_size(conn.get(), 16 * 1024 * 1024);
    if (timeout > 0) {
        evhttp_connection_set_timeout(conn.get(), timeout);
    }
    closeArgs.beginTime = OS::getNowTime();
    closeArgs.host = host;
    closeArgs.path = evhttp_uri_get_path(http_uri.get());
    evhttp_connection_set_closecb(conn.get(), connection_close_cb, nullptr);
//    evhttp_connection_set_base(conn.get(), OS::UnixCurrentThread::currentLoop->getEventBase());

}

bool HttpClient::get(void* arg) {
    if (!build_unlikely(OS::UnixCurrentThread::currentLoop)) {
        SPDLOG_ERROR("OS::UnixCurrentThread::currentLoop is nullptr");
        return false;
    }

    if (!build_unlikely(OS::UnixCurrentThread::currentLoop->getEventBase())) {
        SPDLOG_ERROR("OS::UnixCurrentThread::currentLoop->getEventBase() is nullptr");
        return false;
    }

    if (build_unlikely(!conn.get())) {
        SPDLOG_ERROR("conn is nullptr");
        return false;
    }

    //这个地方直接使用new 注意内存释放
    auto args = std::make_unique<CallbackArgs>();
    args->cb_ = cb;
    args->beginTime = OS::getNowTime();
    args->host = host;
    args->path = evhttp_uri_get_path(http_uri.get());
    args->args = arg;
    args->obj = this;


    struct evhttp_request *req = evhttp_request_new(http_request_done, args.get());
    if (!req) {
        SPDLOG_ERROR("request failed");
        return false;
    }

    struct evkeyvalq *head = evhttp_request_get_output_headers(req);
    for (auto iter = headers.begin(); iter != headers.end(); iter++) {
        evhttp_add_header(req->output_headers, iter->first.c_str(), iter->second.c_str());
    }
    evhttp_add_header(head,"Host",http_host.c_str());
    evhttp_request_set_error_cb(req, http_error_cb);
    int ret = evhttp_make_request(conn.get(), req, EVHTTP_REQ_GET, uri.c_str());
    if (ret == -1) {
        SPDLOG_ERROR("evhttp_make_request failed");
        return false;
    }
    requestIndex++;
    args->index = requestIndex;
    argsPool[requestIndex] = std::move(args);
    setStatus(HTTP_CLIENT_REQUESTING);

    return true;
}

bool HttpClient::post(unsigned char* data, size_t length, void* arg) {
    if (!build_unlikely(OS::UnixCurrentThread::currentLoop)) {
        SPDLOG_ERROR("OS::UnixCurrentThread::currentLoop is nullptr");
        return false;
    }

    if (!build_unlikely(OS::UnixCurrentThread::currentLoop->getEventBase())) {
        SPDLOG_ERROR("OS::UnixCurrentThread::currentLoop->getEventBase() is nullptr");
        return false;
    }

    if (build_unlikely(!conn.get())) {
        SPDLOG_ERROR("conn is nullptr");
        return false;
    }
    auto args = std::make_unique<CallbackArgs>();
    args->cb_ = cb;
    args->beginTime = OS::getNowTime();
    args->host = host;
    args->path = evhttp_uri_get_path(http_uri.get());
    args->args = arg;
    args->method = "post";
    args->obj = this;
    args->loadRequestMetric(length);
    struct evhttp_request *req = evhttp_request_new(http_request_done, args.get());
    if (!req) {
        SPDLOG_ERROR("request failed");
        return false;
    }

    struct evbuffer *output_buffer = evhttp_request_get_output_buffer(req);

    struct evkeyvalq *head = evhttp_request_get_output_headers(req);
    evhttp_add_header(head,"Host",http_host.c_str());
    for (auto iter = headers.begin(); iter != headers.end(); iter++) {
        evhttp_add_header(req->output_headers, iter->first.c_str(), iter->second.c_str());
    }

    /** Set the post data */
    evbuffer_add(output_buffer, data, length);

    evhttp_request_set_error_cb(req, http_error_cb);

    int ret = evhttp_make_request(conn.get(), req, EVHTTP_REQ_POST, uri.c_str());
    if (ret == -1) {
        SPDLOG_ERROR("evhttp_make_request failed");
        return false;
    }
    requestIndex++;
    args->index = requestIndex;
    argsPool[requestIndex] = std::move(args);
    setStatus(HTTP_CLIENT_REQUESTING);

    return true;
}

void HttpClient::releaseArgs(uint32_t index) {
    auto iter = argsPool.find(index);
    if (iter == argsPool.end()) {
        return;
    }
    argsPool.erase(iter);
}

HttpClient::~HttpClient() {
#ifdef USE_DEBUG
    SPDLOG_DEBUG("~HttpClient:{}", int(status));
    SPDLOG_DEBUG("~HttpClient ptr:{}", static_cast<void*>(this));
#endif
}
}
}
