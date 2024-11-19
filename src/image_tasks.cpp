#include "image_tasks.h"
#include "util.h"
#include "stable-diffusion.h"

#include <iostream>

#define CALLBACK_IMAGE_PREVIEW_INTERVAL 3

void generate_image_step_callback(size_t number, size_t step, uint8_t* image_data, void* data);

struct task_run_ctx {
    sd_ctx_t* ctx;
    Task* task;
};

GenerateImageTask::GenerateImageTask(const GenerateImageInputData& req) {
    input_data = std::make_shared<GenerateImageInputData>(req);
}

void GenerateImageTask::run(sd_ctx_t* ctx) {
    auto req = std::static_pointer_cast<GenerateImageInputData>(input_data);
    auto task_ctx = std::make_shared<task_run_ctx>();
    task_ctx->ctx = ctx;
    task_ctx->task = this;

    sd_ctx_set_result_step_callback(ctx, generate_image_step_callback, CALLBACK_IMAGE_PREVIEW_INTERVAL, &task_ctx);

    // populate the output buffer with dummy data
    for (int i = 0; i < req->batch_count; ++i) {
        std::vector<unsigned char> x;
        output_data.push_back(x);
    }

    sd_image_t* images = txt2img(ctx, req->prompt.c_str(), req->negative_prompt.c_str(), req->clip_skip, req->cfg_scale, req->guidance, req->width, req->height,
                                EULER_A, req->sample_steps, req->seed, req->batch_count, NULL, 0.0f, 0.0f, false, "");

    if (images == NULL) {
        std::cerr << "txt2img failed." << std::endl;
        free_sd_ctx(ctx);
        throw std::runtime_error("txt2img failed");
    }

    std::cout << "Generated images: " << req->batch_count << " images of size "
              << images[0].width << "x" << images[0].height << std::endl;

    int width = images[0].width;
    int height = images[0].height;
    int channels = images[0].channel;

    // Clear existing output_data
    output_data.clear();

    // Process each generated image and store it in the output_data vector
    for (int i = 0; i < req->batch_count; ++i) {
        auto png_buffer = convert_to_image_buffer(images[i].data, "png", width, height, channels);
        output_data.push_back(std::move(png_buffer)); // Move to avoid copying
    }

    output_data_type = "image/png";
}

void generate_image_step_callback(size_t number, size_t step, uint8_t* image_data, void* data) {
    auto task_ctx = *static_cast<std::shared_ptr<task_run_ctx>*>(data);
    auto task = task_ctx->task;
    auto ctx = task_ctx->ctx;

    auto req = std::static_pointer_cast<GenerateImageInputData>(task->input_data);

    int total_steps = req->batch_count * req->sample_steps;
    int steps_complete = number * req->sample_steps + step;

    task->progress = (double)steps_complete / total_steps;

    if (image_data != nullptr) {
        int width = req->width;
        int height = req->height;
        int channels = 3;

        auto png_buffer = convert_to_image_buffer(image_data, "jpg", width, height, channels);
        task->output_data[number] = std::move(png_buffer); // Move to avoid copying
    }
}

FilterImageTask::FilterImageTask(const FilterImageInputData& req) {
    input_data = std::make_shared<FilterImageInputData>(req);
}

void FilterImageTask::run(sd_ctx_t* ctx) {
    // std::string str = "Filtered image";
    // output_data = reinterpret_cast<unsigned char*>("Filtered image");
    // output_data_size = str.length();
    // output_data_type = "text/plain";
}
