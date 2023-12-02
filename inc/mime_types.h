#pragma once

typedef enum {
    UNKNOWN_MIME,
    TEXT_PLAIN,
    TEXT_HTML,
    TEXT_CSS,
    TEXT_JAVASCRIPT,
    IMAGE_JPEG,
    IMAGE_PNG,
    IMAGE_GIF,
    IMAGE_SVG,
} mime_type_t;

const char* mime_type_str(mime_type_t mime_type);
mime_type_t mime_type_from_path(const char* path);
