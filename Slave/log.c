
#include <stdio.h>
#include <time.h>

#include "log.h"

static LOG_LEVEL level = INFO_LOG_LEVEL;
static const char *LOG_LEVEL_STRINGS[] = {"Error", "Warning", "Info"};

void log_set_level(LOG_LEVEL _level)
{
    level = _level;
}
static void log_message(LOG_LEVEL log_level, const char *file, int line, const char *function, const char *msg)
{
    if (level >= log_level)
    {
        time_t now = time(NULL);
        char *time_str = ctime(&now);
        time_str[strlen(time_str) - 1] = '\0'; // Remove newline character

        printf("[%s]: %s %s:%d (%s) - %s\n", LOG_LEVEL_STRINGS[log_level], time_str, file, line, function, msg);
    }
}

void log_error(const char *file, int line, const char *function, const char *msg)
{
    log_message(ERROR_LOG_LEVEL, file, line, function, msg);
}

void log_warning(const char *file, int line, const char *function, const char *msg)
{
    log_message(WARNING_LOG_LEVEL, file, line, function, msg);
}

void log_info(const char *msg)
{
    log_message(INFO_LOG_LEVEL, "", 0, "", msg);
}
