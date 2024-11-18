#include <iostream>
#include "stable-diffusion.h"
#include "util.h"

const int MIN_LOG_LEVEL = SD_LOG_DEBUG;
const bool COLOR_LOGGING = true;

/* Enables Printing the log level tag in color using ANSI escape codes */
void sd_log_cb(enum sd_log_level_t level, const char* log, void* data) {
    int tag_color;
    const char* level_str;
    FILE* out_stream = (level == SD_LOG_ERROR) ? stderr : stdout;

    if (!log || level < MIN_LOG_LEVEL) {
        return;
    }

    switch (level) {
        case SD_LOG_DEBUG:
            tag_color = 37;
            level_str = "DEBUG";
            break;
        case SD_LOG_INFO:
            tag_color = 34;
            level_str = "INFO";
            break;
        case SD_LOG_WARN:
            tag_color = 35;
            level_str = "WARN";
            break;
        case SD_LOG_ERROR:
            tag_color = 31;
            level_str = "ERROR";
            break;
        default: /* Potential future-proofing */
            tag_color = 33;
            level_str = "?????";
            break;
    }

    std::string time_str = get_local_time();

    if (COLOR_LOGGING) {
        fprintf(out_stream, "%s \033[%d;1m[%-5s]\033[0m ", time_str.c_str(), tag_color, level_str);
    } else {
        fprintf(out_stream, "%s [%-5s] ", time_str.c_str(), level_str);
    }
    fputs(log, out_stream);
    fflush(out_stream);
}
