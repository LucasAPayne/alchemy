#pragma once

#include "alchemy/util/memory.h"
#include "alchemy/util/str.h"
#include "alchemy/util/types.h"

typedef enum FileMode
{
    FileMode_Read = (1 << 0),
    FileMode_Write = (1 << 1),
    FileMode_Append = (1 << 2)
} FileMode;

typedef enum FileSeekMethod
{
    FileSeek_Begin = 0,
    FileSeek_Current,
    FileSeek_End
} FileSeekMethod;

// TODO(lucas): Consider overlapped file operations

b32 file_exists(char* filename);
size file_get_size(char* filename); // Returns size in bytes of file
void* file_open(char* filename, FileMode mode); // Opens file with given mode(s) and returns file handle
void file_close(void* file_handle);

u64 file_get_last_write_time(char* filename);
b32 file_is_modified(char* filename, u64 reference_time);

s8 file_to_string(char* filename, MemoryArena* arena);

char* get_filename(void* file);

i64 file_seek(void* file_handle, i64 byte_offset, FileSeekMethod seek_method);
i64 file_seek_begin(void* file_handle);
i64 file_seek_end(void* file_handle);

int file_read(void* file_handle, void* buffer, size num_bytes_to_read);
int file_write(void* file_handle, void* buffer, size num_bytes_to_write);
int file_write_byte(void* file_handle, size offset, u8 byte); // Writes byte to file at offset from beginning of file
