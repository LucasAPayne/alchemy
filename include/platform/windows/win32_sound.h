#pragma once

#include "sound.h"
#include "util/types.h"

#include <xaudio2.h>

typedef struct XAudio2State
{
    IXAudio2* xaudio2;
    IXAudio2MasteringVoice* master_voice;
} XAudio2State;

XAudio2State xaudio2_state_init(void);
void win32_process_sound_output(SoundOutput* sound_output, XAudio2State* xaudio2_state);
