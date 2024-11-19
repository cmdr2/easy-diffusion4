#include "image_tasks.h"
#include "util.h"

#include <iostream>

GenerateImageTask::GenerateImageTask(const GenerateImageInputData& req) {
    input_data = std::make_shared<GenerateImageInputData>(req);
}

void GenerateImageTask::run(sd_ctx_t* ctx) {
    auto req = std::static_pointer_cast<GenerateImageInputData>(input_data);

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
        auto png_buffer = convert_to_png_buffer(images[i].data, width, height, channels);
        output_data.push_back(std::move(png_buffer)); // Move to avoid copying
    }

    output_data_type = "image/png";
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
