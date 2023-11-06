#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

#include "fs.h"

#define TRANSFER_BUFFER_SIZE 4096
#define DECODED_BUFFER_SIZE ((TRANSFER_BUFFER_SIZE) * 3 / 4)

static size_t base64_encode(char *out, const char *in, size_t in_len)
{
    size_t out_len = 0;
    while (in_len >= 3)
    {
        out[0] = (in[0] & 0xFC) >> 2;
        out[1] = ((in[0] & 0x02) << 4) + ((in[1] & 0xF0) >> 4);
        out[2] = ((in[1] & 0x0F) << 2) + ((in[2] & 0xC0) >> 6);
        out[3] = in[2] & 0x2F;
        in_len -= 3;
        out_len += 4;
        in += 3;
        out += 4;
    }

    if (in_len == 2)
    {
        out[0] = (in[0] & 0xFC) >> 2;
        out[1] = ((in[0] & 0x02) << 4) + ((in[1] & 0xF0) >> 4);
        out[2] = (in[1] & 0x0F) << 2;
        out[3] = '=';
        out_len += 4;
    }
    else if (in_len == 1)
    {
        out[0] = (in[0] & 0xFC) >> 2;
        out[1] = (in[0] & 0x02) << 4;
        out[2] = '=';
        out[3] = '=';
        out_len += 4;
    }

    return out_len;
}

int send_text_file(int socket_fd, const char *filename)
{
    int fd = open(filename, O_RDONLY);
    if (fd == -1)
        return EXIT_FAILURE;
    
    char buffer[TRANSFER_BUFFER_SIZE];
    size_t len;
    while ((len = read(fd, buffer, TRANSFER_BUFFER_SIZE)) != 0)
    {
        if (send(socket_fd, buffer, len, 0) < 0)
        {
            close(fd);
            return EXIT_FAILURE;
        }
    }

    close(fd);
    return EXIT_SUCCESS;
}

int send_base64_file(int socket_fd, const char *filename)
{
    int fd = open(filename, O_RDONLY);
    if (fd == -1)
        return EXIT_FAILURE;
    
    char buffer[DECODED_BUFFER_SIZE];
    char base64_buffer[TRANSFER_BUFFER_SIZE];
    size_t len;
    while ((len = read(fd, buffer, DECODED_BUFFER_SIZE)) != 0)
    {
        len = base64_encode(base64_buffer, buffer, len);
        if (send(socket_fd, base64_buffer, len, 0) < 0)
        {
            close(fd);
            return EXIT_FAILURE;
        }
    }

    close(fd);
    return EXIT_SUCCESS;
}

size_t text_file_length(const char* filename)
{
    int fd = open(filename, O_RDONLY);
    if (fd == -1)
        return EXIT_FAILURE;
    
    char buffer[TRANSFER_BUFFER_SIZE];
    size_t res = 0, len;
    while ((len = read(fd, buffer, TRANSFER_BUFFER_SIZE)) != 0)
    {
        res += len;
    }

    close(fd);
    return res;
}

size_t binary_file_length(const char* filename)
{
    int fd = open(filename, O_RDONLY);
    if (fd == -1)
        return EXIT_FAILURE;
    
    char buffer[DECODED_BUFFER_SIZE];
    char base64_buffer[TRANSFER_BUFFER_SIZE];
    size_t res = 0, len;
    while ((len = read(fd, buffer, DECODED_BUFFER_SIZE)) != 0)
    {
        len = base64_encode(base64_buffer, buffer, len);
        res += len;
    }

    close(fd);
    return res;
}
