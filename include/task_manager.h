#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include "stable-diffusion.h"

extern const std::chrono::seconds FINISHED_TASK_TTL;

enum class TaskStatus { PENDING, RUNNING, ERROR, FINISHED };

struct Task {
    std::string taskId;
    TaskStatus status = TaskStatus::PENDING;
    std::shared_ptr<void> input_data;
    unsigned char* output_data;
    size_t output_data_size;
    std::string output_data_type;

    std::chrono::system_clock::time_point enqueued_time;
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point finished_time;

    Task();
    virtual ~Task() = default;
    virtual void run(sd_ctx_t* ctx) = 0;
};

std::string taskStatusToString(TaskStatus status);

class Worker {
private:
    sd_ctx_t* ctx;

public:
    std::string name;

    Worker(const std::string& name);
    void run(std::queue<std::shared_ptr<Task>>& pendingTasks,
             std::unordered_map<std::string, std::shared_ptr<Task>>& finishedTasks,
             std::mutex& queueMutex,
             std::condition_variable& taskNotifier,
             std::atomic<bool>& stopFlag);
};

class TaskManager {
private:
    std::vector<std::unique_ptr<Worker>> workers;
    std::queue<std::shared_ptr<Task>> pendingTasks;
    std::unordered_map<std::string, std::shared_ptr<Task>> finishedTasks;
    std::mutex queueMutex;
    std::condition_variable taskNotifier;
    std::atomic<bool> stopFlag;

    void cleanupFinishedTasks();

public:
    TaskManager(const std::vector<std::string>& workerNames);
    ~TaskManager();

    void addTask(std::shared_ptr<Task> task);
    std::shared_ptr<Task> getFinishedTask(const std::string& taskId);
};

#endif // TASK_MANAGER_H
