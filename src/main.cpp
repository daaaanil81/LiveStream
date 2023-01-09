#include "Logger/reactive_logging_server.h"
#include <ace/Log_Msg.h>
#include <iostream>

int main(int argc, char *argv[]) {

    ACE::debug(true);
    Reactive_Logging_Server server;
    if (server.run(argc, argv) == -1) {
        ACE_ERROR_RETURN((LM_ERROR, "%N:%I %p\n", "server.run()"), 1);
    }
    return 0;
}
