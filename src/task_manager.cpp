#include "task_manager.h"
#include <thread>

Worker::Worker() {
    ctx = sd_ctx_t{}; // Initialize the worker-specific context
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
            task->run(ctx);
            task->status = TaskStatus::FINISHED;
        } catch (...) {
            task->status = TaskStatus::ERROR;
            task->responseData = "Error while executing task.";
        }

        {
            std::lock_guard<std::mutex> lock(queueMutex);
            finishedTasks[task->taskId] = task;
        }
    }
}

TaskManager::TaskManager(const std::vector<std::string>& workerNames) : stopFlag(false) {
    for (const auto& name : workerNames) {
        workers.emplace_back(std::make_unique<Worker>());
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
    std::lock_guard<std::mutex> lock(queueMutex);
    auto it = finishedTasks.find(taskId);
    if (it != finishedTasks.end()) {
        return it->second;
    }
    return nullptr;
}
