#include "state.h"

#include <windows.h>

// TODO(lucas): Move these out into a string library when necessary
internal int str_len(char* s)
{
    int count = 0;
    while(*s++)
        ++count;
    
    return count;
}

internal void str_cat(char* source_a, usize source_a_len, char* source_b, usize source_b_len,
                 char* dest, usize dest_len)
{
    // TODO(lucas): dest bounds checking
    // TODO(lucas): Check for null terminator before adding

    // For now, just loop through each source string and add each character one at a time to the dest string
    for (usize i = 0; i < source_a_len; ++i)
        *dest++ = *source_a++;

    for (usize i = 0; i < source_b_len; ++i)
        *dest++ = *source_b++;

    *dest++ = 0;
}

internal void win32_get_exe_filname(GameCode* game_code)
{
    // NOTE(lucas): Never use MAX_PATH in code that is user-facing because it can be dangerous and lead to bad results
    // Get full path to game DLL
    DWORD size_of_filename = GetModuleFileNameA(NULL, game_code->exe_filename, sizeof(game_code->exe_filename));
    game_code->one_past_last_slash = game_code->exe_filename;
    for (char* scan = game_code->exe_filename; *scan; ++scan)
    {
        if (*scan == '\\')
            game_code->one_past_last_slash = scan + 1;
    }
}

internal void win32_build_exe_path_filename(GameCode* game_code, char* filename, char* dest, int dest_len)
{
    usize exe_filename_len = game_code->one_past_last_slash - game_code->exe_filename;
    str_cat(game_code->exe_filename, exe_filename_len, filename, str_len(filename), dest, dest_len);
}

internal void win32_get_input_file_location(GameCode* game_code, b32 input_stream, int slot_index,
                                            char* dest, int dest_len)
{
    char filename[64];
    wsprintf(filename, "loop_edit_%d_%s.input", slot_index, input_stream ? "input" : "state");
    win32_build_exe_path_filename(game_code, filename, dest, dest_len);
}

internal FILETIME win32_get_last_write_time(char* filename)
{
    FILETIME last_write_time = {0};

    WIN32_FILE_ATTRIBUTE_DATA data;
    if (GetFileAttributesExA(filename, GetFileExInfoStandard, &data))
        last_write_time = data.ftLastWriteTime;

    return last_write_time;
}

GameCode game_code_load(char* source_dll_name, char* temp_dll_name, char* lock_file_name)
{
    GameCode result = {0};

    win32_get_exe_filname(&result);
    char source_dll_full_path[MAX_FILENAME_LEN];
    win32_build_exe_path_filename(&result, source_dll_name, source_dll_full_path, sizeof(source_dll_full_path));

    char temp_dll_full_path[MAX_FILENAME_LEN];
    win32_build_exe_path_filename(&result, temp_dll_name, temp_dll_full_path, sizeof(temp_dll_full_path));

    char lock_file_full_path[MAX_FILENAME_LEN];
    win32_build_exe_path_filename(&result, lock_file_name, lock_file_full_path, sizeof(lock_file_full_path));

    // Make sure lock file has been deleted before loading game code.
    // This ensures all necessary resources, including PDBs, are fully loaded.
    WIN32_FILE_ATTRIBUTE_DATA ignored;
    if (!GetFileAttributesExA(lock_file_full_path, GetFileExInfoStandard, &ignored))
    {
        FILETIME file_time = win32_get_last_write_time(source_dll_full_path);
        result.dll_last_write_time = (u64)file_time.dwHighDateTime << 32 | file_time.dwLowDateTime;
        CopyFileA(source_dll_full_path, temp_dll_full_path, FALSE);
        result.game_code_dll = LoadLibraryA(temp_dll_full_path);
        if (result.game_code_dll)
        {
            result.update_and_render = (UpdateAndRender*)GetProcAddress(result.game_code_dll, "update_and_render");

            result.is_valid = (result.update_and_render != 0);
        }
        if (!result.is_valid)
            result.update_and_render = NULL;
    }

    return result;
}

void game_code_unload(GameCode* game_code)
{
    if (game_code->game_code_dll)
        FreeLibrary(game_code->game_code_dll);

    game_code->is_valid = false;
    game_code->update_and_render = NULL;
}
