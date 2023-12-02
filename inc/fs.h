#pragma once

#include <stdbool.h>
#include <stddef.h>

int send_text_file(int socket_fd, const char* filename);

int send_base64_file(int socket_fd, const char* filename);

bool file_exists(const char* filename);

size_t text_file_length(const char* filename);

size_t binary_file_length(const char* filename);
