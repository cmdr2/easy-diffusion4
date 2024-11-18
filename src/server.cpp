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

    CROW_ROUTE(app, "/api/tasks").methods("POST"_method)([&](const crow::request& req) {
        auto json = crow::json::load(req.body);
        if (!json) return crow::response(400, "Invalid JSON");

        std::shared_ptr<Task> task;

        std::string type = json["type"].s();
        if (type == "generate_image") {
            GenerateImageInputData generateRequest = GenerateImageInputData::deserialize(json);
            task = std::make_shared<GenerateImageTask>(generateRequest);
        } else if (type == "filter_image") {
            FilterImageInputData filterRequest = FilterImageInputData::deserialize(json);
            task = std::make_shared<FilterImageTask>(filterRequest);
        } else {
            return crow::response(400, "Unknown task type");
        }

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

        return crow::response(result.dump());
    });

    CROW_ROUTE(app, "/api/tasks/<string>/artifacts")([&](const std::string& task_id) {
        auto task = taskManager.getFinishedTask(task_id);
        if (!task || task->status != TaskStatus::FINISHED) {
            return crow::response(404);
        }

        crow::response res(200);
        res.set_header("Content-Type", task->output_data_type);
        res.body.assign(reinterpret_cast<char*>(task->output_data), task->output_data_size);

        return res;
    });

    std::thread([&]() {
        // Delay to ensure server starts before opening browser
        std::this_thread::sleep_for(std::chrono::seconds(1));
        openInDefaultBrowser("http://localhost:8080/");
    }).detach();

    app.loglevel(crow::LogLevel::Warning);

    app.port(8080).multithreaded().run();
}
