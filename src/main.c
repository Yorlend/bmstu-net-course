#include "server.h"
#include "logger.h"

int main(void)
{
    //set_logger_level(LOG_LEVEL_DEBUG);
    set_logger_file("../server.log");
    set_logger_redirect(LOG_REDIRECT_STDERR | LOG_REDIRECT_FILE);

    init_server("127.0.0.1", 8080);
    
    return run_server();
}
