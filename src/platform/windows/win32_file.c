#include "alchemy/util/file.h"
#include "win32_base.c"

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
    file_close(file);
    return file_size;
}

void* file_open(char* filename, FileMode mode)
{
    DWORD file_access = 0;
    DWORD file_share = 0;

    DWORD creation_disposition = 0;
    if (!file_exists(filename))
        creation_disposition = CREATE_NEW;
    else
        creation_disposition = OPEN_EXISTING;

    if (mode | FileMode_Read)
    {
        file_access |= GENERIC_READ;
        file_share |= FILE_SHARE_READ;
    }
    if (mode | FileMode_Write || mode | FileMode_Append)
    {
        file_access |= GENERIC_WRITE;
        file_share |= FILE_SHARE_WRITE;
    }
    if (mode | FileMode_Append)
    {
        file_access |= FILE_APPEND_DATA;
        file_access |= FILE_SHARE_READ;
    }
    HANDLE file = CreateFileA(filename, file_access, file_share, NULL, creation_disposition, FILE_ATTRIBUTE_NORMAL, NULL);

    if (file == INVALID_HANDLE_VALUE)
    {
        log_error("CreateFileA failed");
        win32_error_callback();
        return 0;
    }

    return file;
}

void file_close(void* file_handle)
{
    // TODO(lucas): For any failure to operate on a file, make sure to log the filename.
    // This will require a function to get the filename from a handle.
    if (CloseHandle(file_handle) == FALSE)
    {
        log_error("Failed to close file");
        win32_error_callback();
    }
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
        file_close(file);
        return last_write_time;
    }
    file_close(file);

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
        file_close(file);
        return result;
    }
    file_close(file);

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
            log_error("Failed to read file %s", filename);
            file_close(file);
            memory_arena_pop(arena, file_size);
            return s8("");
        }

        size_remaining -= read_size;
        result.data += read_size;
    }
    result.data = begin;
    file_close(file);
    return result;
}

char* get_filename(void* file_handle)
{
    if (file_handle == INVALID_HANDLE_VALUE)
    {
        log_warn("get_filename() received invalid file handle");
        return 0;
    }

    DWORD len = GetFinalPathNameByHandleA(file_handle, NULL, 0, FILE_NAME_NORMALIZED);
    if (len == 0)
    {
        log_warn("Failed to get filename");
        return 0;
    }

    char* filename = malloc(len+1);
    ASSERT(filename, "Failed to allocate memory");
    if (!filename)
        return 0;

    len = GetFinalPathNameByHandleA(file_handle, filename, MAX_PATH, FILE_NAME_NORMALIZED);
    if (len == 0)
    {
        free(filename);
        log_warn("Failed to get filename");
        return 0;
    }

    return filename;
}

i64 file_seek(void* file_handle, i64 byte_offset, FileSeekMethod seek_method)
{
    ASSERT(file_handle, "Invalid file handle");

    LARGE_INTEGER dist_to_move = {0};
    dist_to_move.QuadPart = byte_offset;
    LARGE_INTEGER new_ptr = {0};

    DWORD method = 0;
    switch (seek_method)
    {
        case FileSeek_Begin:   method = FILE_BEGIN;   break;
        case FileSeek_Current: method = FILE_CURRENT; break;
        case FileSeek_End:     method = FILE_END;     break; 
        default: log_error("Invalid file seek method: %d", seek_method); break;
    }

    b32 success = SetFilePointerEx(file_handle, dist_to_move, &new_ptr, method);
    if (success == FALSE)
    {
        char* filename = get_filename(file_handle);
        log_warn("Failed to move file pointer %lld bytes using seek method %d in file %s",
                 byte_offset, seek_method, filename);
        free(filename);
        win32_error_callback();
    }
    i64 result = new_ptr.QuadPart;
    return result;
}

i64 file_seek_begin(void* file_handle)
{
    return file_seek(file_handle, 0, FileSeek_Begin);
}

i64 file_seek_end(void* file_handle)
{
    return file_seek(file_handle, 0, FileSeek_End);
}

// TODO(lucas): Consider reading from/writing to files >4GB, similar to file_to_string()
int file_read(void* file_handle, void* buffer, size num_bytes_to_read)
{
    ASSERT(file_handle, "Invalid file handle");
    DWORD num_bytes_read = 0;
    b32 success = ReadFile(file_handle, buffer, (u32)num_bytes_to_read, &num_bytes_read, NULL);
    if (success == FALSE)
    {
        char* filename = get_filename(file_handle);
        log_warn("Failed to read from file %s", filename);
        free(filename);
        win32_error_callback();
    }
    if (num_bytes_read != num_bytes_to_read)
    {
        char* filename = get_filename(file_handle);
        log_warn("Number of bytes read (%u) does not match expected number of bytes (%u) in file %s",
                 num_bytes_read, num_bytes_to_read, filename);
        free(filename);
        win32_error_callback();
    }

    return num_bytes_read;
}

int file_write(void* file_handle, void* buffer, size num_bytes_to_write)
{
    ASSERT(file_handle, "Invalid file handle");
    DWORD num_bytes_written = 0;
    b32 success = WriteFile(file_handle, buffer, (u32)num_bytes_to_write, &num_bytes_written, NULL);
    if (success == FALSE || (num_bytes_to_write != num_bytes_written))
    {
        char* filename = get_filename(file_handle);
        log_warn("Failed to write to file %s", filename);
        win32_error_callback();
        free(filename);
    }

    return num_bytes_written;
}

int file_write_byte(void* file_handle, size offset, u8 byte)
{
    ASSERT(file_handle, "Invalid file handle");
    LARGE_INTEGER off;
    off.QuadPart = offset;
    if (SetFilePointerEx(file_handle, off, NULL, FILE_BEGIN) == FALSE)
    {
        char* filename = get_filename(file_handle);
        log_error("Failed file seek to location %llu", offset);
        free(filename);
        return 0;
    }

    DWORD num_bytes_written = 0;
    if (!WriteFile(file_handle, &byte, 1, &num_bytes_written, NULL) == FALSE)
    {
        char* filename = get_filename(file_handle);
        log_error("Failed to write byte to location %llu in file %s", offset, filename);
        free(filename);
    }

    return num_bytes_written;
}
