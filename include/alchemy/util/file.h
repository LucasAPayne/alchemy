#pragma once

#include "alchemy/util/memory.h"
#include "alchemy/util/str.h"
#include "alchemy/util/types.h"

typedef enum FileMode
{
    FileMode_Read = (1 << 0),
    FileMode_Write = (1 << 1)
} FileMode;

b32 file_exists(char* filename);
size file_get_size(char* filename); // Returns size in bytes of file
void* file_open(char* filename, FileMode mode); // Opens file with given mode(s) and returns file handle
void file_close(void* file_handle);

u64 file_get_last_write_time(char* filename);
b32 file_is_modified(char* filename, u64 reference_time);

s8 file_to_string(char* filename, MemoryArena* arena);

void file_write_byte(void* file_handle, size offset, u8 byte); // Writes byte to file at offset from beginning of file
