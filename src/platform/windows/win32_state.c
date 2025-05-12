#include "alchemy/state.h"
#include "alchemy/util/str.h"

#include <windows.h>

internal char* win32_filename_from_full_path(char* full_path)
{
    char* filename = full_path;
    for (char* scan = full_path; *scan; ++scan)
    {
        if (*scan == '\\')
            filename = scan + 1;
    }
    return filename;
}

internal void win32_build_exe_path_filename(GameCode* game_code, char* filename, char* dest, int dest_len)
{
    char* one_past_last_slash = win32_filename_from_full_path(game_code->exe_full_path);
    size exe_filename_len = one_past_last_slash - game_code->exe_full_path;
    str_cat(game_code->exe_full_path, exe_filename_len, filename, str_len(filename), dest, dest_len);
}

internal FILETIME win32_get_last_write_time(char* filename)
{
    FILETIME last_write_time = {0};

    WIN32_FILE_ATTRIBUTE_DATA data;
    if (GetFileAttributesExA(filename, GetFileExInfoStandard, &data))
        last_write_time = data.ftLastWriteTime;

    return last_write_time;
}

GameCode game_code_load(char* dll_name)
{
    GameCode result = {0};

#ifndef ALCHEMY_NO_HOT_RELOAD

    char temp_dll_name[MAX_FILEPATH_LEN];
    char lock_file_name[MAX_FILEPATH_LEN];
    char temp_dll_full_path[MAX_FILEPATH_LEN];
    char lock_file_full_path[MAX_FILEPATH_LEN];

    const char* ext = str_find_last(dll_name, '.');
    size base_len = ext ? (size)(ext - dll_name) : str_len(dll_name);

    snprintf(temp_dll_name, sizeof(temp_dll_name), "%.*s_temp.dll", (int)base_len, dll_name);
    snprintf(lock_file_name, sizeof(temp_dll_name), "%.*s_lock.dll", (int)base_len, dll_name);

    // NOTE(lucas): Never use MAX_PATH in code that is user-facing because it can be dangerous and lead to bad results
    // Get full path to game DLL
    DWORD size_of_filename = GetModuleFileNameA(NULL, result.exe_full_path, sizeof(result.exe_full_path));

    win32_build_exe_path_filename(&result, dll_name, result.dll_full_path, sizeof(result.dll_full_path));
    win32_build_exe_path_filename(&result, temp_dll_name, temp_dll_full_path, sizeof(temp_dll_full_path));
    win32_build_exe_path_filename(&result, lock_file_name, lock_file_full_path, sizeof(lock_file_full_path));

    // Make sure lock file has been deleted before loading game code.
    // This ensures all necessary resources, including PDBs, are fully loaded.
    WIN32_FILE_ATTRIBUTE_DATA ignored;
    if (!GetFileAttributesExA(lock_file_full_path, GetFileExInfoStandard, &ignored))
    {
        FILETIME file_time = win32_get_last_write_time(result.dll_full_path);
        result.dll_last_write_time = (u64)file_time.dwHighDateTime << 32 | file_time.dwLowDateTime;
        CopyFileA(result.dll_full_path, temp_dll_full_path, FALSE);
        result.game_code_dll = LoadLibraryA(temp_dll_full_path);
        if (result.game_code_dll)
        {
            result.update_and_render = (UpdateAndRender*)GetProcAddress(result.game_code_dll, "update_and_render");

            result.is_valid = (result.update_and_render != 0);
        }
        if (!result.is_valid)
            result.update_and_render = NULL;
    }
#endif

    return result;
}

void game_code_unload(GameCode* game_code)
{
#ifndef ALCHEMY_NO_HOT_RELOAD

    if (game_code->game_code_dll)
        FreeLibrary(game_code->game_code_dll);

    game_code->is_valid = false;
    game_code->update_and_render = NULL;

#endif
}

// Check if game DLL has been updated, and if so, reload it.
void game_code_update(GameCode* game_code)
{
#ifndef ALCHEMY_NO_HOT_RELOAD

    // NOTE(lucas): Preserve input looping info
    ReplayBuffer replay_buffer = game_code->replay_buffer;

    FILETIME new_dll_write_time = win32_get_last_write_time(game_code->dll_full_path);
    LARGE_INTEGER li = {0};
    li.QuadPart = game_code->dll_last_write_time;
    FILETIME dll_last_write_time = {0};
    dll_last_write_time.dwLowDateTime = li.LowPart;
    dll_last_write_time.dwHighDateTime = li.HighPart;

    if (CompareFileTime(&new_dll_write_time, &dll_last_write_time) != 0)
    {
        char* dll_filename = win32_filename_from_full_path(game_code->dll_full_path);

        game_code_unload(game_code);
        *game_code = game_code_load(dll_filename);
        game_code->replay_buffer = replay_buffer;
    }

#endif
}

internal void input_loop_get_file_location(GameCode* game_code, b32 input_stream, char* dest, int dest_len)
{
#ifndef ALCHEMY_NO_HOT_RELOAD

    char filename[64];
    wsprintf(filename, "loop_edit_%s.input", input_stream ? "input" : "state");
    win32_build_exe_path_filename(game_code, filename, dest, dest_len);

#endif
}

