#include "image_tasks.h"

void RenderImageTask::run(sd_ctx_t& ctx) {
    responseData = "Rendered Image";
}

void FilterImageTask::run(sd_ctx_t& ctx) {
    responseData = "Filtered Image";
}
