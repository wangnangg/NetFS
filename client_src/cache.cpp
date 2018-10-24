#include "cache.hpp"
#include <cassert>

size_t blockNum(size_t offset, size_t block_size)
{
    return offset / block_size;
}
size_t blockOffset(size_t offset, size_t block_size)
{
    return offset % block_size;
}
