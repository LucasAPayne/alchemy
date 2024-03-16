#pragma once

#include "types.h"
#include "memory.h"

// TODO(lucas): Make variants that take an arena and allocate and return the string

inline int str_len(char* s)
{
    int count = 0;
    while(*s++)
        ++count;
    
    return count;
}

inline void str_cat(char* source_a, usize source_a_len, char* source_b, usize source_b_len,
                 char* dest, usize dest_len)
{
    // TODO(lucas): dest bounds checking
    // TODO(lucas): Check for null terminator before adding

    // For now, just loop through each source string and add each character one at a time to the dest string
    usize total_len = 0;
    for (usize i = 0; i < source_a_len; ++i)
        dest[total_len++] = *source_a++;

    for (usize i = 0; i < source_b_len; ++i)
        dest[total_len++] = *source_b++;

    dest[total_len++] = 0;
}

inline char* str_copy(char* src, MemoryArena* arena)
{
    usize len = str_len(src);
    char* result = push_array(arena, len+1, char);
    for (usize i = 0; i < len; ++i)
        result[i] = src[i];

    result[len] = '\0';
    return result;
}
<<<<<<< HEAD
=======

inline char* str_sub(char* src, usize pos, usize len, MemoryArena* arena)
{
    usize src_len = str_len(src);
    char* result = push_array(arena, len+1, char);
    
    if (pos >= src_len)
    {
        pos = len - 1;
        len = 0;
    }
    if (len > src_len)
        len = src_len;

    for (u32 i = 0; i < src_len; ++i, ++src)
        *(result + i) = *(src + pos);
    
    *(result + len) = '\0';
    return result;
}
>>>>>>> ebd83c9268a6a9fec3725ad1abd65f4521e57b33
