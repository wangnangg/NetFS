#include "netfs.hpp"
#include <netdb.h>
#include <netinet/in.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/statvfs.h>
#include <cassert>
#include <cstring>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <system_error>

int connect(const std::string& hostname, const std::string& port)
{
    int socket_fd;
    struct addrinfo host_info;
    struct addrinfo* host_info_list;
    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family = AF_INET;
    host_info.ai_socktype = SOCK_STREAM;

    int s = getaddrinfo(hostname.c_str(), port.c_str(), &host_info,
                        &host_info_list);
    if (s != 0)
    {
        throw std::runtime_error("getaddrinfo: " +
                                 std::string(gai_strerror(s)));
    }

    socket_fd = socket(host_info_list->ai_family, host_info_list->ai_socktype,
                       host_info_list->ai_protocol);
    if (socket_fd == -1)
    {
        freeaddrinfo(host_info_list);
        throw std::system_error(errno, std::system_category());
    }
    if (connect(socket_fd, host_info_list->ai_addr,
                host_info_list->ai_addrlen) < 0)
    {
        freeaddrinfo(host_info_list);
        throw std::system_error(errno, std::system_category());
    }
    freeaddrinfo(host_info_list);
    return socket_fd;
}
using namespace std::placeholders;
NetFS::NetFS(const std::string& hostname, const std::string& port,
             size_t block_size, size_t max_cache_entry, size_t evict_count,
             size_t flush_interval)
    : msg_id(0),
      writer(),
      reader(),
      block_size(block_size),
      max_cache_entry(max_cache_entry),
      evict_count(evict_count),
      flush_interval(flush_interval),
      write_op_count(0),
      cache(block_size,
            std::bind(&NetFS::do_write, this, _1, _2, _3, _4, _5, _6),
            std::bind(&NetFS::do_write_attr, this, _1, _2, _3),
            std::bind(&NetFS::do_read, this, _1, _2, _3, _4, _5),
            std::bind(&NetFS::do_read_attr, this, _1, _2)

      )
{
    int wt = connect(hostname, port);
    writer = FdWriter(wt);
    int rd = dup(wt);
    if (rd < 0)
    {
        throw std::system_error(errno, std::system_category());
    }
    reader = FdReader(rd);
}

int NetFS::access(const std::string& filename)
{
    MsgAccess msg(msg_id++, filename);
    sendMsg(msg);
    auto resp = recvMsg();
    auto ptr = dynamic_cast<MsgAccessResp*>(resp.get());
    assert(ptr);
    assert(ptr->id == msg.id);

    if (ptr->error != 0)
    {
        std::cout << "invalidate due to file not exist: " << filename
                  << std::endl;
        cache.invalidate(filename);
    }
    else
    {
        if (cache.isStale(filename))
        {
            std::cout << "invalidate due to past stale: " << filename
                      << std::endl;
            cache.invalidate(filename);
        }
        else
        {
            const FileTime* cached_time = cache.getFileTime(filename);
            if (cached_time != nullptr && *cached_time != ptr->time)
            {
                std::cout << "invalidate due to stale cache ";
                std::cout << "(time miss match, cached: "
                          << cached_time->mtime
                          << ", remote: " << ptr->time.mtime
                          << "): " << filename << std::endl;
                cache.invalidate(filename);
            }
        }
    }
    return ptr->error;
}

int NetFS::create(const std::string& filename)
{
    MsgCreate msg(msg_id++, filename);
    sendMsg(msg);
    auto resp = recvMsg();
    auto ptr = dynamic_cast<MsgCreateResp*>(resp.get());
    assert(ptr);
    assert(ptr->id == msg.id);
    return ptr->error;
}

int NetFS::stat(const std::string& filename, struct stat& stbuf)
{
    FileAttr attr;

    int err = do_read_attr(filename, attr);
    if (err)
    {
        cache.invalidate(filename);
        return err;
    }

    if (cache.isStale(filename))
    {
        std::cout << "invalidate due to past stale: " << filename
                  << std::endl;
        cache.invalidate(filename);
    }
    else
    {
        const FileTime* cached_time = cache.getFileTime(filename);
        if (cached_time != nullptr && *cached_time != attr.time)
        {
            std::cout << "invalidate due to stale cache ";
            std::cout << "(time miss match, cached: " << cached_time->mtime
                      << ", remote: " << attr.time.mtime << "): " << filename
                      << std::endl;
            cache.invalidate(filename);
        }
    }

    memset(&stbuf, 0, sizeof(struct stat));
    stbuf.st_size = attr.size;
    stbuf.st_mode = attr.mode;
    stbuf.st_atim.tv_sec = attr.time.atime.time_sec;
    stbuf.st_atim.tv_nsec = attr.time.atime.time_nsec;
    stbuf.st_mtim.tv_sec = attr.time.mtime.time_sec;
    stbuf.st_mtim.tv_nsec = attr.time.mtime.time_nsec;
    stbuf.st_ctim.tv_sec = attr.time.ctime.time_sec;
    stbuf.st_ctim.tv_nsec = attr.time.ctime.time_nsec;
    return 0;
}

