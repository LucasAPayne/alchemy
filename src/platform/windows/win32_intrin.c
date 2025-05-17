#include "alchemy/util/intrin.h"

BitScanResult find_least_significant_bit(u32 value)
{
    BitScanResult result = {0};
    result.found = _BitScanForward(&result.index, value);
    return result;
}
