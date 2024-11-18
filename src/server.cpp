#include "server.h"
#include "util.h"
#include <thread>
#include <crow.h>

#include "image_tasks.h"

Server::Server(TaskManager& manager) : taskManager(manager) {}

void Server::start() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([&]() {
        std::ifstream file("ui/index.html");
        if (!file) return crow::response(404);

        std::stringstream buffer;
        buffer << file.rdbuf();
        crow::response res(200, buffer.str());
        res.set_header("Content-Type", "text/html");
        return res;
    });

    CROW_ROUTE(app, "/api/tasks").methods("POST"_method)([&]() {
        auto task = std::make_shared<RenderImageTask>();
        taskManager.addTask(task);
        crow::json::wvalue result{{"taskId", task->taskId}};
        return crow::response(result.dump());
    });

    CROW_ROUTE(app, "/api/tasks/<string>")([&](const std::string& task_id) {
        auto task = taskManager.getFinishedTask(task_id);
        if (!task) return crow::response(404);

        crow::json::wvalue result{
            {"taskId", task->taskId},
            {"status", taskStatusToString(task->status)},
            {"enqueued_time", toSecondsSinceEpochDouble(task->enqueued_time)}
        };

        if (task->status != TaskStatus::PENDING) {
            result["start_time"] = toSecondsSinceEpochDouble(task->start_time);

            if (task->status != TaskStatus::RUNNING) {
                result["finished_time"] = toSecondsSinceEpochDouble(task->finished_time);
                result["time_taken"] = toSecondsSinceEpochDouble(task->finished_time) - toSecondsSinceEpochDouble(task->start_time);
            }
        }

        if (task->status != TaskStatus::PENDING) {
            result["artifacts"] = "/api/tasks/" + task->taskId + "/artifacts";
        }

        return crow::response(result.dump());
    });

    CROW_ROUTE(app, "/api/tasks/<string>/artifacts")([&](const std::string& task_id) {
        auto task = taskManager.getFinishedTask(task_id);
        if (!task || task->status != TaskStatus::FINISHED) return crow::response(404);

        return crow::response(task->responseData);
    });

    std::thread([&]() {
        // Delay to ensure server starts before opening browser
        std::this_thread::sleep_for(std::chrono::seconds(1));
        openInDefaultBrowser("http://localhost:8080/");
    }).detach();

    app.loglevel(crow::LogLevel::Warning);

    app.port(8080).multithreaded().run();
}
