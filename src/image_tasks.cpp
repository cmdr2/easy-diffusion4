#include "image_tasks.h"

#include <iostream>

void RenderImageTask::run(sd_ctx_t* ctx) {
    int width = 64;
    int height = 64;
    int steps = 25;

    // Generate image using txt2img
    const char* prompt = "A beautiful landscape painting";
    const char* negative_prompt = "";
    sd_image_t* image = txt2img(ctx, prompt, negative_prompt, 0, 7.5f, 1.0f, width, height,
                                EULER_A, steps, 42, 1, NULL, 0.0f, 0.0f, false, "");

    if (image == NULL) {
        std::cerr << "txt2img failed." << std::endl;
        free_sd_ctx(ctx);
        throw std::exception("txt2img failed");
    }

    // Output image details
    std::cout << "Generated image: " << image->width << "x" << image->height << std::endl;

    responseData = "Rendered Image";
}

void FilterImageTask::run(sd_ctx_t* ctx) {
    responseData = "Filtered Image";
}
