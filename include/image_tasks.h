#ifndef __ED__IMAGE_TASKS_H__
#define __ED__IMAGE_TASKS_H__

#include "task_manager.h"

class RenderImageTask : public Task {
public:
    void run(sd_ctx_t& ctx) override;
};

class FilterImageTask : public Task {
public:
    void run(sd_ctx_t& ctx) override;
};

#endif // IMAGE_TASKS_H
