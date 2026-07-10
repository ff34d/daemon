#include "include/logger.h"

#include <stdio.h>
#include <time.h>

void log_message(log_level_t level, const char *name, const char *str) {
    time_t raw_time = time(NULL);
    struct tm *time_info = localtime(&raw_time);
    char time_str[9];

    strftime(time_str, sizeof(time_str), "%H:%M:%S", time_info);

    char *level_colors[] = {
        [LOG_INFO] = "\033[0m",
        [LOG_WARN] = "\033[33m",
        [LOG_ERROR] = "\033[31m",
    };

    printf(
        "%s%s [%s]: %s%s\n",
        level_colors[level],
        time_str,
        name,
        str,
        "\033[0m");
}
