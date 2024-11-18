#include "image_tasks.h"
#include "util.h"

#include <iostream>

GenerateImageTask::GenerateImageTask(const GenerateImageInputData& req) {
    input_data = std::make_shared<GenerateImageInputData>(req);
}

void GenerateImageTask::run(sd_ctx_t* ctx) {
    auto req = std::static_pointer_cast<GenerateImageInputData>(input_data);

    sd_image_t* image = txt2img(ctx, req->prompt.c_str(), req->negative_prompt.c_str(), req->clip_skip, req->cfg_scale, req->guidance, req->width, req->height,
                                EULER_A, req->sample_steps, req->seed, req->batch_count, NULL, 0.0f, 0.0f, false, "");

    if (image == NULL) {
        std::cerr << "txt2img failed." << std::endl;
        free_sd_ctx(ctx);
        throw std::exception("txt2img failed");
    }

    // Output image details
    std::cout << "Generated image: " << image->width << "x" << image->height << std::endl;

    int width = image[0].width;
    int height = image[0].height;
    int channels = image[0].channel;

    // Convert the image data to a PNG buffer
    output_data = convert_to_png_buffer(image[0].data, width, height, channels, output_data_size);
    output_data_type = "image/png";
}

FilterImageTask::FilterImageTask(const FilterImageInputData& req) {
    input_data = std::make_shared<FilterImageInputData>(req);
}

void FilterImageTask::run(sd_ctx_t* ctx) {
    std::string str = "Filtered image";
    output_data = reinterpret_cast<unsigned char*>("Filtered image");
    output_data_size = str.length();
    output_data_type = "text/plain";
}
