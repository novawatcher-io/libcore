#include "common/helper.h"
#include "build_expect.h"
#include <arpa/inet.h>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <net/if.h>
#include <netinet/in.h>
#include <string>
#include <string_view>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <zconf.h>
#include <zlib.h>

std::string pod_name;
std::string app_name;
std::string instance;
std::string eth;
std::string env;
std::string host_name;

namespace Core {
namespace Common {
std::string &getAppName() {
  if (!app_name.empty()) {
    return app_name;
  }

  std::string &pod_name = getPodName();
  if (pod_name.empty()) {
    app_name = "unknown";
    return app_name;
  }
  /**
   * 由于只查找两次所以不写递归了,
   */
  // 查找第一次
  size_t pos = pod_name.length();
  int count = 0;
  while (auto position = pod_name.rfind('-', pos)) {
    if (position == std::string::npos) {
      app_name = "unknown";
      return app_name;
    }
    if (count == 1) {
      app_name.assign(pod_name.c_str(), position);
      return app_name;
    }
    count++;
    pos = position - 1;
  };

  app_name = "unknown";
  return app_name;
}

std::string &getEthIp() {
  if (!instance.empty()) {
    return instance;
  }
  int sd;
  struct sockaddr_in sin;
  struct ifreq ifr;

  sd = socket(AF_INET, SOCK_DGRAM, 0);
  if (-1 == sd) {
    return instance;
  }

  strncpy(ifr.ifr_name, eth.c_str(), IFNAMSIZ);
  ifr.ifr_name[IFNAMSIZ - 1] = 0;

  // if error: No such device
  if (ioctl(sd, SIOCGIFADDR, &ifr) < 0) {
    printf("ioctl error: %s\n", strerror(errno));
    close(sd);
    return instance;
  }

  memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
  instance = inet_ntoa(sin.sin_addr);
  close(sd);
  return instance;
}

bool gzipCompress(const unsigned char *context, size_t length, std::string &compressBuffer) {
  z_stream dest_stream{};
  if (build_unlikely(deflateInit2(&dest_stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, MAX_WBITS + 16, 9,
                                  Z_DEFAULT_STRATEGY) != Z_OK)) {
    return false;
  }
  uLong len = compressBound(length);
  if (len <= 0) {
    return false;
  }
  auto *buf = (unsigned char *)malloc(len);
  if (!buf) {
    return false;
  }

  dest_stream.next_in = (unsigned char *)context;
  dest_stream.avail_in = length;
  dest_stream.next_out = buf;
  dest_stream.avail_out = len;
  deflate(&dest_stream, Z_FINISH);
  deflateEnd(&dest_stream);
  compressBuffer.assign((char *)buf, dest_stream.total_out);
  free(buf);
  return true;
}

std::string stripslashes(std::string_view str) {
  std::string result;
  if (str.empty()) {
    return result.assign(str);
  }

  for (size_t i = 0; i < str.length(); i++) {
    if (str[i] == '\\' && (i + 1 < str.length())) {
      char byte = str[i + 1];
      switch (byte) {
      case 'n':
        result += '\n';
        break;
      case 'r':
        result += '\r';
        break;
      case 'a':
        result += '\a';
        break;
      case 't':
        result += '\t';
        break;
      case 'v':
        result += '\v';
        break;
      case 'b':
        result += '\b';
        break;
      case 'f':
        result += '\f';
        break;
      case '\\':
        result += '\\';
        break;
        //                case 'x':
        //                    result += byte;
        //                    break;
      default:
        result += byte;
        break;
      }
      i++;
      continue;
    } else {
      result += str[i];
    }
  }
  return result;
}

char *cStrTolower(char *str) {
  if (!str) {
    return nullptr;
  }

  char *new_str = str;
  while (*str) {
    *str = static_cast<char>(tolower(*str));
    str++;
  }
  return new_str;
}

char *cStrToUpper(char *str) {
  if (!str) {
    return nullptr;
  }

  char *new_str = str;
  while (*str) {
    *str = static_cast<char>(toupper(*str));
    str++;
  }
  return new_str;
}

bool checkIpv4(const char *ip) {
  unsigned char buf[sizeof(struct in6_addr)];
  int ret = inet_pton(AF_INET, ip, buf);
  if (ret > 0) {
    return true;
  }

  return false;
}

bool checkIpv6(const char *ip) {
  unsigned char buf[sizeof(struct in6_addr)];
  int ret = inet_pton(AF_INET6, ip, buf);
  if (ret > 0) {
    return true;
  }

  return false;
}

std::string getAddressFromIpPortString(const std::string &addressInfo) {
  auto position = addressInfo.find(':');
  if (position == std::string::npos) {
    return addressInfo;
  }

  return addressInfo.substr(0, position);
}

std::string &getEnv() { return env; }

bool setEnv(const std::string &env_) {
  if (!env.empty()) {
    return false;
  }

  if (env_.empty()) {
    env = "default";
    return true;
  }

  env = env_;
  return true;
}

/**
 * 获取主机名字
 * @return
 */
std::string &getHostName() { return host_name; }

/**
 * 初始化主机名字
 * @return
 */
bool initHostName() {
  if (!host_name.empty()) {
    return false;
  }

  char name[65] = {0};
  int res = gethostname(name, sizeof(name));
  if (res == -1) {
    host_name = "unknown";
    return true;
  }
  host_name = name;
  return true;
}
} // namespace Common
} // namespace Core
