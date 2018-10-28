#pragma once
#include <cassert>
#include <functional>
#include <list>
#include <unordered_map>
#include <vector>
#include "range.hpp"

struct CacheEntryID
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
    std::list<CacheEntryID>::iterator use_record;
    CacheEntry(State state, std::vector<char> data, RangeList ranges,
               std::list<CacheEntryID>::iterator use_record)
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
    mode_t mode;
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
    FileCache(FileAttr attr) : state(Clean), attr(attr) {}
};

class Cache
{
public:
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
    std::list<CacheEntryID> _recent_list;
    size_t _block_size;
    WriteBackContentFunc _content_wb;
    WriteBackFileAttrFunc _attr_wb;
    FetchContentFunc _content_ft;
    FetchFileAttrFunc _attr_ft;

public:
    Cache(size_t block_size, WriteBackContentFunc content_wb,
          WriteBackFileAttrFunc attr_wb, FetchContentFunc content_ft,
          FetchFileAttrFunc attr_ft)
        : _block_size(block_size),
          _content_wb(content_wb),
          _attr_wb(attr_wb),
          _content_ft(content_ft),
          _attr_ft(attr_ft)
    {
    }

    int write(const std::string& filename, size_t offset, const char* buf,
              size_t size);

    int read(const std::string& filename, off_t offset, char* buf,
             size_t size, size_t& read_size);

    int truncate(const std::string& filename, size_t fsize);
    int stat(const std::string& filename, FileAttr& attr);

    int flush(const std::string& filename);

    void invalidate(const std::string& filename);

    size_t countCachedBlocks() const { return _recent_list.size(); }
    int evictBlocks(size_t count, size_t& eviced_count);

private:
    size_t blockNum(size_t offset) { return offset / _block_size; }

    size_t blockOffset(size_t offset) { return offset % _block_size; }

    int cacheFileAttr(const std::string& filename);
    int cacheBlocks(const std::string& filename, size_t block_start,
                    size_t block_end);

    void writeBlock(const std::string& filename, size_t block_num,
                    size_t offset, const char* buf, size_t size);

    void readBlock(const std::string& filename, size_t block_num,
                   size_t offset, char* buf, size_t size);

    void newEntry(const std::string& filename, size_t block_num,
                  std::vector<char> block, RangeList ranges,
                  CacheEntry::State state,
                  std::list<CacheEntryID>::iterator pos);
    void deleteEntry(const std::string& filename, size_t block_num);
    void deleteEntryBeyond(const std::string& filename, size_t block_bound);

    void moveToHead(std::list<CacheEntryID>::const_iterator rec_pos);
};
