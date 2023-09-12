#include "platform/windows/win32_sound.h"
#include "sound.h"
#include "util/types.h"

#include <windows.h>
#include <xaudio2.h>

// Little-Endian
#define FOURCC_RIFF 'FFIR'
#define FOURCC_DATA 'atad'
#define FOURCC_FMT  ' tmf'
#define FOURCC_WAVE 'EVAW'
#define FOURCC_XWMA 'AMWX'
#define FOURCC_DPDS 'sdpd'

// NOTE(lucas): Callback handling comes from https://github.com/tsherif/xaudio2-c-demo/blob/master/xaudio2-c-demo.c
// Set up function callbacks
void OnBufferEnd(IXAudio2VoiceCallback* This, void* pBufferContext) { }
void OnStreamEnd(IXAudio2VoiceCallback* This) { }
void OnVoiceProcessingPassEnd(IXAudio2VoiceCallback* This) { }
void OnVoiceProcessingPassStart(IXAudio2VoiceCallback* This, UINT32 SamplesRequired) { }
void OnBufferStart(IXAudio2VoiceCallback* This, void* pBufferContext) { }
void OnLoopEnd(IXAudio2VoiceCallback* This, void* pBufferContext) { }
void OnVoiceError(IXAudio2VoiceCallback* This, void* pBufferContext, HRESULT Error) { }

// Callbacks are set up by adding them to the lpVtbl, which is type IXAudio2VoiceCallbackVtbl*
global_variable IXAudio2VoiceCallback xaudio_callbacks = {
    .lpVtbl = &(IXAudio2VoiceCallbackVtbl) {
        .OnStreamEnd = OnStreamEnd,
        .OnVoiceProcessingPassEnd = OnVoiceProcessingPassEnd,
        .OnVoiceProcessingPassStart = OnVoiceProcessingPassStart,
        .OnBufferEnd = OnBufferEnd,
        .OnBufferStart = OnBufferStart,
        .OnLoopEnd = OnLoopEnd,
        .OnVoiceError = OnVoiceError
    }
};

void xaudio2_state_init(XAudio2State* xaudio2_state)
{
    xaudio2_state->xaudio2 = NULL;
    if (FAILED(XAudio2Create(&xaudio2_state->xaudio2, 0, XAUDIO2_DEFAULT_PROCESSOR)))
        MessageBoxA(0, "Xaudio2Create failed", "XAudio2 error", MB_OK);

    xaudio2_state->master_voice = NULL;
    if (FAILED(IXAudio2_CreateMasteringVoice(xaudio2_state->xaudio2, &xaudio2_state->master_voice,
                                             XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE,
                                             0, NULL, NULL, AudioCategory_GameEffects)))
        MessageBoxA(0, "Xaudio2Create failed", "XAudio2 error", MB_OK);
}

internal b32 find_chunk(HANDLE file, DWORD fourcc, DWORD* chunk_size, DWORD* chunk_data_pos)
{
    HRESULT hr = S_OK;

    // Set file poitner to beginning of file
    if (INVALID_SET_FILE_POINTER == SetFilePointer(file, 0, NULL, FILE_BEGIN))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        return 0;
    }

    DWORD chunk_type;
    DWORD chunk_data_size;
    DWORD riff_data_size = 0;
    DWORD file_type;
    DWORD bytes_read = 0;
    DWORD offset = 0;

    while (hr == S_OK)
    {
        DWORD read;

        // The first 4 bytes (characters) are the chunk type (e.g., RIFF)
        if (ReadFile(file, &chunk_type, sizeof(DWORD), &read, NULL) == 0)
            hr = HRESULT_FROM_WIN32(GetLastError());
        
        // The next 4 bytes give the size of the data in the file
        if (ReadFile(file, &chunk_data_size, sizeof(DWORD), &read, NULL) == 0)
            hr = HRESULT_FROM_WIN32(GetLastError());
        
        switch(chunk_type)
        {
            case FOURCC_RIFF:
            {
                riff_data_size = chunk_data_size;
                chunk_data_size = 4;

                // The next 4 bytes give the file type (e.g., WAVE)
                if (ReadFile(file, &file_type, sizeof(DWORD), &read, NULL) == 0)
                    hr = HRESULT_FROM_WIN32(GetLastError());
            } break;

            // If first iteration, not a RIFF file
            default:
            {
                // Advance to next chunk
                if (SetFilePointer(file, chunk_data_size, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                    return 0;
                }
            }
        }

        offset += 2 * sizeof(DWORD);

        if (chunk_type == fourcc)
        {
            *chunk_size = chunk_data_size;
            *chunk_data_pos = offset;
            hr = S_OK;
            return 1;
        }

        offset += chunk_data_size;
        if (bytes_read >= riff_data_size)
            return 0;
    }

    return 1;
}

