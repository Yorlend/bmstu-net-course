#pragma once

typedef enum {
    HTTP_STATUS_OK                 = 200,
    HTTP_STATUS_NO_CONTENT         = 204,
    HTTP_STATUS_BAD_REQUEST        = 400,
    HTTP_STATUS_FORBIDDEN          = 403,
    HTTP_STATUS_NOT_FOUND          = 404,
    HTTP_STATUS_METHOD_NOT_ALLOWED = 405
} http_status_code_t;

const char* http_status_str(http_status_code_t code);
