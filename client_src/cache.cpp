#include "cache.hpp"
#include <string.h>
#include <algorithm>
#include <cassert>
#include <iostream>

/* determine if the file cache is stale.
 */
bool Cache::isStale(const std::string& filename) const
{
    auto fc_itor = _file_map.find(filename);
    if (fc_itor == _file_map.end())
    {
        return false;
    }
    if (fc_itor->second.stale)
    {
        return true;
    }
    return false;
}

const FileTime* Cache::getFileTime(const std::string& filename) const
{
    auto fc_itor = _file_map.find(filename);
    if (fc_itor == _file_map.end())
    {
        return nullptr;
    }

    return &fc_itor->second.attr.time;
}

/* write to cache, possibly increase file size. The file must exist.  Affected
 * blocks become dirty.
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
    FileAttr& attr = fc.attr;
    if (offset + size > attr.size)
    {
        attr.size = offset + size;
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
    auto block_itor = fc.entries.find(block_num);
    if (block_itor == fc.entries.end())
    {
        CacheEntry& entry = newEntry(filename, block_num, _recent_list.end());
        entry.write(offset, buf, size);
    }
    else
    {
        CacheEntry& entry = block_itor->second;
        entry.write(offset, buf, size);
    }
}
size_t Cache::endBlock(size_t fsize) const
{
    if (fsize == 0)
    {
        return 0;
    }
    return blockNum(fsize - 1) + 1;
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
            block_bound = endBlock(fsize);
        }
        deleteEntryBeyond(filename, block_bound);
        file.attr.size = fsize;
        int err = _attr_wb(filename, file.attr, file.stale);
        if (err)
        {
            return err;
        }
    }
    else if (fsize > file.attr.size)
    {
        file.attr.size = fsize;
        int err = _attr_wb(filename, file.attr, file.stale);
        if (err)
        {
            return err;
        }
    }
    return 0;
}

/* create a new cache entry, along with its use record. The initial position
 * of the record is specified by `pos`.
 */
CacheEntry& Cache::newEntry(const std::string& filename, size_t block_num,
                            std::list<CacheEntryID>::iterator pos)
{
    FileCache& fc = _file_map.at(filename);
    assert(fc.entries.find(block_num) == fc.entries.end());
    auto rec_itor =
        _recent_list.insert(pos, CacheEntryID{filename, block_num});
    auto res =
        fc.entries.insert({block_num, CacheEntry(_block_size, rec_itor)});
    assert(res.second);
    return res.first->second;
}

