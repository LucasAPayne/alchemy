#include "alchemy/util/time.h"

#if _WIN32
    #include <windows.h>
#endif

void timer_init(Timer* timer, f32 start_seconds, b32 start_active)
{
    timer->start_seconds = start_seconds;
    timer->ms_left = start_seconds * 1000.0f;
    timer->is_active = start_active;
}

f32 timer_milliseconds(const Timer* timer)
{
    return timer->ms_left;
}

f32 timer_seconds(const Timer* timer)
{
    return timer->ms_left / 1000.0f;
}

void timer_start(Timer* timer)
{
    timer->is_active = true;
}

void timer_update(Timer* timer, f32 delta_time, b32 reset_on_end)
{
    if (timer->is_active)
        timer->ms_left -= delta_time*1000.0f;

    if (timer->ms_left <= 0.0f)
    {
        timer->is_active = false;
        if (reset_on_end)
            timer_reset(timer);
        else
            timer->ms_left = 0.0f;
    }
}

void timer_stop(Timer* timer)
{
    timer->is_active = false;
}

void timer_reset(Timer* timer)
{
    timer->ms_left = timer->start_seconds * 1000.0f;
}

void stopwatch_init(Stopwatch* stopwatch, b32 start_active)
{
    stopwatch->ms_elapsed = 0.0f;
    stopwatch->is_active = start_active;
}

f32  stopwatch_milliseconds(const Stopwatch* stopwatch)
{
    return stopwatch->ms_elapsed;
}

f32  stopwatch_seconds(const Stopwatch* stopwatch)
{
    return stopwatch->ms_elapsed / 1000.0f;
}

void stopwatch_start(Stopwatch* stopwatch)
{
    stopwatch->is_active = true;
}

void stopwatch_update(Stopwatch* stopwatch, f32 delta_time)
{
    if (stopwatch->is_active)
        stopwatch->ms_elapsed += delta_time*1000.0f;
}

void stopwatch_stop(Stopwatch* stopwatch)
{
    stopwatch->is_active = false;
}

void stopwatch_reset(Stopwatch* stopwatch)
{
    stopwatch->ms_elapsed = 0.0f;
}

LocalTime get_local_time(void)
{
    LocalTime result = {0};

#if _WIN32
    SYSTEMTIME lt;
    GetLocalTime(&lt);
    result.hour = lt.wHour;
    result.minute = lt.wMinute;
    result.second = lt.wSecond;
    result.millisecond = lt.wMilliseconds;
#endif

    return result;
}
