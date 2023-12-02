#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "logger.h"
#include "request.h"

const char* http_version_str(http_version_t version)
{
    switch (version) {
        case HTTP_1_0:
            return "HTTP/1.0";
        case HTTP_1_1:
            return "HTTP/1.1";
        case HTTP_2_0:
            return "HTTP/2.0";
        default:
            LOG_ERROR("invalid http version: %d", version);
            return "(unknown)";
    }
}

const char* request_method_str(request_method_t method)
{
    switch (method) {
        case OPTIONS:
            return "OPTIONS";
        case GET:
            return "GET";
        case HEAD:
            return "HEAD";
        case POST:
            return "POST";
        case PUT:
            return "PUT";
        case DELETE:
            return "DELETE";
        case TRACE:
            return "TRACE";
        case CONNECT:
            return "CONNECT";
        default:
            LOG_ERROR("invalid request method: %d", method);
            return "(unknown)";
    }
}

/**
 * Cuts of the first 'size' characters from the buffer
 * 
 * Inserts '\0' after 'size' characters. Increases
 * buffer pointer so that it points after eaten string
 */
static inline void eat(char** buffer, size_t size)
{
    (*buffer)[size] = '\0';
    *buffer += size + 1;
}

/**
 * Skips space separators (spaces and tabs)
 * 
 * Buffer then points to the first non-space character
 * 
 * Returns EXIT_SUCCESS if space was eaten
 */
static inline int eat_sp(char** buffer)
{
    int res = EXIT_FAILURE;
    while (**buffer == ' ' || **buffer == '\t')
    {
        ++*buffer;
        res = EXIT_SUCCESS;
    }
    return res;
}

/**
 * Skips CRLF character sequence in any order
 * 
 * Buffer then points to the first non-CRLF character
 * 
 * Returns EXIT_SUCCESS if CRLF was eaten
 */
static inline int eat_crlf(char** buffer)
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
    Request structure from RFC

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

/**
 * This function parses the method from the request line in an HTTP request.
 * The method is a token that represents the request method to be performed on the resource identified by the Request-URI.
 * 
 * The function takes in two arguments:
 * - method: a pointer to the request_method_t enum where the parsed method will be stored.
 * - buffer: a pointer to the character buffer that contains the request.
 * 
 * The function iterates over the buffer, comparing the current sequence of characters to the defined request methods (OPTIONS, GET, HEAD, etc).
 * If it finds a match, it sets the method argument to the corresponding request_method_t value and increments the buffer pointer by the length of the matched method.
 * 
 * The function returns EXIT_SUCCESS if a method is successfully parsed and EXIT_FAILURE otherwise.
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

/**
 * This function parses the URI from the request line in an HTTP request.
 * The URI is a string of characters that identifies a name or a resource on the Internet.
 * 
 * The function takes in two arguments:
 * - uri: a pointer to the const char* where the parsed URI will be stored.
 * - buffer: a pointer to the character buffer that contains the request.
 * 
 * The function assigns the start of the URI to the incoming pointer, then
 * iterates over the buffer until it finds a whitespace character, incrementing the buffer pointer along the way.
 * It then calls the `eat` function to consume the whitespace and any trailing null characters.
 * 
 * The function returns EXIT_SUCCESS if the URI is successfully parsed.
 * TODO: handle the case where the URI is at the end of the buffer.
 */
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
