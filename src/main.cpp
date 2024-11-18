#include "stable-diffusion.h"
#include <iostream>

#include "task_manager.h"
#include "server.h"
#include "logging.h"

int main(int argc, char* argv[]) {
    sd_set_log_callback(sd_log_cb, NULL);

    TaskManager manager({"Worker1", "Worker2", "Worker3"});
    Server server(manager);

    server.start();

    return 0;
}