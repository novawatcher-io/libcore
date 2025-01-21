#pragma once
#include <cstdint>     // for uint8_t
#include <cstring>     // for size_t
#include <string>      // for string, basic_string
#include <string_view> // for string_view
#include <unistd.h>    // for gethostname

extern std::string pod_name;
extern std::string app_name;
extern std::string instance;
extern std::string eth;

namespace Core::Common {

static inline std::string &getPodName() {
  if (!pod_name.empty()) {
    return pod_name;
  }
  char name[65] = {0};
  int res = gethostname(name, sizeof(name));
  if (res == -1) {
    return pod_name;
  }
  pod_name.assign(name);
  return pod_name;
}

/**
 *
 * @return
 */
std::string &getAppName();

// 设置选定网卡
static inline bool setEth(const std::string &eth_) {
  if (eth_.empty()) {
    return false;
  }
  eth = eth_;
  return true;
}

// 获取本机ip
std::string &getEthIp();

static inline uint8_t getHexByAscii(uint8_t ascii) {
  if (ascii >= 'a') {
    return ascii - 97 + 10;
  }
  return ascii;
}

/**
 * 编码traceId为16位
 * @param traceId
 * @return
 */
inline static bool encodeTraceId(const std::string_view &traceId, std::string &encodeTraceId) {
  if (traceId.length() < 32) {
    return false;
  }

  uint8_t buf[16] = {0};
  for (int i = 0; i < 16; ++i) {
    buf[i] = (((getHexByAscii(traceId[i * 2 + 0]) & 0xF) << 4)) | (((getHexByAscii(traceId[i * 2 + 1]) & 0xF) << 0));
  }

  encodeTraceId.assign(buf, buf + 16);
  return true;
};

/**
 * 编码spanId为16位
 * @param traceId
 * @return
 */
inline static bool encodeSpanId(const std::string_view &spanId, std::string &encodeSpanId) {
  if (spanId.length() < 16) {
    return false;
  }

  uint8_t buf[8] = {0};
  for (int i = 0; i < 8; ++i) {
    buf[i] = (((getHexByAscii(spanId[i * 2 + 0]) & 0xF) << 4)) | (((getHexByAscii(spanId[i * 2 + 1]) & 0xF) << 0));
  }

  encodeSpanId.assign(buf, buf + 8);
  return true;
};

// 去掉所有的反斜杠转义符
std::string stripslashes(std::string_view str);
/**
 * 将内容压缩为gzip
 * @param context
 * @param buffer
 * @return
 */
bool gzipCompress(const unsigned char *context, size_t length, std::string &);

/**
 * 出于性能方面考虑，直接用c实现了一套大小写转换
 * @param str
 * @return
 */
char *cStrTolower(char *str);

/**
 * 出于性能方面考虑，直接用c实现了一套大小写转换
 * @param str
 * @return
 */
char *cStrToUpper(char *str);

/**
 * 检查是否是ipv4
 * @param ip
 * @return
 */
bool checkIpv4(const char *ip);

/**
 * 检查是否是ipv6
 * @param ip
 * @return
 */
bool checkIpv6(const char *ip);

/**
 * 从ip端口里去获取地址信息
 * "localhost:5000"
 * @param address_info
 * @return
 */
std::string getAddressFromIpPortString(const std::string &addressInfo);

/**
 * 获取环境变量名字
 * @return
 */
std::string &getEnv();

/**
 * 设置环境变量名字
 * @param env_
 * @return
 */
bool setEnv(const std::string &env_);

/**
 * 获取主机名字
 * @return
 */
std::string &getHostName();

/**
 * 初始化主机名字
 * @return
 */
bool initHostName();

/**
 *
 * @param str
 * @param h
 * @return
 */
constexpr static inline unsigned int str2int(const char *str, int h = 0) {
  return !str[h] ? 5381 : (str2int(str, h + 1) * 33) ^ str[h];
}

} // namespace Core::Common
