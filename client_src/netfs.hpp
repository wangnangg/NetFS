#pragma once
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include "cache.hpp"
#include "msg.hpp"
#include "serial.hpp"
#include "stream.hpp"

/* all file operations return errno
 * all methods have no throw guarantee
 */
class NetFS
{
    int msg_id;
    FdWriter writer;
    FdReader reader;
    size_t block_size;
    size_t max_cache_entry;
    size_t evict_count;
    size_t flush_interval;
    size_t write_op_count;
    Cache cache;

public:
    NetFS(const std::string& hostname, const std::string& port,
          size_t block_size, size_t max_cache_entry, size_t evict_count,
          size_t flush_interval);

    int access(const std::string& filename);
    int create(const std::string& filename);
    int stat(const std::string& filename, struct stat& statbuf);
    int statfs(struct statvfs& statbuf);

    int readdir(const std::string& filename, std::vector<std::string>& dirs);

    int read(const std::string& filename, off_t offset, size_t size,
             char* buf, size_t& total_read);

    int write(const std::string& filename, off_t offset, const char* buf,
              size_t size);

    int truncate(const std::string& filename, off_t offset);

    int unlink(const std::string& filename);
    int rmdir(const std::string& filename);

    int mkdir(const std::string& filename, mode_t mode);

    int flush(const std::string& filename);

    int rename(const std::string& from, const std::string& to,
               unsigned int flags);

private:
    int fetchBlocks(const std::string& filename, uint32_t block_start,
                    uint32_t block_end, uint32_t& block_count);

    int cacheBlocks(const std::string& filename, uint32_t block_start,
                    uint32_t block_end);
    int cacheFile(const std::string& filename);

    int do_write(const std::string& filename, off_t offset, const char* buf,
                 size_t size, FileTime& cached_time, bool& stale);
    int do_read(const std::string& filename, off_t offset, char* buf,
                size_t size, size_t& read_size);
    int do_read_attr(const std::string& filename, FileAttr& attr);
    int do_write_attr(const std::string& filename, FileAttr& attr,
                      bool& stale);

    void sendMsg(const Msg& msg);
    std::unique_ptr<Msg> recvMsg();
    uint32_t blockNum(off_t offset);
    size_t blockOffset(off_t offset);
    int evict();
};
