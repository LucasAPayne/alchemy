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
    size total_bytes;
    void* permanent_storage; // MUST be cleared to 0 at startup
    size permanent_storage_bytes;
    void* transient_storage; // MUST be cleared to 0 at startup
    size transient_storage_bytes;
} GameMemory;

typedef struct MemoryArena
{
    size bytes;
    size used;
    u8* memory;
} MemoryArena;

GameMemory game_memory_init(size permanent_storage_size, size transient_storage_size);
MemoryArena memory_arena_alloc(size bytes);

// TODO(lucas): This should probably take an offset in to the base
inline MemoryArena memory_arena_init_from_base(void* base, size bytes)
{
    MemoryArena arena = {0};
    arena.bytes = bytes;
    arena.memory = (u8*)base;
    arena.used = 0;
    return arena;
}

inline void memory_arena_pop(MemoryArena* arena, size bytes)
{
    arena->used -= bytes;
}

inline void memory_arena_clear(MemoryArena* arena)
{
    arena->used = 0;
}

inline void* push_size_(MemoryArena* arena, size bytes)
{
    // First free part of the arena is the base plus whatever was already being used
    ASSERT((arena->used + bytes) <= arena->bytes);
    void* result = arena->memory + arena->used;

    // Add additional amount being used
    arena->used += bytes;
    return result;
}

inline void zero_size_(size bytes, void* ptr)
{
    // TODO(lucas): Check this guy for performance
    u8* byte = (u8*)ptr;
    while (bytes--)
        *byte++ = 0;
}

// Define macro to cast to correct type and get correct size
#define push_struct(arena, type) (type*)push_size_(arena, sizeof(type))
#define zero_struct(instance) zero_size_(sizeof((instance)), &(instance))
#define push_array(arena, count, type) (type*)push_size_(arena, (count)*sizeof(type))
#define zero_array(first, count, type) zero_size_((count)*sizeof(type), first)
#define push_size(arena, bytes) push_size_(arena, bytes)
