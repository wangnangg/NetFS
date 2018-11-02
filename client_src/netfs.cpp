#include "netfs.hpp"
#include <netdb.h>
#include <netinet/in.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/socket.h>
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
             size_t block_size, size_t max_cache_entry, size_t dirty_thresh)
    : msg_id(0),
      writer(),
      reader(),
      block_size(block_size),
      max_cache_entry(max_cache_entry),
      dirty_threshold(dirty_thresh),
      cache(block_size, std::bind(&NetFS::do_write, this, _1, _2, _3, _4),
            std::bind(&NetFS::do_write_attr, this, _1, _2),
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

int NetFS::open(const std::string& filename, int flags, mode_t mode)
{
    MsgOpen msg(msg_id++, flags, filename, mode);
    sendMsg(msg);
    auto resp = recvMsg();
    auto ptr = dynamic_cast<MsgOpenResp*>(resp.get());
    assert(ptr);
    assert(ptr->id == msg.id);
    return ptr->error;
}

int NetFS::stat(const std::string& filename, struct stat& stbuf)
{
    FileAttr attr;
    int err = cache.stat(filename, attr);
    if (err)
    {
        return err;
    }
    memset(&stbuf, 0, sizeof(struct stat));
    stbuf.st_size = attr.size;
    stbuf.st_mode = attr.mode;
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
        std::cout << "evicting" << std::endl;
        int err = cache.evictBlocks(cache.countCachedBlocks() -
                                    (max_cache_entry / 2));
        if (err)
        {
            return err;
        }
        std::cout << "evicted." << std::endl;
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

int NetFS::do_write(const std::string& filename, off_t offset,
                    const char* buf, size_t size)
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
    return ptr->error;
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
    if (ptr->error != 0)
    {
        return ptr->error;
    }
    attr.size = ptr->stat.size;
    attr.mode = ptr->stat.mode;
    return 0;
}
int NetFS::do_write_attr(const std::string& filename, FileAttr attr)
{
    MsgTruncate msg(msg_id++, filename, attr.size);
    sendMsg(msg);
    auto resp = recvMsg();
    auto ptr = dynamic_cast<MsgTruncateResp*>(resp.get());
    assert(ptr);
    assert(ptr->id == msg.id);
    return ptr->error;
}
