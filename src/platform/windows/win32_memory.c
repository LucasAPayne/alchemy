#include "util/alchemy_memory.h"
#include "util/types.h"

#include <windows.h>

MemoryArena memory_arena_alloc(usize size)
{
    MemoryArena arena = {0};
    arena.used = 0;
    arena.memory = VirtualAllocEx(GetCurrentProcess(), NULL, size, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    if (arena.memory)
        arena.size = size;
    return arena;
}