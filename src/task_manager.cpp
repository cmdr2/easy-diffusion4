#include "task_manager.h"
#include "util.h"
#include <thread>
#include <iostream>

const std::chrono::seconds FINISHED_TASK_TTL(30 * 60); // Tasks expire after 30 minutes

Task::Task() {
    taskId = generate_uuid();
    enqueued_time = std::chrono::system_clock::now();
}

std::string taskStatusToString(TaskStatus status) {
    switch (status) {
        case TaskStatus::PENDING:
            return "PENDING";
        case TaskStatus::RUNNING:
            return "RUNNING";
        case TaskStatus::ERROR:
            return "ERROR";
        case TaskStatus::FINISHED:
            return "FINISHED";
        default:
            return "UNKNOWN";
    }
}

Worker::Worker(const std::string& name) : name(name) {
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
    ctx = new_sd_ctx(model_path, clip_l_path, clip_g_path, t5xxl_path, diffusion_model_path,
                               vae_path, taesd_path, control_net_path, lora_model_dir,
                               embed_dir, stacked_id_embed_dir, false, false, false,
                               -1, SD_TYPE_F16, STD_DEFAULT_RNG, DEFAULT, false, false, false);

    if (ctx == NULL) {
        std::cerr << "Failed to create Stable Diffusion context." << std::endl;
        throw std::exception("Failed to create Stable Diffusion context.");
    }

    std::cout << "Made the context for worker: " << name << std::endl;
}

void Worker::run(std::queue<std::shared_ptr<Task>>& pendingTasks,
                 std::unordered_map<std::string, std::shared_ptr<Task>>& finishedTasks,
                 std::mutex& queueMutex,
                 std::condition_variable& taskNotifier,
                 std::atomic<bool>& stopFlag) {
    while (!stopFlag) {
        std::shared_ptr<Task> task = nullptr;

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            taskNotifier.wait(lock, [&]() { return !pendingTasks.empty() || stopFlag; });

            if (stopFlag && pendingTasks.empty()) return;

            task = pendingTasks.front();
            pendingTasks.pop();
        }

        try {
            std::cout << "Worker " << name << " is starting task " << task->taskId << std::endl;
            task->status = TaskStatus::RUNNING;
            task->start_time = std::chrono::system_clock::now();
            task->run(ctx);
            task->status = TaskStatus::FINISHED;
        } catch (const std::exception& e) {
            task->status = TaskStatus::ERROR;
            task->responseData = "Error while executing task: " + std::string(e.what());
        }

        task->finished_time = std::chrono::system_clock::now();

        {
            std::lock_guard<std::mutex> lock(queueMutex);
            finishedTasks[task->taskId] = task;
        }
    }
}

TaskManager::TaskManager(const std::vector<std::string>& workerNames) : stopFlag(false) {
    for (const auto& name : workerNames) {
        workers.emplace_back(std::make_unique<Worker>(name));
    }

    for (auto& worker : workers) {
        std::thread([worker = worker.get(), this]() {
            worker->run(pendingTasks, finishedTasks, queueMutex, taskNotifier, stopFlag);
        }).detach();
    }
}

TaskManager::~TaskManager() {
    stopFlag = true;
    taskNotifier.notify_all();
}

void TaskManager::addTask(std::shared_ptr<Task> task) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        pendingTasks.push(task);
    }
    taskNotifier.notify_one();
}

std::shared_ptr<Task> TaskManager::getFinishedTask(const std::string& taskId) {
    cleanupFinishedTasks();
    std::lock_guard<std::mutex> lock(queueMutex);
    auto it = finishedTasks.find(taskId);
    if (it != finishedTasks.end()) {
        return it->second;
    }
    return nullptr;
}

void TaskManager::cleanupFinishedTasks() {
    std::lock_guard<std::mutex> lock(queueMutex);
    auto now = std::chrono::system_clock::now();
    for (auto it = finishedTasks.begin(); it != finishedTasks.end();) {
        if (std::chrono::duration_cast<std::chrono::seconds>(now - it->second->finished_time) > FINISHED_TASK_TTL) {
            it = finishedTasks.erase(it);
        } else {
            ++it;
        }
    }
}
