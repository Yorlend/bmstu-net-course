#pragma once

#include "request.h"

void respond_error(int client_socket, int status, const char* message);

int handle_request(int client_socket, struct request request);
