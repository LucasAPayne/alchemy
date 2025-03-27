#include "alchemy/util/file.h"

#include <sys/stat.h>
#include <windows.h>

HANDLE win32_file_open_normal_read(char* filename)
{
    HANDLE file = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    return file;
}

b32 file_exists(char* filename)
{
    DWORD attrib = GetFileAttributesA(filename);
    b32 result = (attrib != INVALID_FILE_ATTRIBUTES && !(attrib & FILE_ATTRIBUTE_DIRECTORY));
    return result;
}

size file_get_size(char* filename)
{
    usize file_size = 0;
    LARGE_INTEGER fsize;
    HANDLE file = win32_file_open_normal_read(filename);
    if (file == INVALID_HANDLE_VALUE)
    {
        log_error("Failed to open file %s", filename);
        return file_size;
    }

    GetFileSizeEx(file, &fsize);
    file_size = fsize.QuadPart;
    return file_size;
}

void* file_open(char* filename, FileMode mode)
{
    DWORD file_access = 0;
    DWORD file_share = 0;
    if (mode | FileMode_Read)
    {
        file_access |= GENERIC_READ;
        file_share |= FILE_SHARE_READ;
    }
    if (mode | FileMode_Write)
    {
        file_access |= GENERIC_WRITE;
        file_share |= FILE_SHARE_WRITE;
    }
    HANDLE file = CreateFileA(filename, file_access, file_share, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    return file;
}

void file_close(void* file_handle)
{
    if (CloseHandle(file_handle) == FALSE)
        log_error("Failed to close file");
}

u64 file_get_last_write_time(char* filename)
{
    u64 last_write_time = 0;
    HANDLE file = win32_file_open_normal_read(filename);
    if (file == INVALID_HANDLE_VALUE)
    {
        log_error("Failed to open file %s", filename);
        return last_write_time;
    }

    FILETIME creation_time, access_time, write_time;
    if (!GetFileTime(file, &creation_time, &access_time, &write_time))
    {
        log_error("Failed to get file time for %s", filename);
        CloseHandle(file);
        return last_write_time;
    }
    CloseHandle(file);

    last_write_time = u64_high_low(write_time.dwHighDateTime, write_time.dwLowDateTime);
    return last_write_time;
}

b32 file_is_modified(char* filename, u64 reference_time)
{
    b32 result = false;
    u32 ref_time_hi = (u32)(reference_time >> 32);
    u32 ref_time_lo = (u32)reference_time;
    FILETIME ref_time = {.dwLowDateTime=ref_time_lo, .dwHighDateTime=ref_time_hi};
    HANDLE file = win32_file_open_normal_read(filename);
    if (file == INVALID_HANDLE_VALUE)
    {
        log_error("Failed to open file %s", filename);
        return result;
    }

    FILETIME creation_time, access_time, write_time;
    if (!GetFileTime(file, &creation_time, &access_time, &write_time))
    {
        log_error("Failed to get file time for %s", filename);
        CloseHandle(file);
        return result;
    }
    CloseHandle(file);

    result = CompareFileTime(&write_time, &ref_time);
    return result;
}

s8 file_to_string(char* filename, MemoryArena* arena)
{
    size file_size = file_get_size(filename);
    s8 result = s8_alloc(arena, file_size);
    u8* begin = result.data;

    HANDLE file = win32_file_open_normal_read(filename);
    if (file == INVALID_HANDLE_VALUE)
    {
        log_error("Failed to open file %s", filename);
        memory_arena_pop(arena, file_size);
        return s8("");
    }

    u64 size_remaining = file_size;
    while (size_remaining)
    {
        u32 read_size = (u32)-1;
        if ((u64)read_size > size_remaining)
            read_size = (u32)size_remaining;

        DWORD bytes_read = 0;
        BOOL read_success = ReadFile(file, result.data, read_size, &bytes_read, 0);

        if (!read_success)
        {
            LPVOID msg_buf;
            DWORD error = GetLastError();
            FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                           NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &msg_buf, 0, NULL);

            log_error("Failed to read file %s", filename);
            MessageBoxA(NULL, (LPCTSTR)msg_buf, TEXT("Error"), MB_ICONERROR);
            LocalFree(msg_buf);
            CloseHandle(file);
            memory_arena_pop(arena, file_size);
            return s8("");
        }

        size_remaining -= read_size;
        result.data += read_size;
    }
    result.data = begin;
    CloseHandle(file);
    return result;
}

void file_write_byte(void* file_handle, size offset, u8 byte)
{
    LARGE_INTEGER off;
    off.QuadPart = offset;
    if (SetFilePointerEx(file_handle, off, NULL, FILE_BEGIN) == FALSE)
    {
        log_error("Failed file seek to location %llu", offset);
        return;
    }

    DWORD bytes_written = 0;
    if (!WriteFile(file_handle, &byte, 1, &bytes_written, NULL) == FALSE)
    {
        log_error("Failed to write byte to location %llu in file", offset);
    }
}
