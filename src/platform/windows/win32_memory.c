#include "alchemy/util/memory.h"
#include "alchemy/util/types.h"

#include <windows.h>

GameMemory game_memory_init(size permanent_storage_bytes, size transient_storage_bytes)
{
    GameMemory result = {0};
    result.is_initialized = false;

    result.total_bytes = permanent_storage_bytes + transient_storage_bytes;
    result.memory_block = VirtualAllocEx(GetCurrentProcess(), NULL, result.total_bytes, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    result.permanent_storage_bytes = permanent_storage_bytes;
    result.transient_storage_bytes = transient_storage_bytes;
    result.permanent_storage = result.memory_block;
    result.transient_storage = (u8*)result.permanent_storage + result.permanent_storage_bytes;
    return result;
}

MemoryArena memory_arena_alloc(size bytes)
{
    MemoryArena arena = {0};
    arena.used = 0;
    arena.memory = VirtualAllocEx(GetCurrentProcess(), NULL, bytes, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    if (arena.memory)
        arena.bytes = bytes;
    return arena;
}
