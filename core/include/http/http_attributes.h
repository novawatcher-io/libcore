#pragma once

#include <jemalloc/jemalloc.h>
#include <string>

namespace Core {
namespace Http {

class HttpAttributes {
public:
    std::string prefixStr = "";

    std::string middlewareStr = "";

    std::string namespaceStr = "";
};
}
}