//
// Created by co2ma on 2023/3/26.
//

#ifndef CODE_LOG_H
#define CODE_LOG_H

#include <cstdio>
#include <ctime>

#define __INFO  "INFO"
#define __WARN  "WARN"
#define __ERROR "ERROR"
#define __DEBUG "DEBUG"

#define __LOG_(level, fmt, ...)                                         \
    do {                                                                \
        struct timespec __tp{};                                           \
        ::clock_gettime(CLOCK_REALTIME, &__tp);                         \
        struct tm __t{};                                                  \
        ::localtime_r(&__tp.tv_sec, &__t);                              \
        char __time_stamp[64]{};                                        \
        snprintf(__time_stamp, 64, "%4d%02d%02d %02d:%02d:%02d.%06ld",  \
        __t.tm_year + 1900, __t.tm_mon + 1, __t.tm_mday,                \
        __t.tm_hour, __t.tm_min, __t.tm_sec, __tp.tv_nsec / 1000);      \
        printf("[%s][%s] [" __FILE__":%d] (%s) - " fmt"\n",             \
        __time_stamp, level, __LINE__, __func__, ##__VA_ARGS__);        \
    } while(0)

#define LOG_INFO(fmt, ...)  __LOG_(__INFO, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  __LOG_(__WARN, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) __LOG_(__ERROR, fmt, ##__VA_ARGS__)

#ifdef DEBUG
#define LOG_DEBUG(fmt, ...) __LOG_(__DEBUG, fmt, ##__VA_ARGS__)
#else
#define LOG_DEBUG(...) do {} while(0)
#endif

#endif //CODE_LOG_H
