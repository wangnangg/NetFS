#pragma once
#include <functional>
#include <unordered_map>
#include <vector>

class RemoteInterface
{
public:
    virtual std::vector<char> read(const std::string& filename, size_t offset,
                                   size_t size) = 0;

    // if file doesn't exist, they must be created.
    virtual void write(const std::string& filename, size_t offset,
                       std::vector<char> data) = 0;
};

class Cache
{
    enum State
    {
        Clean,
        Dirty
    };
    struct Range
    {
        size_t start_offset;
        std::vector<char> data;
    };
    struct Entry
    {
        State state;
        std::vector<Range> ranges;
    };
    struct File
    {
        size_t size;
        std::unordered_map<size_t, Entry> blocks;
    };

    std::unordered_map<std::string, File> _cache_map;
    size_t _block_size;

    RemoteInterface* _remote;

public:
    Cache(size_t block_size, RemoteInterface* remote)
        : _block_size(block_size), _remote(remote)
    {
    }

    /* 1. determine the range of blocks
     * 2. ensure all blocks are in the cache
     * 3. copy data from blocks to buf.
     */
    size_t read(const std::string& filename, size_t offset, size_t size,
                char* buf);
    void write(const std::string& filename, size_t offset, size_t size,
               const char* buf);

private:
    /* multiple blocks are fetched from the server.
     * If blocks are not available (file may be removed or truncated by other
     * clients), zeros are filled.
     */
    void fetchBlock(File& file, size_t block_start, size_t block_count) const;

    void readFromBlock(File& file, size_t block_num, size_t offset,
                       size_t size, char* buf);
    void ensureFile(const std::string& filename)
    {
        if (_cache_map.find(filename) != _cache_map.end())
        {
            return;
        }
    }
};
