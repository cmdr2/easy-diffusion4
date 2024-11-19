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
        auto task = taskManager.getTask(task_id);
        if (!task) return crow::response(404);

        crow::json::wvalue result{
            {"taskId", task->taskId},
            {"status", taskStatusToString(task->status)},
            {"progress", task->progress},
            {"enqueued_time", toSecondsSinceEpochDouble(task->enqueued_time)}
        };

        if (task->status == TaskStatus::PENDING) {
            return crow::response(result.dump());
        }

        result["start_time"] = toSecondsSinceEpochDouble(task->start_time);

        if (task->status == TaskStatus::FAILED) {
            result["error"] = task->error;
        } else if (task->status == TaskStatus::RUNNING) {
            result["outputs"] = "/api/tasks/" + task_id + "/outputs";
        } else { // FINISHED
            result["finished_time"] = toSecondsSinceEpochDouble(task->finished_time);
            result["time_taken"] = toSecondsSinceEpochDouble(task->finished_time) - toSecondsSinceEpochDouble(task->start_time);
        }

        return crow::response(result.dump());
    });


    // Route to list all outputs for a given task
    CROW_ROUTE(app, "/api/tasks/<string>/outputs")([&](const std::string& task_id) {
        auto task = taskManager.getTask(task_id);
        if (!task || task->status == TaskStatus::PENDING || task->status == TaskStatus::FAILED) {
            return crow::response(404);
        }

        if (task->output_data.empty()) {
            return crow::response(404);
        }

        crow::json::wvalue output_list = crow::json::wvalue::list();
        for (size_t i = 0; i < task->output_data.size(); ++i) {
            output_list[i] = "/api/tasks/" + task_id + "/outputs/" + std::to_string(i);
        }

        return crow::response(200, output_list.dump());
    });


    // Route to fetch a specific output based on output_id
    CROW_ROUTE(app, "/api/tasks/<string>/outputs/<int>")([&](const std::string& task_id, int output_id) {
        auto task = taskManager.getTask(task_id);
        if (!task || task->status == TaskStatus::PENDING || task->status == TaskStatus::FAILED) {
            return crow::response(404);
        }

        // Check if output_data is unassigned or empty
        if (task->output_data.empty()) {
            return crow::response(404);
        }

        if (output_id < 0 || output_id >= static_cast<int>(task->output_data.size())) {
            return crow::response(400, "Invalid output ID");
        }

        crow::response res(200);
        res.set_header("Content-Type", task->output_data_type);
        res.body.assign(
            reinterpret_cast<const char*>(task->output_data[output_id].data()),
            task->output_data[output_id].size()
        );

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
