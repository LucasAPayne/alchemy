#pragma once

#include "types.h"

#define KILOBYTES(value) ((u64)(value)*1024)
#define MEGABYTES(value) ((u64)KILOBYTES(value)*1024)
#define GIGABYTES(value) ((u64)MEGABYTES(value)*1024)
#define TERABYTES(value) ((u64)GIGABYTES(value)*1024)

typedef struct MemoryArena
{
    usize size;
    usize used;
    u8* memory;
} MemoryArena;

MemoryArena memory_arena_alloc(usize size);

inline void memory_arena_clear(MemoryArena* arena)
{
    arena->used = 0;
}

inline void* push_size(MemoryArena* arena, usize size)
{
    // First free part of the arena is the base plus whatever was already being used
    ASSERT((arena->used + size) <= arena->size);
    void* result = arena->memory + arena->used;

    // Add additional amount being used
    arena->used += size;
    return result;
}

inline void zero_size(usize size, void* ptr)
{
    // TODO(lucas): Check this guy for performance
    u8* byte = (u8*)ptr;
    while (size--)
        *byte++ = 0;
}

// Define macro to cast to correct type and get correct size
#define push_struct(arena, type) (type*)push_size(arena, sizeof(type))
#define push_array(arena, count, type) (type*)push_size(arena, (count)*sizeof(type))
#define zero_struct(instance) zero_size(sizeof((instance)), &(instance))
