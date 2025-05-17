#include "alchemy/util/log.h"
#include "alchemy/util/time.h"
#include "alchemy/util/types.h"

#include <stdarg.h>
#include <stdio.h>

global const char* level_strs[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL", "SUCCESS", "FAILURE"};
global const char* level_colors[] = {BWHITE, BBLUE, BCYAN, BYELLOW, BRED, REDB, BGREEN, BRED};

void log_log(LogLevel level, const char* fmt, ...)
{
    LocalTime lt = get_local_time();

    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "[%02d:%02d:%02d.%03d] %s[%s]:%s ", lt.hour, lt.minute, lt.second, lt.millisecond,
        level_colors[level], level_strs[level], COLOR_RESET);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}
