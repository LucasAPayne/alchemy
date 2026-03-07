#pragma once

#include "types.h"
#include "memory.h"

#include <stdarg.h>
#include <string.h>

#include <assert.h>

#define s8(s) (s8){(u8*)s, lengthof(s)} // For use inside functions
#define S8(s) {(u8*)s, lengthof(s)} // For file or global scope
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

internal inline void s8_iter_move(s8_iter* it, size bytes)
{
    it->at += bytes;
    it->idx += bytes;
}

// Allocate a string but do not set its len
internal inline s8 s8_alloc(MemoryArena* arena, size cap)
{
    s8 result = {0};
    result.data = push_array(arena, cap, u8);
    return result;
}

// Allocate a string and set its len
internal inline s8 s8_init(MemoryArena* arena, size len)
{
    s8 result = s8_alloc(arena, len);
    result.len = len;
    return result;
}

internal inline char* s8_get_char(MemoryArena* arena, s8 src)
{
    char* dst = push_array(arena, src.len+1, char);
    memcpy(dst, src.data, src.len);
    dst[src.len] = 0;
    return dst;
}

// Reserves max_len bytes
internal inline s8 s8_format_max(MemoryArena* arena, size max_len, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    size len = (size)vsnprintf(NULL, 0, format, args);
    va_end(args);

    s8 result = s8_alloc(arena, max_len);
    result.len = len;

    va_start(args, format);
    vsnprintf((char*)result.data, len+1, format, args);
    va_end(args);

    return result;
}

internal inline s8 s8_format(MemoryArena* arena, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    size len = (size)vsnprintf(NULL, 0, format, args);
    va_end(args);

    s8 result = s8_init(arena, len);

    va_start(args, format);
    vsnprintf((char*)result.data, len+1, format, args);
    va_end(args);

    return result;
}

// Get a slice of a string from [begin, end)
internal inline s8 s8_slice(s8 s, size begin, size end)
{
    s8 result = {0};

    if (begin <= end && begin < s.len)
    {
        if (end > s.len)
            end = s.len;

        result.data = s.data + begin;
        result.len = end - begin;
    }

    return result;
}

internal inline s8 s8_copyn(s8 src, size len, MemoryArena* arena)
{
    if (len > src.len)
        len = src.len;

    s8 result = s8_init(arena, len);
    memcpy(result.data, src.data, len);

    // assert(((uintptr_t)result.data & 15) == 0);
    // assert(((uintptr_t)src.data & 15) == 0);

    return result;
}

internal inline s8 s8_copy(s8 src, MemoryArena* arena)
{
    return s8_copyn(src, src.len, arena);
}

internal inline b32 s8_eq(s8 a, s8 b)
{
    if (a.len != b.len)
        return false;

    return memcmp(a.data, b.data, a.len) == 0;
}

internal inline b32 s8_contains(s8 s, s8 sub)
{
    if (sub.len > s.len)
        return false;

    if (sub.len == 0)
        return true;

    for (size i = 0; i <= (s.len - sub.len); ++i)
    {
        if (s.data[i] == sub.data[0])
        {
            if (memcmp(s.data + i, sub.data, sub.len) == 0)
                return true;
        }
    }

    return false;
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
    s8 result = s8_init(arena, a.len + b.len);
    s8_cat(a, b, result);
    return result;
}

internal inline s8 s8_substr(s8 src, size start, size len, MemoryArena* arena)
{
    s8 result = s8_init(arena, len);

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

internal inline s8 s8_to_lower(s8 src, MemoryArena* arena)
{
    s8 result = s8_init(arena, src.len);

    u8* in = src.data;
    u8* out = result.data;
    size remaining = src.len;
    size idx = 0;

    while (remaining > 0)
    {
        int num_bytes = utf8_get_num_bytes(*in);

        if (num_bytes <= 0 || num_bytes > remaining)
        {
            out[idx] = in[idx];
            ++idx;
            --remaining;
            continue;
        }

        // ASCII
        if (num_bytes == 1)
        {
            u8 c = in[idx];
            if (c >= 'A' && c <= 'Z')
                out[idx] = (u8)(c + 32);
            else
                out[idx] = c;

            ++idx;
            --remaining;
        }
        else
        {
            // TODO(lucas): Handle other character sets
            for (int i = 0; i < num_bytes; ++i)
            {
                out[idx] = in[idx];
                ++idx;
            }
            remaining -= num_bytes;
        }
    }

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

internal inline char char_to_lower(char c)
{
    char result = c + 32;
    return result;
}

internal inline s8 s8_from_int(int x, MemoryArena* arena)
{
    size len = 0;
    int temp = x;
    do
    {
        temp /= 10;
        ++len;
    } while (temp != 0);

    s8 str = s8_init(arena, len);
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

internal inline int str_len(char* str)
{
    int len = 0;
    while (*str++)
        ++len;
    return len;
}

inline internal b32 str_eq(char* a, char* b)
{
    while(*a && (*a == *b))
    {
        ++a;
        ++b;
    }

    if (*a || *b)
        return false;

    return true;
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

internal inline char* str_find_last(char* s, char c)
{
    char* result = 0;

    // NOTE(lucas): This is a do...while for the case where c == '\0'.
    do
    {
        if (*s == c)
            result = s;
    } while (*s++);

    return result;
}
