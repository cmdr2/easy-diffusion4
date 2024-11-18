#ifndef __ED__TASK_MANAGER_H__
#define __ED__TASK_MANAGER_H__

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

enum class TaskStatus { PENDING, RUNNING, ERROR, FINISHED };

struct sd_ctx_t {
    // Context-specific data
};

struct Task {
    std::string taskId;
    TaskStatus status = TaskStatus::PENDING;
    std::string responseData;

    virtual ~Task() = default;
    virtual void run(sd_ctx_t& ctx) = 0;
};

class Worker {
private:
    sd_ctx_t ctx;

public:
    Worker();
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

public:
    TaskManager(const std::vector<std::string>& workerNames);
    ~TaskManager();

    void addTask(std::shared_ptr<Task> task);
    std::shared_ptr<Task> getFinishedTask(const std::string& taskId);
};

#endif // TASK_MANAGER_H
