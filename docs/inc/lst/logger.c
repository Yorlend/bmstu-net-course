#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "logger.h"

static log_level_t logger_level = LOG_LEVEL_INFO;
static log_redirect_t logger_redirect = LOG_REDIRECT_STDERR;
static FILE* logger_file = NULL;

int set_logger_file(const char* filename)
{
    if (logger_file)
        fclose(logger_file);
    logger_file = fopen(filename, "a");
    if (!logger_file)
    {
        perror("open");
        return EXIT_FAILURE;
    }
    setbuf(logger_file, NULL);
    return EXIT_SUCCESS;
}

void set_logger_level(log_level_t level)
{
    logger_level = level;
}

void set_logger_redirect(log_redirect_t redirect)
{
    logger_redirect = redirect;
}

void close_logger(void)
{
    if (logger_file)
    {
        fclose(logger_file);
        logger_file = NULL;
    }
}

void log_message(log_level_t level, const char* fmt, ...)
{
    if (level > logger_level)
        return;

    if (logger_redirect & LOG_REDIRECT_STDOUT)
    {
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
    }
    
    if (logger_redirect & LOG_REDIRECT_STDERR)
    {
        va_list args;
        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        va_end(args);
    }
    
    if (logger_file && (logger_redirect & LOG_REDIRECT_FILE))
    {
        va_list args;
        va_start(args, fmt);
        vfprintf(logger_file, fmt, args);
        va_end(args);
    }
}
