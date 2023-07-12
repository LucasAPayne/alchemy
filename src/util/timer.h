#pragma once

#include "util/types.h"

typedef struct Timer
{
    f32 start_seconds;
    f32 ms_left;
    b32 is_active;
} Timer;

void timer_init(Timer* timer, f32 start_seconds, b32 start_active);
f32 timer_milliseconds(const Timer* timer);
f32 timer_seconds(const Timer* timer);
void timer_start(Timer* timer);
void timer_update(Timer* timer, f32 delta_time, b32 reset_on_end);
void timer_stop(Timer* timer);
void timer_reset(Timer* timer);
