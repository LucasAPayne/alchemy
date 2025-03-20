#pragma once

#include "alchemy/input.h"
#include "alchemy/window.h"
#include "alchemy/renderer/renderer.h"
#include "alchemy/util/memory.h"
#include "alchemy/util/types.h"

#define UPDATE_AND_RENDER(name) void name(GameMemory* memory, Input* input, Renderer* renderer, Window window, f32 delta_time)
typedef UPDATE_AND_RENDER(UpdateAndRender);

// TODO(lucas): This #define is based on MAX_PATH from windows.h
// This needs to be changed in the future.
#define MAX_FILEPATH_LEN 260

typedef struct RecordedInput
{
    int input_count;
    Input* input_stream;
} RecordedInput;

typedef struct ReplayBufer
{
    void* file_handle;
    void* memory_map;
    char filename[MAX_FILEPATH_LEN];
    void* memory_block; // Copy of game memory
    size total_bytes;
    void* recording_handle;
    void* playback_handle;
    b32 is_recording;
    b32 is_playing;
} ReplayBuffer;

typedef struct GameCode
{
    void* game_code_dll;
    u64 dll_last_write_time;

    // NOTE(lucas): Callback can be NULL
    // Must check before calling
    UpdateAndRender* update_and_render;

    b32 is_valid;

    char exe_full_path[MAX_FILEPATH_LEN];
    char dll_full_path[MAX_FILEPATH_LEN];
    char temp_dll_full_path[MAX_FILEPATH_LEN];
    char lock_file_full_path[MAX_FILEPATH_LEN];

    ReplayBuffer replay_buffer;
} GameCode;

GameCode game_code_load(char* source_dll_name, char* temp_dll_name, char* lock_file_full_path);
void game_code_unload(GameCode* game_code);
void game_code_update(GameCode* game_code);

// Looped live code editing
void input_loop_init(GameCode* game_code, GameMemory* game_memory);
void input_loop_update(GameCode* game_code, GameMemory* game_memory, Input* input);
