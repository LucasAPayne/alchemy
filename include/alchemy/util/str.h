#pragma once

#include "types.h"
#include "memory.h"

#include <stdarg.h>

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

internal inline s8 s8_format(MemoryArena* arena, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    size len = (size)vsnprintf(NULL, 0, format, args);
    va_end(args);

    s8 result = s8_alloc(arena, len);

    va_start(args, format);
    vsnprintf((char*)result.data, len+1, format, args);
    va_end(args);

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
    ASSERT(source_a.len + source_b.len <= dest.len, "s8 overflow");

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

internal inline b32 char_is_whitespace(char c)
{
    b32 result = (c == ' ' ) || (c == '\t') || (c == '\v') || (c == '\f');
    return result;
}

internal inline b32 char_is_digit(char c)
{
    b32 result = (c >= '0' && c <= '9');
    return result;
}

internal inline s8 s8_from_int(int x, MemoryArena* arena)
{
    size len = 0;
    int temp = x;
    while (temp != 0)
    {
        temp /= 10;
        ++len;
    }

    s8 str = s8_alloc(arena, len);
    for (size i = len-1; i >= 0; --i)
    {
        str.data[i] = (x % 10) + '0';
        x /= 10;
    }

    return str;
}

// TODO(lucas): Handle int overflow
internal inline int s8_to_int(s8 s)
{
    int result = 0;
    b32 neg = false;
    size i = 0;

    while (i < s.len && char_is_whitespace(s.data[i]))
        ++i;
    
    if (i >= s.len)
        return result;

    if (s.data[i] == '-' || s.data[i] == '+')
    {
        neg = (s.data[i] == '-');
        ++i;
    }

    while (i < s.len && char_is_digit(s.data[i]))
    {
        result = result*10 + (s.data[i] - '0');
        ++i;
    }

    if (neg)
        result = -result;
    
    return result;
}

// Given the first byte of a UTF-8 encoded character, returns the total number of bytes for that character
internal inline int utf8_get_num_bytes(u8 c)
{
    persist const char lengths[] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0
    };

    int num_bytes = lengths[c >> 3];
    return num_bytes;
}

// Given a UTF-8 enc
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

// Given the codepoint for a UTF-8 character, writes up to 4 bytes into c
internal inline void utf8_from_codepoint(u8* c, u32 codepoint)
{
    if (codepoint < 0x7f)
        c[0] = (u8)codepoint;
    else if (codepoint < 0x7ff)
    {
        c[0] = (u8)(0xc0 | (codepoint >> 6));
        c[1] = (u8)(0x80 | (codepoint & 0x3f));
    }
    else if (codepoint < 0xffff)
    {
        c[0] = (u8)(0xe0 | (codepoint >> 12));
        c[1] = (u8)(0x80 | ((codepoint >> 6) & 0x3f));
        c[2] = (u8)(0x80 | (codepoint & 0x3f));
    }
    else if (codepoint <= 0x10ffff)
    {
        c[0] = (u8)(0xf0 | (codepoint >> 18));
        c[1] = (u8)(0x80 | ((codepoint >> 12) & 0x3f));
        c[2] = (u8)(0x80 | ((codepoint >> 6) & 0x3f));
        c[3] = (u8)(0x80 | (codepoint & 0x3f));
    }
    else
    {
        // TODO(lucas): Add error handling for invalid codepoint
    }
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
    ASSERT(source_a_len + source_b_len <= dest_len, "string overflow");

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