int NetFS::statfs(struct statvfs& stbuf)
{
    MsgStatfs msg(msg_id++);
    sendMsg(msg);
    auto resp = recvMsg();
    auto ptr = dynamic_cast<MsgStatfsResp*>(resp.get());
    assert(ptr);
    assert(ptr->id == msg.id);
    if (ptr->error)
    {
        return ptr->error;
    }
    memset(&stbuf, 0, sizeof(struct statvfs));
    stbuf.f_bsize = ptr->stat.bsize;
    stbuf.f_frsize = ptr->stat.frsize;
    stbuf.f_blocks = ptr->stat.blocks;
    stbuf.f_bfree = ptr->stat.bfree;
    stbuf.f_bavail = ptr->stat.bavail;
    stbuf.f_files = ptr->stat.files;
    stbuf.f_ffree = ptr->stat.ffree;
    stbuf.f_namemax = ptr->stat.namemax;
    return 0;
}

int NetFS::readdir(const std::string& filename,
                   std::vector<std::string>& dirs)
{
    MsgReaddir msg(msg_id++, filename);
    sendMsg(msg);
    auto resp = recvMsg();
    auto ptr = dynamic_cast<MsgReaddirResp*>(resp.get());
    assert(ptr);
    assert(ptr->id == msg.id);
    if (ptr->error != 0)
    {
        return ptr->error;
    }
    dirs = std::move(ptr->dir_names);
    return 0;
}

int NetFS::evict()
{
    if (cache.countCachedBlocks() > max_cache_entry)
    {
        int err = cache.evictBlocks(evict_count);
        if (err)
        {
            return err;
        }
    }
    return 0;
}
int NetFS::read(const std::string& filename, off_t offset, size_t size,
                char* buf, size_t& total_read)
{
#ifndef NDEBUG
    std::cout << "NetFS::read(in) filename: " << filename
              << ", offset: " << offset << ", size: " << size << std::endl;
#endif
    int err = cache.read(filename, offset, buf, size, total_read);
    if (err)
    {
        return err;
    }
    err = evict();
    if (err)
    {
        return err;
    }
#ifndef NDEBUG
    std::cout << "NetFS::read(out) total_read: " << total_read
              << ", err: " << err << std::endl;
#endif
    return 0;
}

int NetFS::write(const std::string& filename, off_t offset, const char* buf,
                 size_t size)
{
    int err = cache.write(filename, offset, buf, size);
    if (err)
    {
        return err;
    }
    err = evict();
    if (err)
    {
        return err;
    }
    write_op_count += 1;
    if (write_op_count >= flush_interval)
    {
        write_op_count = 0;
        cache.flushDirtyBlocks();
    }
    return 0;
}

int NetFS::truncate(const std::string& filename, off_t offset)
{
    int err = cache.truncate(filename, offset);
    return err;
}

int NetFS::unlink(const std::string& filename)
{
    MsgUnlink msg(msg_id++, filename);
    sendMsg(msg);
    auto resp = recvMsg();
    auto ptr = dynamic_cast<MsgUnlinkResp*>(resp.get());
    assert(ptr);
    assert(ptr->id == msg.id);
    if (ptr->error == 0)
    {
        std::cout << "invalidate due to unlink: " << filename << std::endl;
        cache.invalidate(filename);
    }
    return ptr->error;
}

int NetFS::rmdir(const std::string& filename)
{
    MsgRmdir msg(msg_id++, filename);
    sendMsg(msg);
    auto resp = recvMsg();
    auto ptr = dynamic_cast<MsgRmdirResp*>(resp.get());
    assert(ptr);
    assert(ptr->id == msg.id);
    if (ptr->error == 0)
    {
        std::cout << "invalidate due to rmdir: " << filename << std::endl;
        cache.invalidate(filename);
    }
    return ptr->error;
}

