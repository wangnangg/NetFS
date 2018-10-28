#include "cache.hpp"
#include <cassert>

/* write to cache, possibly increase file size. Affected blocks become dirty.
 * Incomplete entries may be created at head or tail.
 *
 */
int Cache::write(const std::string& filename, size_t offset, const char* buf,
                 size_t size)
{
    int err = cacheFileAttr(filename);
    if (err)
    {
        return err;
    }
    FileCache& fc = _file_map.at(filename);
    const FileAttr& attr = fc.attr;
    if (offset + size > attr.size)
    {
        int err = truncate(filename, offset + size);
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
        writeBlock(filename, curr_block, bstart, buf + written_size, bsize);
        curr_block += 1;
        written_size += bsize;
        bstart = 0;
        bsize = std::min(_block_size, size - written_size);
    }
    return 0;
}

/* write buf to block `block_num`. If the block exists in cache, the content
 * is updated, otherwise a new entry for this block is created. In both cases,
 * the block is marked dirty. If a new entry is created, the content may not
 * span the whole block, therefore, the entry is not complete. Read incomplete
 * block will trigger fetching.
 *
 */
void Cache::writeBlock(const std::string& filename, size_t block_num,
                       size_t offset, const char* buf, size_t size)
{
    assert(offset + size <= _block_size);
    FileCache& fc = _file_map.at(filename);
    if (fc.entries.find(block_num) == fc.entries.end())
    {
        std::vector<char> block(_block_size, 0);
        std::copy(buf, buf + size, &block[offset]);
        newEntry(filename, block_num, std::move(block),
                 RangeList(offset, offset + size), CacheEntry::Dirty,
                 _recent_list.end());
    }
    else
    {
        CacheEntry& entry = fc.entries.at(block_num);
        std::copy(buf, buf + size, &entry.data[offset]);
        entry.state = CacheEntry::Dirty;
    }
}

/* change file size. If size is reduced, blocks outside file boundary are
 * dropped.
 */
int Cache::truncate(const std::string& filename, size_t fsize)
{
    int err = cacheFileAttr(filename);
    if (err)
    {
        return err;
    }
    FileCache& file = _file_map.at(filename);
    if (fsize < file.attr.size)
    {
        size_t block_bound;
        if (fsize == 0)
        {
            block_bound = 0;
        }
        else
        {
            block_bound = blockNum(fsize - 1) + 1;
        }
        deleteEntryBeyond(filename, block_bound);
        file.attr.size = fsize;
        file.state = FileCache::Dirty;
    }
    return 0;
}

int Cache::stat(const std::string& filename, FileAttr& attr)
{
    int err = cacheFileAttr(filename);
    if (err)
    {
        return err;
    }
    attr = _file_map.at(filename).attr;
    return 0;
}

/* create a new cache entry, along with its use record. The initial position
 * of the record is specified by `pos`.
 */
void Cache::newEntry(const std::string& filename, size_t block_num,
                     std::vector<char> block, RangeList ranges,
                     CacheEntry::State state,
                     std::list<CacheEntryID>::iterator pos)
{
    assert(block.size() == _block_size);
    FileCache& fc = _file_map.at(filename);
    assert(fc.entries.find(block_num) == fc.entries.end());
    auto rec_itor =
        _recent_list.insert(pos, CacheEntryID{filename, block_num});
    auto res = fc.entries.insert(
        {block_num,
         CacheEntry(state, std::move(block), std::move(ranges), rec_itor)});
    assert(res.second);
}

/*delete entry, along with its usage record. */
void Cache::deleteEntry(const std::string& filename, size_t block_num)
{
    FileCache& file = _file_map.at(filename);
    auto entry_itor = file.entries.find(block_num);
    assert(entry_itor != file.entries.end());
    _recent_list.erase(entry_itor->second.use_record);
    file.entries.erase(entry_itor);
}

/*delete entries whose block_num >= block_bound, along with their usage record
 */
