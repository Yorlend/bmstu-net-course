#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "request.h"

static void eat(char** buffer, size_t size)
{
    (*buffer)[size] = '\0';
    *buffer += size + 1;
}

static int eat_sp(char** buffer)
{
    int res = EXIT_FAILURE;
    while (**buffer == ' ' || **buffer == '\t')
    {
        ++*buffer;
        res = EXIT_SUCCESS;
    }
    return res;
}

static int eat_crlf(char** buffer)
{
    int res = EXIT_FAILURE;
    while (**buffer == '\r' || **buffer == '\n')
    {
        ++*buffer;
        res = EXIT_SUCCESS;
    }
    return res;
}

/*

    Request       = Request-Line                 ; Section 5.1
                    *(( general-header           ; Section 4.5
                        | request-header         ; Section 5.3
                        | entity-header ) CRLF)  ; Section 7.1
                    CRLF
                    [ message-body ]             ; Section 4.3

    Request-Line   = Method SP Request-URI SP HTTP-Version CRLF

    Method         = "OPTIONS"                ; Section 9.2
                      | "GET"                    ; Section 9.3
                      | "HEAD"                   ; Section 9.4
                      | "POST"                   ; Section 9.5
                      | "PUT"                    ; Section 9.6
                      | "DELETE"                 ; Section 9.7
                      | "TRACE"                  ; Section 9.8
                      | "CONNECT"                ; Section 9.9
    
    Request-URI    = "*" | absoluteURI | abs_path | authority
*/

static int parse_request_method(request_method_t* method, char** buffer)
{
#define CHECK_METHOD(method_name) \
    if (strncmp(*buffer, #method_name, strlen(#method_name)) == 0) \
    { \
        *method = method_name; \
        *buffer += strlen(#method_name); \
        return EXIT_SUCCESS; \
    }

    CHECK_METHOD(OPTIONS)
    else CHECK_METHOD(GET)
    else CHECK_METHOD(HEAD)
    else CHECK_METHOD(POST)
    else CHECK_METHOD(PUT)
    else CHECK_METHOD(DELETE)
    else CHECK_METHOD(TRACE)
    else CHECK_METHOD(CONNECT)

#undef CHECK_METHOD
    
    return EXIT_FAILURE;
}

static int parse_request_uri(const char** uri, char** buffer)
{
    *uri = *buffer;
    while (!isspace(**buffer))
        ++*buffer;
    eat(buffer, 0);
    // TODO: handle uri if it is at the end
    return EXIT_SUCCESS;
}

static int parse_http_version(http_version_t* version, char** buffer)
{
    if (strncmp(*buffer, "HTTP/1.0", strlen("HTTP/1.0")) == 0)
    {
        *version = HTTP_1_0;
        *buffer += strlen("HTTP/1.0");
        return EXIT_SUCCESS;
    }
    else if (strncmp(*buffer, "HTTP/1.1", strlen("HTTP/1.1")) == 0)
    {
        *version = HTTP_1_1;
        *buffer += strlen("HTTP/1.1");
        return EXIT_SUCCESS;
    }
    else if (strncmp(*buffer, "HTTP/2.0", strlen("HTTP/2.0")) == 0)
    {
        *version = HTTP_2_0;
        *buffer += strlen("HTTP/2.0");
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}

static int parse_headers(struct request_header header[REQUEST_MAX_HEADERS_COUNT], char** buffer)
{
    int i = 0;
    while (**buffer != '\0' && !((*buffer)[0] == '\r' && (*buffer)[1] == '\n' || (*buffer)[0] == '\n'))
    {
        if (i >= REQUEST_MAX_HEADERS_COUNT)
        {
            fprintf(stderr, "too many headers in request!\n");
            return EXIT_FAILURE; // tot many headers in request
        }

        // parse header
        char* pos = strstr(*buffer, ":");
        char* endl = strstr(*buffer, "\r\n");
        if (pos == NULL || endl == NULL)
        {
            fprintf(stderr, "failed to parse header at '%s'\n", *buffer);
            return EXIT_FAILURE;
        }
        *pos = '\0';
        *endl = '\0';
        eat_sp(&pos);
        header[i].key = *buffer;
        header[i].value = pos;
        *buffer = endl + 2;
        ++i;        
    }

    return EXIT_SUCCESS;
}

parse_status_t parse_request(struct request *request, char* buffer)
{
    if (parse_request_method(&request->method, &buffer) != EXIT_SUCCESS)
        return INVALID_METHOD;

    eat_sp(&buffer);

    if (parse_request_uri(&request->uri, &buffer) != EXIT_SUCCESS)
        return INVALID_URI;

    eat_sp(&buffer);

    if (parse_http_version(&request->http_version, &buffer) != EXIT_SUCCESS)
        return INVALID_HTTP_VERSION;
    
    if (eat_crlf(&buffer) != EXIT_SUCCESS)
        return EXPECTED_CRLF;

    if (parse_headers(request->headers, &buffer) != EXIT_SUCCESS)
        return INVALID_HEADERS;
    
    if (request->method != OPTIONS && request->method != HEAD)
    {
        if (eat_crlf(&buffer) != EXIT_SUCCESS)
            return EXPECTED_CRLF;
        request->body = buffer;
    }
        
    return PARSE_SUCCESS;
}
