#include "server.h"

int main(void)
{
    init_server("127.0.0.1", 8080);
    
    return run_server();
}
