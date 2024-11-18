#include "stable-diffusion.h"
#include <iostream>

#include "logging.h"

int main(int argc, char* argv[]) {
    sd_set_log_callback(sd_log_cb, NULL);

    // Define paths for model files
    const char* model_path = "F:\\models\\stable-diffusion\\sd-v1-5.safetensors";
    // const char* model_path = "F:\\ED4\\server\\miniSD.ckpt";
    const char* clip_l_path = "";
    const char* clip_g_path = "";
    const char* t5xxl_path = "";
    const char* diffusion_model_path = "";
    const char* vae_path = "";
    const char* taesd_path = "";
    const char* control_net_path = "";
    const char* lora_model_dir = "";
    const char* embed_dir = "";
    const char* stacked_id_embed_dir = "";

    // Create the Stable Diffusion context
    sd_ctx_t* ctx = new_sd_ctx(model_path, clip_l_path, clip_g_path, t5xxl_path, diffusion_model_path,
                               vae_path, taesd_path, control_net_path, lora_model_dir,
                               embed_dir, stacked_id_embed_dir, false, false, false,
                               -1, SD_TYPE_F16, STD_DEFAULT_RNG, DEFAULT, false, false, false);

    if (ctx == NULL) {
        std::cerr << "Failed to create Stable Diffusion context." << std::endl;
        return -1;
    }

    std::cout<<"Made the context";

    int width = 64;
    int height = 64;
    int steps = 25;

    // Generate image using txt2img
    const char* prompt = "A beautiful landscape painting";
    const char* negative_prompt = "";
    sd_image_t* image = txt2img(ctx, prompt, negative_prompt, 0, 7.5f, 1.0f, width, height,
                                EULER_A, steps, 42, 1, NULL, 0.0f, 0.0f, false, "");

    sd_image_t* image2 = txt2img(ctx, prompt, negative_prompt, 0, 7.5f, 1.0f, width, height,
                                EULER_A, steps, 42, 1, NULL, 0.0f, 0.0f, false, "");

    if (image == NULL) {
        std::cerr << "txt2img failed." << std::endl;
        free_sd_ctx(ctx);
        return -1;
    }

    // Output image details
    std::cout << "Generated image: " << image->width << "x" << image->height << std::endl;

    return 0;
}