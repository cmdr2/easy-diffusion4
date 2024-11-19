#ifndef __ED__IMAGE_TASKS_H__
#define __ED__IMAGE_TASKS_H__

#include "task_manager.h"
#include "crow.h"

struct GenerateImageInputData : InputData {
    std::string prompt = "";
    std::string negative_prompt = "";
    int seed = 42;
    int sample_steps = 25;
    int width = 512;
    int height = 512;
    float cfg_scale = 7.5f;
    float guidance = 7.5f;
    int batch_count = 1;
    int clip_skip = 0;

    // Deserialize with default values if keys are missing
    static GenerateImageInputData deserialize(const crow::json::rvalue& json) {
        GenerateImageInputData request;
        request.prompt = json.has("prompt") ? std::string(json["prompt"].s()) : "";
        request.negative_prompt = json.has("negative_prompt") ? std::string(json["negative_prompt"].s()) : "";
        request.seed = json.has("seed") ? json["seed"].i() : 42;
        request.sample_steps = json.has("sample_steps") ? json["sample_steps"].i() : 25;
        request.width = json.has("width") ? json["width"].i() : 512;
        request.height = json.has("height") ? json["height"].i() : 512;
        request.cfg_scale = json.has("cfg_scale") ? json["cfg_scale"].d() : 7.5f;
        request.guidance = json.has("guidance") ? json["guidance"].d() : 7.5f;
        request.batch_count = json.has("batch_count") ? json["batch_count"].i() : 1;
        request.clip_skip = json.has("clip_skip") ? json["clip_skip"].i() : 0;
        return request;
    }
};

struct FilterImageInputData : InputData {
    std::string filter_name;

    // Deserialize with default values if keys are missing
    static FilterImageInputData deserialize(const crow::json::rvalue& json) {
        FilterImageInputData request;
        request.filter_name = std::string(json["filter_name"].s());
        return request;
    }
};

class GenerateImageTask : public Task {
public:
    GenerateImageTask(const GenerateImageInputData& req);
    void run(sd_ctx_t* ctx) override;
};

class FilterImageTask : public Task {
public:
    FilterImageTask(const FilterImageInputData& req);
    void run(sd_ctx_t* ctx) override;
};

#endif // IMAGE_TASKS_H
