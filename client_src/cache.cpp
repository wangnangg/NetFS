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

int Cache::write(const std::string& filename, size_t offset, const char* buf,
                 size_t size)
{
    if (_file_map.find(filename) == _file_map.end())
    {
        int err = cacheFileAttr(filename);
        if (err)
        {
            return err;
        }
    }
    FileCache& fc = _file_map.at(filename);
    FileAttr attr = fc.attr;
    if (offset + size > attr.size)
    {
        attr.size = offset + size;
        int err = writeFileAttr(filename, attr);
        if (err)
        {
            return err;
        }
    }
    size_t curr_block = blockNum(offset);
    size_t bstart = blockOffset(offset);
    size_t bsize = std::min(_block_size - bstart, size);
    size_t written_size = 0;
    while (written_size < size)
    {
        writeToBlock(filename, curr_block, bstart, buf + written_size, bsize);
        curr_block += 1;
        written_size += bsize;
        bstart = 0;
        bsize = std::min(_block_size, size - written_size);
    }
    return 0;
}

void Cache::writeToBlock(const std::string& filename, size_t block_num,
                         size_t offset, const char* buf, size_t size)
{
    assert(offset + size <= _block_size);
    FileCache& fc = _file_map.at(filename);
    if (fc.entries.find(block_num) == fc.entries.end())
    {
        std::vector<char> block(_block_size, 0);
        std::copy(buf, buf + size, &block[offset]);
        newCacheEntry(filename, block_num, offset, std::move(block),
                      CacheEntry::Dirty, _recent_list.begin());
    }
    else
    {
        CacheEntry& entry = fc.entries.at(block_num);
        std::copy(buf, buf + size, &entry.data[offset]);
        entry.state = CacheEntry::Dirty;
    }
}

int Cache::writeFileAttr(const std::string& filename, FileAttr attr)
{
    _file_map[filename] = FileCache(FileCache::Dirty, attr);
    return 0;
}

int Cache::readFileAttr(const std::string& filename, FileAttr& attr)
{
    int err = cacheFileAttr(filename);
    if (err)
    {
        return err;
    }
    attr = _file_map.at(filename).attr;
    return 0;
}

void Cache::newCacheEntry(const std::string& filename, size_t block_num,
                          size_t offset, std::vector<char> block,
                          CacheEntry::State state,
                          std::list<RecentUsedRecord>::iterator pos)
{
    FileCache& fc = _file_map.at(filename);
    auto rec_itor =
        _recent_list.insert(pos, RecentUsedRecord{filename, block_num});
    fc.entries[block_num] =
        CacheEntry(state, std::move(block),
                   RangeList(offset, offset + block.size()), rec_itor);
}