internal b32 read_chunk_data(HANDLE file, void* buffer, DWORD buffer_size, DWORD buffer_offset)
{
    // Set file poitner to buffer_offset bytes from the beginning of the file
    if (SetFilePointer(file, buffer_offset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
        return 0;

    // Fill buffer with contents
    DWORD read;
    if (ReadFile(file, buffer, buffer_size, &read, NULL) == 0)
        return 0;

    return 1;
}

void win32_process_sound_output(SoundOutput* sound_output, XAudio2State* xaudio2_state)
{
    WAVEFORMATEX wave = {0};
    XAUDIO2_BUFFER buffer = {0};
    HANDLE sound_file = CreateFileA(sound_output->filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

    // If handle is invalid, or file poitner cannot be set, return
    if (sound_file == INVALID_HANDLE_VALUE)
    {
        // TODO(lucas): Diagnostic
    }
    if (SetFilePointer(sound_file, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        // TODO(lucas): Diagnostic
    }

    DWORD chunk_size;
    DWORD chunk_pos;
    DWORD file_type;
    
    // Find the "RIFF" chunk and determine file type
    find_chunk(sound_file, FOURCC_RIFF, &chunk_size, &chunk_pos);
    read_chunk_data(sound_file, &file_type, sizeof(DWORD), chunk_pos);

    // File type must be wave
    if (file_type != FOURCC_WAVE)
        return;
    
    // Locate "fmt" chunk and copy contents into wave struct
    find_chunk(sound_file, FOURCC_FMT, &chunk_size, &chunk_pos);
    read_chunk_data(sound_file, &wave, chunk_size, chunk_pos);

    // Locate "data" chunk and read contents into buffer
    find_chunk(sound_file, FOURCC_DATA, &chunk_size, &chunk_pos);
    ubyte* data_buffer = (ubyte*)malloc(chunk_size);
    read_chunk_data(sound_file, data_buffer, chunk_size, chunk_pos);

    buffer.AudioBytes = chunk_size;
    buffer.pAudioData = data_buffer;
    buffer.Flags = XAUDIO2_END_OF_STREAM; // Tell source void not to expect data after this buffer
    
    IXAudio2SourceVoice* source_voice;
    if (FAILED(IXAudio2_CreateSourceVoice(xaudio2_state->xaudio2, &source_voice, &wave, 0, XAUDIO2_DEFAULT_FREQ_RATIO,
                                          &xaudio_callbacks, NULL, NULL)))
    {
        // TODO(lucas): Diagnostic
    }
    
    if (FAILED(IXAudio2SourceVoice_SubmitSourceBuffer(source_voice, &buffer, NULL)))
    {
        // TODO(lucas): Diagnostic
    }

    if (sound_output->should_play)
    {
        if (FAILED(IXAudio2SourceVoice_Start(source_voice, 0, XAUDIO2_COMMIT_NOW)))
        {
            // TODO(lucas): Diagnostic
        }
    }

    if (FAILED(IXAudio2SourceVoice_SetVolume(source_voice, sound_output->volume, 0)))
    {
        // TODO(lucas): Diagnostic
    }
}
