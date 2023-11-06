#pragma once

#include <stddef.h>

/**
 * Prepends current working directory to path.
 * Double dots are removed from result path
 * 
 * Returns 0 on success, -1 on buffer overflow
 */
int join_paths_secure(char *abs_path_buffer, size_t buffer_size, const char* cwd, const char* rel_path);
