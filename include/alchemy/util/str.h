#pragma once

#include "types.h"

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

