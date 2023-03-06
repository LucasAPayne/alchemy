#pragma once

#include "types.h"

struct SoundOutput
{
    f32 volume; // value between 0 and 1
    b32 should_play;
    char filename[512]; // Temporary
};

inline void set_volume(SoundOutput* sound_output, f32 volume)
{
    sound_output->volume = volume;
}
