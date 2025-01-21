add_library(
    http
    http_client.cc
    http_manager.cc
    http_request.cc
    http_response.cc
    http_router.cc
    http_worker.cc
    ssl/hostcheck.c
    ssl/openssl_hostname_validation.c)
add_library(libcore::http ALIAS http)

target_include_directories(http PUBLIC ${libcore_INCLUDE_DIR})
target_link_libraries(
    http
    PRIVATE spdlog::spdlog
            libcore::common
            libcore::component
            fmt::fmt
            libevent::core
            libevent::extra
            libevent::pthreads)
