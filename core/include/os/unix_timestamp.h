#pragma once

#include <sys/time.h>
#include <cstdint>
#include <string>

namespace Core {
namespace OS {

/**
* @brief unix 时间戳转换,出于性能方面考虑不会使用std::chnoro
*
*/
class UnixTimestamp {
public:
    UnixTimestamp() {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        int64_t seconds = tv.tv_sec;
        microSecondsSinceEpochArg = seconds * microSecondsPerSecond + tv.tv_usec;
    }

    /**
     * @brief 获取当前的年月日时分秒
     *
     * @return std::string
     */
    std::string toDate();

    /**
     * @brief 获取当前的年月日时分秒毫秒
     *
     * @return std::string
     */
    std::string toMicroDate();

    /**
     * @brief 为了日志获取年月日时
     *
     * @return std::string
     */
    std::string toHourDateForLog();

    ~UnixTimestamp() {};
private:
    uint32_t microSecondsPerSecond = 1000 * 1000;
    unsigned long microSecondsSinceEpochArg = 0;
};
}
}