void Cache::deleteEntryBeyond(const std::string& filename, size_t block_bound)
{
    assert(_file_map.find(filename) != _file_map.end());
    FileCache& file = _file_map.at(filename);
    for (auto it = file.entries.begin(); it != file.entries.end();)
    {
        if (it->first >= block_bound)
        {
            _recent_list.erase(it->second.use_record);
            it = file.entries.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

/* read into `buf`. If required blocks are not in cache or incomplete, then
 * they are fetched from the server. In case of an incomplete block, the
 * original data overwrites the corresponding part of the fetched data. Usage
 * records are moved to head. Read ends at EOF, actual read size is in
 * `read_size`
 */
int Cache::read(const std::string& filename, off_t offset, char* buf,
                size_t size, size_t& read_size)
{
    int err = cacheFileAttr(filename);
    if (err)
    {
        return err;
    }
    FileCache& fc = _file_map.at(filename);
    assert(offset >= 0);
    if ((size_t)offset >= fc.attr.size || size == 0)
    {
        read_size = 0;
        return 0;
    }
    if (offset + size > fc.attr.size)
    {
        size = fc.attr.size - offset;
    }

    size_t block_start = blockNum(offset);
    size_t block_end = blockNum(offset + size - 1) + 1;
    err = cacheBlocks(filename, block_start, block_end);
    if (err)
    {
        return err;
    }
    size_t curr_block = block_start;
    size_t bstart = blockOffset(offset);
    size_t bsize = std::min(_block_size - bstart, size);
    read_size = 0;
    while (read_size < size)
    {
        readBlock(filename, curr_block, bstart, buf + read_size, bsize);
        curr_block += 1;
        read_size += bsize;
        bstart = 0;
        bsize = std::min(_block_size, size - read_size);
    }
    return 0;
}

/* read content from a block, the block will be moved to the head of usage
 * record.
 */
void Cache::readBlock(const std::string& filename, size_t block_num,
                      size_t offset, char* buf, size_t size)
{
    assert(_file_map.find(filename) != _file_map.end());
    assert(offset + size <= _block_size);
    const FileCache& fc = _file_map.at(filename);
    const CacheEntry& entry = fc.entries.at(block_num);
    std::copy(entry.data.begin() + offset, entry.data.begin() + offset + size,
              buf);
    moveToHead(entry.use_record);
}

void Cache::moveToHead(std::list<CacheEntryID>::const_iterator rec_pos)
{
    _recent_list.splice(_recent_list.begin(), _recent_list, rec_pos);
}

/* evict blocks from the tail of usage record. dirty blocks are written back.
 * clean blocks are dropped.
 */
int Cache::evictBlocks(size_t count, size_t& eviced_count)
{
    eviced_count = 0;
    count = std::min(_recent_list.size(), count);
    while (eviced_count < count)
    {
        const auto& rec = _recent_list.back();
        FileCache& file = _file_map.at(rec.filename);
        auto entry_itor = file.entries.find(rec.block_num);
        assert(entry_itor != file.entries.end());
        if (entry_itor->second.state == CacheEntry::Dirty)
        {
            int err = _content_wb(rec.filename, rec.block_num * _block_size,
                                  &entry_itor->second.data[0], _block_size);
            if (err)
            {
                return err;
            }
        }
        deleteEntry(rec.filename, rec.block_num);
        eviced_count += 1;
    }
    return 0;
}

/*write back all dirty blocks*/
int Cache::flush(const std::string& filename)
{
    FileCache& fc = _file_map.at(filename);
    if (fc.state == FileCache::Dirty)
    {
        int err = _attr_wb(filename, fc.attr);
        if (err)
        {
            return err;
        }
    }
    for (auto& pair : _file_map.at(filename).entries)
    {
        if (pair.second.state == CacheEntry::Dirty)
        {
            int err = _content_wb(filename, pair.first * _block_size,
                                  &pair.second.data[0], _block_size);
            if (err)
            {
                return err;
            }
            pair.second.state = CacheEntry::Clean;
        }
    }
    return 0;
}

void Cache::invalidate(const std::string& filename)
{
    deleteEntryBeyond(filename, 0);
    _file_map.erase(filename);
}

int Cache::cacheFileAttr(const std::string& filename)
{
    if (_file_map.find(filename) != _file_map.end())
    {
        return 0;
    }
    FileAttr attr;
    int err = _attr_ft(filename, attr);
    if (err)
    {
        return err;
    }
    _file_map.insert({filename, FileCache{attr}});
    return 0;
}

/*non-existent block will be filled with zero*/
int Cache::cacheBlocks(const std::string& filename, size_t block_start,
                       size_t block_end)
{
    FileCache& fc = _file_map.at(filename);
    RangeList block_range;
    for (size_t b = block_start; b < block_end; b++)
    {
        if (fc.entries.find(b) == fc.entries.end())
        {
            block_range.insertRange(b, b + 1);
        }
    }
    for (auto rg : block_range)
    {
        size_t block_count = rg.end - rg.start;
        std::vector<char> buf(block_count * _block_size);
        size_t read_size;
        int err = _content_ft(filename, rg.start * _block_size, &buf[0],
                              buf.size(), read_size);
        if (err)
        {
            return err;
        }

        size_t curr_block;
        size_t copy_size = 0;
        for (curr_block = block_start; curr_block < block_end; curr_block++)
        {
            std::vector<char> block_data(_block_size, 0);
            size_t copy_end = std::min(copy_size + _block_size, buf.size());
            std::copy(buf.begin() + copy_size, buf.begin() + copy_end,
                      block_data.begin());
            newEntry(filename, curr_block, std::move(block_data),
                     RangeList(0, copy_end - copy_size), CacheEntry::Clean,
                     _recent_list.end());
            copy_size = copy_end;
        }
    }
    return 0;
}
