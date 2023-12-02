#include <errno.h>
#include <string.h>

#include "logger.h"
#include "mime_types.h"

const char* mime_type_str(mime_type_t mime_type)
{
    switch (mime_type)
    {
        case TEXT_PLAIN:
            return "text/plain";
        case TEXT_HTML:
            return "text/html";
        case TEXT_CSS:
            return "text/css";
        case TEXT_JAVASCRIPT:
            return "text/javascript";
        case IMAGE_JPEG:
            return "image/jpeg";
        case IMAGE_PNG:
            return "image/png";
        case IMAGE_GIF:
            return "image/gif";
        case IMAGE_SVG:
            return "image/svg+xml";
        default:
            LOG_ERROR("unknown mime type: %d", mime_type);
            return "text/plain";
    }
}

mime_type_t mime_type_from_path(const char* path)
{
    path = strrchr(path, '.');
    if (strcmp(path, ".html") == 0)
        return TEXT_HTML;
    else if (strcmp(path, ".css") == 0)
        return TEXT_CSS;
    else if (strcmp(path, ".js") == 0)
        return TEXT_JAVASCRIPT;
    else if (strcmp(path, ".jpg") == 0)
        return IMAGE_JPEG;
    else if (strcmp(path, ".jpeg") == 0)
        return IMAGE_JPEG;
    else if (strcmp(path, ".png") == 0)
        return IMAGE_PNG;
    else if (strcmp(path, ".gif") == 0)
        return IMAGE_GIF;
    else if (strcmp(path, ".svg") == 0)
        return IMAGE_SVG;
    else if (strcmp(path, ".txt") == 0)
        return TEXT_PLAIN;
    else
        return UNKNOWN_MIME;
}
