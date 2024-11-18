#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>
#include <uuid_v4.hpp>
#include <cstdlib>

UUIDv4::UUIDGenerator<std::mt19937_64> uuidGenerator;

std::string generate_uuid() {
    // Generate a UUID v4 using the uuid_v4 library
    auto uuid = uuidGenerator.getUUID();

    // Convert the UUID to a string representation
    return uuid.str();
}

// Function to get the current local time as a formatted string
std::string get_local_time() {
    // Get the current time
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);

    // Define a tm struct to hold the local time
    std::tm tm_time;

    // Platform-specific localtime function
#if defined(_WIN32) || defined(_WIN64)
    // Use localtime_s on Windows
    localtime_s(&tm_time, &now_time_t);
#elif defined(__unix__) || defined(__APPLE__) || defined(__linux__)
    // Use localtime_r on Linux/macOS
    localtime_r(&now_time_t, &tm_time);
#else
    #error "Unsupported platform"
#endif

    // Get the milliseconds part
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    // Use a stringstream to format the time
    std::ostringstream oss;
    oss << std::put_time(&tm_time, "%H:%M:%S")   // Format hours, minutes, and seconds
        << '.' << std::setw(3) << std::setfill('0') << milliseconds.count();  // Add milliseconds

    return oss.str();  // Return the formatted string
}

void openInDefaultBrowser(const std::string& url) {
#ifdef _WIN32
    std::string command = "start " + url;
    system(command.c_str());
#elif __APPLE__
    std::string command = "open " + url;
    system(command.c_str());
#elif __linux__
    std::string command = "xdg-open " + url;
    system(command.c_str());
#endif
}

long toSecondsSinceEpochLong(const std::chrono::system_clock::time_point& tp) {
    return std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
}

double toSecondsSinceEpochDouble(const std::chrono::system_clock::time_point& tp) {
    return std::chrono::duration_cast<std::chrono::duration<double>>(tp.time_since_epoch()).count();
}
