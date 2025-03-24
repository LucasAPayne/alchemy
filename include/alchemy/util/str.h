#pragma once

#include "types.h"
#include "memory.h"

#define s8(s) (s8){(u8*)s, lengthof(s)}
typedef struct s8
{
    u8* data;
    size len;
} s8;

typedef struct s8_iter
{
    u8* at;
    size idx;
} s8_iter;

internal inline void s8_iter_move(s8_iter* it, size amount)
{
    it->at += amount;
    it->idx += amount;
}

internal inline s8 s8_alloc(MemoryArena* arena, size len)
{
    s8 result = {0};
    result.data = push_array(arena, len, u8);
    result.len = len;
    return result;
}

internal inline s8 s8_copyn(s8 src, size len, MemoryArena* arena)
{
    if (len > src.len)
        len = src.len;

    s8 result = s8_alloc(arena, len);
    for (size i = 0; i < len; ++i)
        result.data[i] = src.data[i];
    
    return result;
}

internal inline s8 s8_copy(s8 src, MemoryArena* arena)
{
    return s8_copyn(src, src.len, arena);
}

inline internal b32 s8_eq(s8 a, s8 b)
{
    if (a.len != b.len)
        return false;
    
    for (size i = 0; i < a.len; ++i)
    {
        if (a.data[i] != b.data[i])
            return false;
    }

    return true;
} 

internal inline void s8_cat(s8 source_a, s8 source_b, s8 dest)
{
    ASSERT(source_a.len + source_b.len <= dest.len);

    size idx = 0;
    for (size i = 0; i < source_a.len; ++i)
        dest.data[idx++] = *source_a.data++;

    for (size i = 0; i < source_b.len; ++i)
        dest.data[idx++] = *source_b.data++;
}

internal inline s8 s8_cat_arena(s8 a, s8 b, MemoryArena* arena)
{
    s8 result = s8_alloc(arena, a.len + b.len);
    s8_cat(a, b, result);
    return result;
}

internal inline s8 s8_substr(s8 src, size start, size len, MemoryArena* arena)
{
    s8 result = s8_alloc(arena, len);

    if (start >= src.len)
    {
        start = len - 1;
        len = 0;
    }
    if (len > src.len)
        len = src.len;

    for (size i = 0; i < len; ++i)
        result.data[i] = src.data[start+i];
    
    return result;
}

internal inline int utf8_get_num_bytes(u8 c)
{
    persist const char lengths[] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0
    };

    int num_bytes = lengths[c >> 3];
    return num_bytes;
}

internal inline u32 utf8_get_codepoint(u8* c)
{
    int num_bytes = utf8_get_num_bytes(*c);

    // Return ASCII characters directly.
    if (num_bytes == 1)
        return (u32)*c;

    // # of bytes: byte 1 prefix
    // 2: 110
    // 3: 1110
    // 4: 11110
    int codepoint = (*c & (0xff >> (num_bytes+1)));
    for (int i = 1; i < num_bytes; ++i)
    {
        ++c;

        // Each byte is prefixed with 10. Invalid otherwise; return replacement character.
        if ((*c & 0xc0) != 0x80)
            return 0xfffd;
        
        codepoint = (codepoint << 6) | (*c & 0x3f);
    }
    return codepoint;
}

internal inline int str_len(char* str)
{
    int len = 0;
    while (*str++)
        ++len;
    return len;
}

internal inline void str_cat(char* source_a, size source_a_len, char* source_b, size source_b_len,
                 char* dest, size dest_len)
{
    ASSERT(source_a_len + source_b_len <= dest_len);

    // TODO(lucas): Check for null terminator before adding
    // For now, just loop through each source string and add each character one at a time to the dest string
    size idx = 0;
    for (size i = 0; i < source_a_len; ++i)
        dest[idx++] = *source_a++;

    for (size i = 0; i < source_b_len; ++i)
        dest[idx++] = *source_b++;

    dest[idx++] = 0;
}

internal inline char* str_copy(char* src, MemoryArena* arena)
{
    usize len = str_len(src);
    char* result = push_array(arena, len+1, char);
    for (usize i = 0; i < len; ++i)
        result[i] = src[i];

    result[len] = '\0';
    return result;
}

internal inline char* str_sub(char* src, usize pos, usize len, MemoryArena* arena)
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
