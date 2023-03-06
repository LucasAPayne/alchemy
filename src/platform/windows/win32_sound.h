#pragma once

#include "sound.h"
#include "types.h"

#include <xaudio2.h>

// TODO(lucas): Is there a better name for this struct?
struct Win32Xaudio2State
{
    IXAudio2* xaudio2;
    IXAudio2MasteringVoice* master_voice;

};

void win32_init_xaudio2(Win32Xaudio2State* xaudio2_state);
void win32_process_sound_output(SoundOutput* sound_output, Win32Xaudio2State* xaudio2_state);
