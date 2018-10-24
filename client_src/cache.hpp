#pragma once
#include <cassert>
#include <functional>
#include <list>
#include <unordered_map>
#include <vector>
#include "range.hpp"

struct RecentUsedRecord
{
    std::string filename;
    size_t block_num;
};

struct CacheEntry
{
    enum State
    {
        Clean,
        Dirty
    };
    State state;
    std::vector<char> data;
    RangeList valid_ranges;
    std::list<RecentUsedRecord>::iterator use_record;
    CacheEntry(State state, std::vector<char> data, RangeList ranges,
               std::list<RecentUsedRecord>::iterator use_record)
        : state(state),
          data(std::move(data)),
          valid_ranges(std::move(ranges)),
          use_record(use_record)
    {
    }
};

struct FileAttr
{
    size_t size;
};

struct FileCache
{
    enum State
    {
        Clean,
        Dirty
    };
    State state;
    FileAttr attr;
    std::unordered_map<size_t, CacheEntry> entries;
    FileCache(State state, FileAttr attr) : state(state), attr(attr) {}
};

class Cache
{
public:
    using BlockID = uint64_t;
    using WriteBackContentFunc =
        std::function<int(const std::string& fname, size_t offset,
                          const char* data, size_t size)>;
    using WriteBackFileAttrFunc =
        std::function<int(const std::string& fname, FileAttr attr)>;
    using FetchContentFunc =
        std::function<int(const std::string& filename, size_t offset,
                          char* data, size_t size, size_t& read_size)>;
    using FetchFileAttrFunc =
        std::function<int(const std::string& filename, FileAttr& attr)>;

private:
    // cache look up map
    std::unordered_map<std::string, FileCache> _file_map;

    // sorted by recent used date, hot entry is near head and cold is near
    // tail.
    std::list<RecentUsedRecord> _recent_list;
    size_t _block_size;
    WriteBackContentFunc content_wb;
    WriteBackFileAttrFunc attr_wb;
    FetchContentFunc content_ft;
    FetchFileAttrFunc attr_ft;

public:
    Cache(size_t block_size, WriteBackContentFunc content_wb,
          WriteBackFileAttrFunc attr_wb, FetchContentFunc content_ft,
          FetchFileAttrFunc attr_ft)
        : _block_size(block_size),
          content_wb(content_wb),
          attr_wb(attr_wb),
          content_ft(content_ft),
          attr_ft(attr_ft)
    {
    }

    int write(const std::string& filename, size_t offset, const char* buf,
              size_t size);

    int read(const std::string& filename, off_t offset, char* buf,
             size_t size, size_t& read_size);

    int writeFileAttr(const std::string& filename, FileAttr attr);
    int readFileAttr(const std::string& filename, FileAttr& attr);

    void invalidateFile(const std::string& filename);

    size_t countCachedBlocks() const;
    int evictBlocks(size_t count, size_t& eviced_count) const;
    int evictFile(const std::string& filename) const;

private:
    size_t blockNum(size_t offset) { return offset / _block_size; }

    size_t blockOffset(size_t offset) { return offset % _block_size; }

    int cacheFileAttr(const std::string& filename);

    void writeToBlock(const std::string& filename, size_t block_num,
                      size_t offset, const char* buf, size_t size);
    void newCacheEntry(const std::string& filename, size_t block_num,
                       size_t offset, std::vector<char> block,
                       CacheEntry::State state,
                       std::list<RecentUsedRecord>::iterator pos);
};
