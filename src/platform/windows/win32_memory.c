#include "util/alchemy_memory.h"
#include "util/types.h"

#include <windows.h>

GameMemory game_memory_init(usize permanent_storage_size, usize transient_storage_size)
{
    usize total_size = permanent_storage_size + transient_storage_size;
    void* base = VirtualAllocEx(GetCurrentProcess(), NULL, total_size, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);

    GameMemory result = {0};
    result.is_initialized = false;
    result.permanent_storage_size = permanent_storage_size;
    result.transient_storage_size = transient_storage_size;
    result.permanent_storage = base;
    result.transient_storage = (u8*)result.permanent_storage + result.permanent_storage_size;
    return result;
}

MemoryArena memory_arena_alloc(usize size)
{
    MemoryArena arena = {0};
    arena.used = 0;
    arena.memory = VirtualAllocEx(GetCurrentProcess(), NULL, size, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    if (arena.memory)
        arena.size = size;
    return arena;
}
