#pragma once

#include "alchemy/util/types.h"

typedef struct BitScanResult
{
    b32 found;
    u32 index;
} BitScanResult;

BitScanResult find_least_significant_bit(u32 value);
