#include "os/unix_timestamp.h"
#include <ctime>
#include <cstdio>

namespace Core {
namespace OS {

std::string UnixTimestamp::toDate() {
    char buf[64] = {0};
    time_t seconds = static_cast<time_t>(microSecondsSinceEpochArg / microSecondsPerSecond);
    struct tm tm_time;
    //gmtime_r(&seconds, &tm_time);
    localtime_r(&seconds, &tm_time);

    snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    return buf;
}

std::string UnixTimestamp::toMicroDate() {
    char buf[64] = {0};
    time_t seconds = static_cast<time_t>(microSecondsSinceEpochArg / microSecondsPerSecond);

    struct tm tm_time;
    //gmtime_r(&seconds, &tm_time);
    localtime_r(&seconds, &tm_time);
    int microseconds = static_cast<int>(microSecondsSinceEpochArg % microSecondsPerSecond);
    snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d.%06d",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
             microseconds);
    return buf;
}

std::string UnixTimestamp::toHourDateForLog() {
    char buf[64] = {0};
    time_t seconds = static_cast<time_t>(microSecondsSinceEpochArg / microSecondsPerSecond);
    struct tm tm_time;
    //gmtime_r(&seconds, &tm_time);
    localtime_r(&seconds, &tm_time);

    snprintf(buf, sizeof(buf), "%4d%02d%02d-%02d",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour);
    return buf;
}
}
}
