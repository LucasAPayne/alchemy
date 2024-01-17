#pragma once

#include "alchemy/util/memory.h"
#include "alchemy/util/types.h"

typedef struct SoundOutput
{
    f32 volume; // value between 0 and 1
    b32 should_play;
    char filename[512]; // Temporary
} SoundOutput;

void sound_output_process(SoundOutput* sound_output, MemoryArena* arena);

inline void sound_output_set_volume(SoundOutput* sound_output, f32 volume)
{
    sound_output->volume = volume;
}
