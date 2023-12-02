#pragma once

#include <time.h>

typedef enum log_level {
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_TRACE
} log_level_t;

typedef enum log_redirect {
    LOG_REDIRECT_NONE,
    LOG_REDIRECT_STDOUT = 1,
    LOG_REDIRECT_STDERR = 2,
    LOG_REDIRECT_FILE = 4
} log_redirect_t;

int set_logger_file(const char* filename);
void set_logger_level(log_level_t level);
void set_logger_redirect(log_redirect_t redirect);
void close_logger(void);

void log_message(log_level_t level, const char* fmt, ...);

#define __IS_VA_ARGS_EMTPY(...) (sizeof((char[]) { __VA_ARGS__ }) == 1)

#define LOG(level, prefix, fmt, ...) \
    do { \
        time_t t = time(NULL); \
        char asctime_buf[64]; \
        strftime(asctime_buf, sizeof(asctime_buf), "%Y-%m-%d %H:%M:%S", localtime(&t)); \
        log_message(level, "%s [" prefix "] %20s:%-4d %20s: " fmt "\n", asctime_buf, __FILE__, __LINE__, __func__, ## __VA_ARGS__); \
    } while (0)

#define LOG_ERROR(fmt, ...)   LOG(LOG_LEVEL_ERROR,   "ERROR", fmt, ## __VA_ARGS__)
#define LOG_WARNING(fmt, ...) LOG(LOG_LEVEL_WARNING, "WARNI", fmt, ## __VA_ARGS__)
#define LOG_INFO(fmt, ...)    LOG(LOG_LEVEL_INFO,    "INFO ", fmt, ## __VA_ARGS__)
#define LOG_DEBUG(fmt, ...)   LOG(LOG_LEVEL_DEBUG,   "DEBUG", fmt, ## __VA_ARGS__)
#define LOG_TRACE(fmt, ...)   LOG(LOG_LEVEL_TRACE,   "TRACE", fmt, ## __VA_ARGS__)
