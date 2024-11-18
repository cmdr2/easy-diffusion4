#ifndef __ED_SERVER_H__
#define __ED_SERVER_H__

#include "task_manager.h"

class Server {
private:
    TaskManager& taskManager;

public:
    Server(TaskManager& manager);
    void start();
};

#endif // SERVER_H