internal void input_loop_begin_recording(GameCode* game_code, GameMemory* game_memory)
{
#ifndef ALCHEMY_NO_HOT_RELOAD

    ReplayBuffer* replay_buffer = &game_code->replay_buffer;
    if (!replay_buffer->memory_block)
    {
        log_error("Input loop replay buffer is invalid");
        return;
    }

    char filename[MAX_FILEPATH_LEN];
    input_loop_get_file_location(game_code, true, filename, sizeof(filename));
    replay_buffer->recording_handle = CreateFileA(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    replay_buffer->is_recording = true;

    CopyMemory(replay_buffer->memory_block, game_memory->memory_block, game_memory->total_bytes);

#endif
}

internal void input_loop_end_recording(GameCode* game_code)
{
#ifndef ALCHEMY_NO_HOT_RELOAD

    CloseHandle(game_code->replay_buffer.recording_handle);
    game_code->replay_buffer.is_recording = false;

#endif
}

internal void input_loop_record_input(GameCode* game_code, Input* input)
{
#ifndef ALCHEMY_NO_HOT_RELOAD

    DWORD bytes_written;
    WriteFile(game_code->replay_buffer.recording_handle, input, sizeof(*input), &bytes_written, 0);

#endif
}

internal void input_loop_begin_playback(GameCode* game_code, GameMemory* game_memory)
{
#ifndef ALCHEMY_NO_HOT_RELOAD

    ReplayBuffer* replay_buffer = &game_code->replay_buffer;
    if (!replay_buffer->memory_block)
    {
        log_error("Input loop replay buffer is invalid");
        return;
    }

    char filename[MAX_FILEPATH_LEN];
    input_loop_get_file_location(game_code, true, filename, sizeof(filename));
    game_code->replay_buffer.playback_handle = CreateFileA(filename, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
    replay_buffer->is_playing = true;

    CopyMemory(game_memory->memory_block, replay_buffer->memory_block, game_memory->total_bytes);

#endif
}

internal void input_loop_end_playback(GameCode* game_code)
{
#ifndef ALCHEMY_NO_HOT_RELOAD

    CloseHandle(game_code->replay_buffer.playback_handle);
    game_code->replay_buffer.is_playing = false;

#endif
}

internal void input_loop_playback_input(GameCode* game_code, GameMemory* game_memory, Input* input)
{
#ifndef ALCHEMY_NO_HOT_RELOAD

    DWORD bytes_read = 0;
    if (ReadFile(game_code->replay_buffer.playback_handle, input, sizeof(*input), &bytes_read, 0))
    {
        if (bytes_read == 0)
        {
            // NOTE(lucas): We've hit the end of the stream, go back to the beginning
            input_loop_end_playback(game_code);
            input_loop_begin_playback(game_code, game_memory);
            ReadFile(game_code->replay_buffer.playback_handle, input, sizeof(*input), &bytes_read, 0);
        }
    }

#endif
}

void input_loop_init(GameCode* game_code, GameMemory* game_memory)
{
#ifndef ALCHEMY_NO_HOT_RELOAD

    // TODO(lucas): Recording system still seems to take too long on recrod start.
    // Find out what Windows is doing and if we can speed up/defer some of that processing.
    ReplayBuffer* replay_buffer = &game_code->replay_buffer;

    input_loop_get_file_location(game_code, false, replay_buffer->filename, sizeof(replay_buffer->filename));

    replay_buffer->file_handle = CreateFileA(replay_buffer->filename, GENERIC_READ|GENERIC_WRITE,
                                             0, 0, CREATE_ALWAYS, 0, 0);

    LARGE_INTEGER max_size;
    max_size.QuadPart = game_memory->total_bytes;
    replay_buffer->memory_map = CreateFileMappingA(replay_buffer->memory_map, 0, PAGE_READWRITE,
                                                   max_size.HighPart, max_size.LowPart, 0);
    replay_buffer->memory_block = MapViewOfFile(replay_buffer->memory_map, FILE_MAP_ALL_ACCESS, 0, 0,
                                                game_memory->total_bytes);

    if (!replay_buffer->memory_block)
        log_error("Input loop replay buffer is invalid");

#endif
}

void input_loop_update(GameCode* game_code, GameMemory* game_memory, Input* input)
{
#ifndef ALCHEMY_NO_HOT_RELOAD

    if (key_released(&input->keyboard, KEY_F1))
    {
        if (!game_code->replay_buffer.is_playing)
        {
            if (!game_code->replay_buffer.is_recording)
            {
                input_loop_begin_recording(game_code, game_memory);
            }
            else
            {
                input_loop_end_recording(game_code);
                input_loop_begin_playback(game_code, game_memory);
            }
        }
        else
        {
            input_loop_end_playback(game_code);
        }

    }

    if (game_code->replay_buffer.is_recording)
        input_loop_record_input(game_code, input);
    if (game_code->replay_buffer.is_playing)
        input_loop_playback_input(game_code, game_memory, input);

#endif
}