int NetFS::mkdir(const std::string& filename, mode_t mode)
{
    MsgMkdir msg(msg_id++, filename, mode);
    sendMsg(msg);
    auto resp = recvMsg();
    auto ptr = dynamic_cast<MsgMkdirResp*>(resp.get());
    assert(ptr);
    assert(ptr->id == msg.id);
    return ptr->error;
}

int NetFS::flush(const std::string& filename)
{
    int err = cache.flush(filename);
    return err;
}

int NetFS::rename(const std::string& from, const std::string& to,
                  unsigned int flags)
{
    MsgRename msg(msg_id++, from, to, flags);
    sendMsg(msg);
    auto resp = recvMsg();
    auto ptr = dynamic_cast<MsgRenameResp*>(resp.get());
    assert(ptr);
    assert(ptr->id == msg.id);
    if (ptr->error == 0)
    {
        cache.invalidate(from);
        cache.invalidate(to);
    }
    return ptr->error;
}

void NetFS::sendMsg(const Msg& msg)
{
    serializeMsg(msg, [&](const char* buf, size_t size) mutable {
        writer.write(buf, size);
    });
    writer.flush();
}

std::unique_ptr<Msg> NetFS::recvMsg()
{
    return unserializeMsg(
        [&](char* buf, size_t size) mutable { reader.read(buf, size); });
}

/* write to the file. detect if the file has been modified by other clients,
 * mark it using `stale`.
 * write error, discrepancy between before_change and cached_time, are all
 * treated as stale
 */
int NetFS::do_write(const std::string& filename, off_t offset,
                    const char* buf, size_t size, FileTime& cached_time,
                    bool& stale)
{
    std::vector<char> data;
    data.resize(size);
    std::copy(buf, buf + size, data.begin());
    MsgWrite msg(msg_id++, filename, offset, std::move(data));
    sendMsg(msg);
    auto resp = recvMsg();
    auto ptr = dynamic_cast<MsgWriteResp*>(resp.get());
    assert(ptr);
    assert(ptr->id == msg.id);
    if (ptr->error)
    {
        std::cout << "error detected during write" << std::endl;
        stale = true;
        return ptr->error;
    }
    else
    {
        if (ptr->before_change != cached_time)
        {
            std::cout << "stale detected during write";
            std::cout << "(cached: " << cached_time.mtime;
            std::cout << " remote: " << ptr->before_change.mtime << std::endl;
            stale = true;
        }
        cached_time = ptr->after_change;
        return 0;
    }
}

int NetFS::do_read(const std::string& filename, off_t offset, char* buf,
                   size_t size, size_t& read_size)
{
    MsgRead msg(msg_id++, filename, offset, size);
    sendMsg(msg);
    auto resp = recvMsg();
    auto ptr = dynamic_cast<MsgReadResp*>(resp.get());
    assert(ptr);
    assert(ptr->data.size() <= size);
    if (ptr->error)
    {
        return ptr->error;
    }
    read_size = ptr->data.size();
    std::copy(ptr->data.begin(), ptr->data.end(), buf);
    return 0;
}
int NetFS::do_read_attr(const std::string& filename, FileAttr& attr)
{
    MsgStat msg(msg_id++, filename);
    sendMsg(msg);
    auto resp = recvMsg();
    auto ptr = dynamic_cast<MsgStatResp*>(resp.get());
    assert(ptr);
    assert(ptr->id == msg.id);
    if (ptr->error)
    {
        return ptr->error;
    }
    attr.size = ptr->stat.size;
    attr.mode = ptr->stat.mode;
    attr.time = ptr->stat.time;
    return 0;
}
int NetFS::do_write_attr(const std::string& filename, FileAttr& attr,
                         bool& stale)
{
    MsgTruncate msg(msg_id++, filename, attr.size);
    sendMsg(msg);
    auto resp = recvMsg();
    auto ptr = dynamic_cast<MsgTruncateResp*>(resp.get());
    assert(ptr);
    assert(ptr->id == msg.id);
    if (ptr->error != 0)
    {
        std::cout << "error detected during attr write" << std::endl;
        stale = true;
        return ptr->error;
    }
    else
    {
        if (ptr->before_change != attr.time)
        {
            std::cout << "stale detected during attr write: ";
            std::cout << "(cached: " << attr.time.mtime,
                std::cout << " remote: " << ptr->before_change.mtime
                          << std::endl;
            stale = true;
        }
        attr.time = ptr->after_change;
        return 0;
    }
}
