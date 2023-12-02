#include <errno.h>
#include <string.h>

#include "logger.h"
#include "http_status_codes.h"

const char* http_status_str(http_status_code_t code)
{
    switch (code)
    {
        case HTTP_STATUS_OK:
            return "OK";
        case HTTP_STATUS_NO_CONTENT:
            return "No Content";
        case HTTP_STATUS_BAD_REQUEST:
            return "Bad Request";
        case HTTP_STATUS_FORBIDDEN:
            return "Forbidden";
        case HTTP_STATUS_NOT_FOUND:
            return "Not Found";
        case HTTP_STATUS_METHOD_NOT_ALLOWED:
            return "Method Not Allowed";
        default:
            LOG_ERROR("unknown http status code: %d", code);
            return "Unknown";
    }
}
