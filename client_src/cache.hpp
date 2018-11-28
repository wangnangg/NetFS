#pragma once
#include <cassert>
#include <functional>
#include <list>
#include <unordered_map>
#include <vector>
#include "msg.hpp"
#include "range.hpp"

struct CacheEntryID
{
    std::string filename;
    size_t block_num;
};

class CacheEntry
{
public:
    enum State
    {
        Clean,
        Dirty
    };

private:
    State _state;
    std::vector<char> _data;
    RangeList _valid_ranges;
    std::list<CacheEntryID>::iterator _use_record;

public:
    CacheEntry(size_t block_size,
               std::list<CacheEntryID>::iterator use_record)
        : _state(Clean),
          _data(block_size, 0),
          _valid_ranges(),
          _use_record(use_record)
    {
    }
    size_t blockSize() const { return _data.size(); }
    State state() const { return _state; }
    void clean() { _state = Clean; }
    const std::vector<char>& data() const { return _data; }
    std::list<CacheEntryID>::iterator useRecord() const
    {
        return _use_record;
    }

    const RangeList& validRanges() const { return _valid_ranges; }

    void fetch(std::vector<char> fetch_data)
    {
        assert(fetch_data.size() == blockSize());
        overlay(&_data[0], validRanges(), &fetch_data[0]);
        std::swap(_data, fetch_data);
        _valid_ranges.insertRange(0, blockSize());
    }

    void write(size_t offset, const char* buf, size_t size)
    {
        assert(offset + size <= blockSize());
        std::copy(buf, buf + size, &_data[offset]);
        _valid_ranges.insertRange(offset, offset + size);
        _state = Dirty;
    }
};

struct FileAttr
{
    size_t size;
    mode_t mode;
    FileTime time;
};

struct FileCache
{
    bool stale;
    FileAttr attr;
    std::unordered_map<size_t, CacheEntry> entries;
    FileCache() : stale(false), attr() {}
    FileCache(FileAttr attr) : stale(false), attr(attr) {}
};

class Cache
{
public:
    using WriteBackContentFunc = std::function<int(
        const std::string& fname, size_t offset, const char* data,
        size_t size, FileTime& time, bool& stale)>;
    using WriteBackFileAttrFunc = std::function<int(
        const std::string& fname, FileAttr& attr, bool& stale)>;
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

    bool isStale(const std::string& filename) const;
    const FileTime* getFileTime(const std::string& filename) const;

    int write(const std::string& filename, size_t offset, const char* buf,
              size_t size);

    int read(const std::string& filename, off_t offset, char* buf,
             size_t size, size_t& read_size);

    int truncate(const std::string& filename, size_t fsize);

    int flush(const std::string& filename);

    void invalidate(const std::string& filename);

    size_t countCachedBlocks() const { return _recent_list.size(); }
    size_t countDirtyBlocks() const;
    int evictBlocks(size_t count);
    int flushDirtyBlocks();

    bool isLastReadHit() const { return _last_read_hit; }

private:
    size_t blockNum(size_t offset) const { return offset / _block_size; }

    size_t blockOffset(size_t offset) const { return offset % _block_size; }

    int cacheFileAttr(const std::string& filename);
    int cacheBlocks(const std::string& filename, size_t block_start,
                    size_t block_end);

    void writeBlock(const std::string& filename, size_t block_num,
                    size_t offset, const char* buf, size_t size);

    void readBlock(const std::string& filename, size_t block_num,
                   size_t offset, char* buf, size_t size);

    bool _last_read_hit = false;

    CacheEntry& newEntry(const std::string& filename, size_t block_num,
                         std::list<CacheEntryID>::iterator pos);
    void deleteEntry(const std::string& filename, size_t block_num);
    void deleteEntryBeyond(const std::string& filename, size_t block_bound);

    void moveToHead(std::list<CacheEntryID>::const_iterator rec_pos);

    size_t endBlock(size_t fsize) const;
    bool isFullBlock(const FileCache& fc, size_t block_num) const;

    int flushBlocks(const std::string& filename,
                    const std::vector<size_t>& sorted_dblocks);
};
