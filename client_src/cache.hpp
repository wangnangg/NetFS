#pragma once
#include <cassert>
#include <functional>
#include <list>
#include <unordered_map>
#include <vector>
#include "range.hpp"

class Cache
{
public:
    using BlockID = uint64_t;

private:
    enum State
    {
        Clean,
        Dirty
    };
    struct Entry
    {
        State state;
        std::vector<char> data;
        RangeList valid_ranges;
        Entry(State state, std::vector<char> data, RangeList ranges)
            : state(state),
              data(std::move(data)),
              valid_ranges(std::move(ranges))
        {
        }
        Entry(const Entry&) = delete;
        Entry& operator=(const Entry&) = delete;
    };

    // cache look up map
    std::unordered_map<BlockID, std::list<Entry>::iterator> _cache_map;

    // sorted by recent used date, hot entry is near head and cold is near
    // tail.
    std::list<Entry> _entry_list;
    size_t _block_size;

private:
    std::list<Entry>::iterator new_head_entry(const BlockID& bid, State state,
                                              std::vector<char> data,
                                              RangeList ranges)
    {
        std::list<Entry>::iterator itor = _entry_list.emplace(
            _entry_list.begin(), state, std::move(data), std::move(ranges));
        assert(_cache_map.find(bid) == _cache_map.end());
        _cache_map[bid] = itor;
        return itor;
    }

public:
    Cache(size_t block_size) : _block_size(block_size) {}

    // determine if a block is fully in cache
    bool blockInCache(const BlockID& bid)
    {
        if (_cache_map.find(bid) == _cache_map.end())
        {
            return false;
        }
        auto eptr = _cache_map.at(bid);
        if (eptr->valid_ranges.ranges().size() != 1)
        {
            return false;
        }
        auto rg = eptr->valid_ranges.ranges().front();
        if (rg.start == 0 && rg.end == _block_size)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    void putBlock(const BlockID& bid, std::vector<char> block)
    {
        assert(!blockInCache(bid));
        assert(block.size() == _block_size);
        if (_cache_map.find(bid) == _cache_map.end())
        {
            new_head_entry(bid, Clean, std::move(block),
                           RangeList(0, _block_size));
        }
        else
        {
            auto eptr = _cache_map.at(bid);
            overlay(&eptr->data[0], _block_size, eptr->valid_ranges,
                    &block[0]);
            std::swap(eptr->data, block);
            eptr->valid_ranges = RangeList(0, _block_size);
        }
    }

    /* get a block from cache, if the block is not cached, then nullptr is
     * returned.
     */
    const std::vector<char>& getBlock(const BlockID& bid)
    {
        assert(blockInCache(bid));
        return _cache_map.at(bid)->data;
    }

    /* write data in a block, then block doesn't need to be presented in the
     * cache. Written data will exist in ranges. If they form a full block,
     * then the block becomes available for read, otherwise, it cannot be
     * read.
     */
    void writeToBlock(const BlockID& bid, size_t offset, size_t size,
                      const char* buf)
    {
        assert(offset + size <= _block_size);
        if (_cache_map.find(bid) == _cache_map.end())
        {
            std::vector<char> data(_block_size);
            std::copy(buf, buf + size, &data[offset]);
            new_head_entry(bid, Dirty, std::move(data),
                           RangeList(offset, offset + size));
        }
        else
        {
            auto eptr = _cache_map.at(bid);
            std::copy(buf, buf + size, &eptr->data[offset]);
            eptr->valid_ranges.insertRange(offset, offset + size);
        }
    }
};