/*delete entry, along with its usage record. */
void Cache::deleteEntry(const std::string& filename, size_t block_num)
{
    FileCache& file = _file_map.at(filename);
    auto entry_itor = file.entries.find(block_num);
    assert(entry_itor != file.entries.end());
    _recent_list.erase(entry_itor->second.useRecord());
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
            _recent_list.erase(it->second.useRecord());
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
        std::cerr << "error in read because of cacheFileAttr" << std::endl;
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
    size_t block_end = endBlock(offset + size);
    err = cacheBlocks(filename, block_start, block_end);
    if (err)
    {
        std::cerr << "error in read because of cacheBlocks" << std::endl;
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
    assert(isFullBlock(fc, block_num));
    std::copy(entry.data().begin() + offset,
              entry.data().begin() + offset + size, buf);
    moveToHead(entry.useRecord());
}

void Cache::moveToHead(std::list<CacheEntryID>::const_iterator rec_pos)
{
    _recent_list.splice(_recent_list.begin(), _recent_list, rec_pos);
}

size_t Cache::countDirtyBlocks() const
{
    size_t count = 0;
    for (const auto& fpair : _file_map)
    {
        for (const auto& bpair : fpair.second.entries)
        {
            if (bpair.second.state() == CacheEntry::Dirty)
            {
                count += 1;
            }
        }
    }
    return count;
}
int Cache::flushDirtyBlocks()
{
    std::unordered_map<std::string, std::vector<size_t>> fblocks;
    auto itor = _recent_list.rbegin();
    while (itor != _recent_list.rend())
    {
        if (_file_map.at(itor->filename)
                .entries.at(itor->block_num)
                .state() == CacheEntry::Dirty)
        {
            fblocks[itor->filename].push_back(itor->block_num);
        }
        itor++;
    }
    for (auto& pair : fblocks)
    {
        std::sort(pair.second.begin(), pair.second.end());
        int err = flushBlocks(pair.first, pair.second);
        if (err)
        {
            return err;
        }
    }
    return 0;
}

/* evict blocks from the tail of usage record. dirty blocks are written back.
 * clean blocks are dropped.
 */
int Cache::evictBlocks(size_t count)
{
    count = std::min(_recent_list.size(), count);
    int err = flushDirtyBlocks();
    if (err)
    {
        return err;
    }
    for (size_t i = 0; i < count; i++)
    {
        const auto& rec = _recent_list.back();
        deleteEntry(rec.filename, rec.block_num);
    }
    return 0;
}

/*write back all dirty blocks*/
int Cache::flush(const std::string& filename)
{
    auto fc_itor = _file_map.find(filename);
    if (fc_itor == _file_map.end())
    {
        return 0;
    }
    FileCache& fc = fc_itor->second;
    std::vector<size_t> dblocks;
    for (auto& pair : fc.entries)
    {
        if (pair.second.state() == CacheEntry::Dirty)
        {
            dblocks.push_back(pair.first);
        }
    }

    if (dblocks.size() > 0)
    {
        std::sort(dblocks.begin(), dblocks.end());
        flushBlocks(filename, dblocks);
    }
    return 0;
}

int Cache::flushBlocks(const std::string& filename,
                       const std::vector<size_t>& sorted_dblocks)
{
    FileCache& fc = _file_map.at(filename);
    std::vector<char> data;
    RangeList flush_range;
    for (size_t b : sorted_dblocks)
    {
        CacheEntry& entry = fc.entries.at(b);
        for (auto rg : entry.validRanges())
        {
            size_t abs_start = rg.start + b * _block_size;
            size_t abs_end = rg.end + b * _block_size;
            if (flush_range.count() == 1)
            {
                if (flush_range.begin()->end == abs_start)
                {
                    flush_range.insertRange(abs_start, abs_end);
                    data.insert(data.end(), entry.data().begin() + rg.start,
                                entry.data().begin() + rg.end);
                }
                else
                {
                    assert(flush_range.begin()->end -
                               flush_range.begin()->start ==
                           data.size());
                    int err = _content_wb(
                        filename, flush_range.begin()->start, &data[0],
                        data.size(), fc.attr.time, fc.stale);
                    if (err)
                    {
                        return err;
                    }
                    flush_range = RangeList();
                    data.resize(0);
                }
            }

            if (flush_range.count() == 0)
            {
                flush_range.insertRange(abs_start, abs_end);
                assert(data.size() == 0);
                data.insert(data.end(), entry.data().begin() + rg.start,
                            entry.data().begin() + rg.end);
            }
        }
    }
    assert(flush_range.count() <= 1);
    if (flush_range.count() == 1)
    {
        assert(flush_range.begin()->end - flush_range.begin()->start ==
               data.size());
        int err = _content_wb(filename, flush_range.begin()->start, &data[0],
                              data.size(), fc.attr.time, fc.stale);
        if (err)
        {
            return err;
        }
    }
    for (size_t b : sorted_dblocks)
    {
        CacheEntry& entry = fc.entries.at(b);
        entry.clean();
    }

    int err = _attr_wb(filename, fc.attr, fc.stale);
    if (err)
    {
        return err;
    }
    return 0;
}

void Cache::invalidate(const std::string& filename)
{
    if (_file_map.find(filename) == _file_map.end())
    {
        return;
    }
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

/* a block is considered full if valid_range is from 0 to
 * _block_size
 *
 */
bool Cache::isFullBlock(const FileCache& fc, size_t block_num) const
{
    auto itor = fc.entries.find(block_num);
    if (itor == fc.entries.end())
    {
        return false;
    }
    const CacheEntry& entry = itor->second;
    if (entry.validRanges().count() == 1 &&
        entry.validRanges().begin()->start == 0 &&
        entry.validRanges().begin()->end == _block_size)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/* make sure these blocks are now full in cache.
 * non-existent block will be fetched.
 *
 * partial block will be filled.
 *
 * full block will not be touched.
 *
 * if _content_ft fails, then no side-effects will happen.
 */
int Cache::cacheBlocks(const std::string& filename, size_t block_start,
                       size_t block_end)
{
#ifndef NDEBUG
    std::cout << "caching blocks(in): " << block_start << " - " << block_end
              << std::endl;
#endif
    assert(_file_map.find(filename) != _file_map.end());
    FileCache& fc = _file_map.at(filename);
    if (fc.attr.size == 0)
    {
        return 0;
    }
    RangeList block_range;
    /* find continuous blocks and try to fetch them in one message
     */
    for (size_t b = block_start; b < block_end; b++)
    {
        if (!isFullBlock(fc, b))
        {
            block_range.insertRange(b, b + 1);
        }
    }

    if (block_range.count() == 0)
    {
        _last_read_hit = true;
#ifndef NDEBUG
        std::cout << "caching blocks: hit!" << std::endl;
#endif
    }
    else
    {
        _last_read_hit = false;
#ifndef NDEBUG
        std::cout << "caching blocks: miss!" << std::endl;
        for (auto rg : block_range)
        {
            std::cout << rg << ", ";
        }
        std::cout << std::endl;
#endif
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
        for (curr_block = rg.start; curr_block < rg.end; curr_block++)
        {
            std::vector<char> block_data(_block_size, 0);
            size_t copy_start = (curr_block - rg.start) * _block_size;
            std::copy(buf.begin() + copy_start,
                      buf.begin() + copy_start + _block_size,
                      block_data.begin());
            auto block_itor = fc.entries.find(curr_block);
            if (block_itor == fc.entries.end())
            {
                CacheEntry& entry =
                    newEntry(filename, curr_block, _recent_list.end());
                entry.fetch(std::move(block_data));
            }
            else
            {
                CacheEntry& entry = block_itor->second;
                entry.fetch(std::move(block_data));
            }
        }
    }
    return 0;
}
