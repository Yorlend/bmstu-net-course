#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "security.h"

int join_paths_secure(char *destination, size_t buffer_size, const char* path1, const char* path2)
{
    if (buffer_size < strlen(path1) + strlen(path2) + 1) {
        return EXIT_FAILURE;
    }

    strcpy(destination, path1);
    // Check if path1 ends with '/' and path2 starts with '/'. If so, remove one.
    int len1 = strlen(path1);
    int len2 = strlen(path2);
    if (path1[len1 - 1] == '/' && path2[0] == '/') {
        strcat(destination + len1 - 1, path2);
    }
    // If neither path1 ends with '/', nor path2 starts with '/', add a '/' between them.
    else if (path1[len1 - 1] != '/' && path2[0] != '/') {
        sprintf(destination, "%s/%s", path1, path2);
    }
    // If only one of path1 ends with '/' or path2 starts with '/', just concatenate them.
    else {
        strcat(destination, path2);
    }

    // replace all occurrences of '../' with '/'
    const char* search = "../";
    const char* replace = "///";
    char* match = strstr(destination, search);
    while (match != NULL) {
        strncpy(match, replace, strlen(replace));
        match = strstr(destination, search);
    }

    return EXIT_SUCCESS;
}

