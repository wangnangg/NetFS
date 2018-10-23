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

void Cache::fetchBlock(const std::string& filename, File& file,
                       size_t block_start, size_t block_count) const
{
    assert(block_count >= 1);
    // block range must be within file boundary
    assert(file.size > (block_start + block_count - 1) * _block_size);
    auto res = _remote->read(filename, block_start * _block_size,
                             _block_size * block_count);
}

std::pair<size_t, size_t> blockRange(size_t offset, size_t size,
                                     size_t block_size)
{
}

/* before reading a block, the block must be full fetched
 *
 */
void Cache::readFromBlock(File& file, size_t block_num, size_t offset,
                          size_t size, char* buf)
{
    const auto& block = file.blocks.at(block_num);
    assert(block.ranges.size() == 1);
    const auto& rg = block.ranges.front();
    assert(rg.start_offset == 0);
    assert(rg.data.size() >= size + offset);
    auto start = rg.data.begin() + offset;
    auto end = start + size;
    std::copy(start, end, buf);
}

size_t Cache::read(const std::string& filename, size_t offset, size_t size,
                   char* buf)
{
    assert(size > 0);
    ensureFile(filename);
    auto& file = _cache_map.at(filename);
    if (offset >= file.size)
    {
        return 0;
    }
    if (offset + size > file.size)
    {
        size = file.size - offset;
    }
    auto [block_start, block_count] = blockRange(offset, size, _block_size);
    fetchBlock(file, block_start, block_count);

    assert(block_count >= 1);
    size_t total_read = 0;
    if (block_count == 1)
    {
        size_t boff = blockOffset(offset, _block_size);
        readFromBlock(file, block_start, boff, size, buf);
        total_read += size;
    }
    else
    {  // block_count >= 2

        // first block
        size_t boff = blockOffset(offset, _block_size);
        size_t bsize = _block_size - boff;
        readFromBlock(file, block_start, boff, bsize, buf + total_read);
        total_read += bsize;

        // middle blocks
        for (size_t bn = block_start + 1; bn < block_start + block_count - 1;
             bn++)
        {
            readFromBlock(file, bn, 0, _block_size, buf + total_read);
            total_read += _block_size;
        }

        // final block
        bsize = blockOffset(offset + size, _block_size);
        readFromBlock(file, block_start + block_count - 1, 0, bsize,
                      buf + total_read);
        total_read += bsize;
    }
    assert(total_read == size);
    return size;
}
