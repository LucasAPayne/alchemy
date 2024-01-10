#pragma once

#include "window.h"
#include "renderer/renderer.h"
#include "util/alchemy_memory.h"
#include "util/types.h"

#define UPDATE_AND_RENDER(name) void name(GameMemory* memory, Renderer* renderer, Window window, f32 delta_time)
typedef UPDATE_AND_RENDER(UpdateAndRender);

// TODO(lucas): This #define is based on MAX_PATH from windows.h
// This needs to be changed in the future.
#define MAX_FILENAME_LEN 260
typedef struct GameCode
{    
    void* game_code_dll;
    u64 dll_last_write_time;

    // NOTE(lucas): Callback can be NULL
    // Must check before calling
    UpdateAndRender* update_and_render;

    b32 is_valid;

    char exe_filename[MAX_FILENAME_LEN];
    char* one_past_last_slash;
} GameCode;

GameCode game_code_load(char* source_dll_name, char* temp_dll_name, char* lock_file_full_path);
void game_code_unload(GameCode* game_code);
