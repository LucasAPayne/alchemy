#pragma once

#include "sound.h"
#include "util/types.h"

#include <xaudio2.h>

// TODO(lucas): Is there a better name for this struct?
typedef struct Win32XAudio2State
{
    IXAudio2* xaudio2;
    IXAudio2MasteringVoice* master_voice;
} Win32XAudio2State;

void win32_init_xaudio2(Win32XAudio2State* xaudio2_state);
void win32_process_sound_output(SoundOutput* sound_output, Win32XAudio2State* xaudio2_state);
