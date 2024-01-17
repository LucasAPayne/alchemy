#pragma once

#include "types.h"

#define KILOBYTES(value) ((u64)(value)*1024)
#define MEGABYTES(value) ((u64)KILOBYTES(value)*1024)
#define GIGABYTES(value) ((u64)MEGABYTES(value)*1024)
#define TERABYTES(value) ((u64)GIGABYTES(value)*1024)

typedef struct GameMemory
{
    b32 is_initialized;
    void* memory_block; // Pointer to the entire block of memory
    usize total_size;
    void* permanent_storage; // MUST be cleared to 0 at startup
    usize permanent_storage_size;
    void* transient_storage; // MUST be cleared to 0 at startup
    usize transient_storage_size;
} GameMemory;

typedef struct MemoryArena
{
    usize size;
    usize used;
    u8* memory;
} MemoryArena;

GameMemory game_memory_init(usize permanent_storage_size, usize transient_storage_size);
MemoryArena memory_arena_alloc(usize size);

inline MemoryArena memory_arena_init_from_base(void* base, usize size)
{
    MemoryArena arena = {0};
    arena.size = size;
    arena.memory = (u8*)base;
    arena.used = 0;
    return arena;
}

inline void memory_arena_clear(MemoryArena* arena)
{
    arena->used = 0;
}

inline void* push_size_(MemoryArena* arena, usize size)
{
    // First free part of the arena is the base plus whatever was already being used
    ASSERT((arena->used + size) <= arena->size);
    void* result = arena->memory + arena->used;

    // Add additional amount being used
    arena->used += size;
    return result;
}

inline void zero_size_(usize size, void* ptr)
{
    // TODO(lucas): Check this guy for performance
    u8* byte = (u8*)ptr;
    while (size--)
        *byte++ = 0;
}

// Define macro to cast to correct type and get correct size
#define push_struct(arena, type) (type*)push_size_(arena, sizeof(type))
#define push_array(arena, count, type) (type*)push_size_(arena, (count)*sizeof(type))
#define zero_struct(instance) zero_size_(sizeof((instance)), &(instance))
#define push_size(arena, size) push_size_(arena, size)
