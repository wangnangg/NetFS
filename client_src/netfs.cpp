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
             size_t block_size)
    : msg_id(0),
      writer(),
      reader(),
      block_size(block_size),
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
    (void)flags;
    (void)mode;
    FileAttr attr;
    int err = cache.readFileAttr(filename, attr);
    if (err)
    {
        return -err;
    }
    return 0;
}

int NetFS::stat(const std::string& filename, struct stat& stbuf)
{
    FileAttr attr;
    int err = cache.readFileAttr(filename, attr);
    if (err)
    {
        return -err;
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

int NetFS::fetchBlocks(const std::string& filename, uint32_t block_start,
                       uint32_t block_end, uint32_t& block_count)
{
    assert(block_start < block_end);
    size_t size = (block_end - block_start) * block_size;
    MsgRead msg(msg_id++, filename, block_start * block_size, size);
    auto resp = recvMsg();
    auto ptr = dynamic_cast<MsgReadResp*>(resp.get());
    assert(ptr);
    assert(ptr->id == msg.id);
    if (ptr->error != 0)
    {
        return ptr->error;
    }
    size_t read_size = 0;

    uint32_t curr_block = block_start;
    while (ptr->data.size() - read_size >= block_size)
    {
        auto start = ptr->data.begin() + read_size;
        auto end = start + block_size;
        std::vector<char> nb(start, end);
        cache.storeBlock(filename, curr_block, std::move(nb));
        read_size += block_size;
        curr_block += 1;
    }
    if (ptr->data.size() - read_size > 0)
    {
        std::vector<char> nb(block_size, 0);
        auto start = ptr->data.begin() + read_size;
        auto end = ptr->data.end();
        std::copy(start, end, &nb[0]);
        cache.storeBlock(filename, curr_block, std::move(nb));
        curr_block += 1;
    }
    block_count = curr_block - block_start;
    return 0;
}

std::pair<size_t, size_t> blockRange(off_t ofset, size_t size,
                                     size_t block_size);

int NetFS::read(const std::string& filename, off_t offset, size_t size,
                char* buf, size_t& total_read)
{
    int err = cache.read(filename, offset, buf, size, total_read);
    return -err;
}

int NetFS::write(const std::string& filename, off_t offset, const char* buf,
                 size_t size)
{
    int err = cache.write(filename, offset, buf, size);
    return -err;
}

int NetFS::truncate(const std::string& filename, off_t offset)
{
    int err = cache.writeFileAttr(filename, FileAttr{(size_t)offset, 0755});
    return -err;
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
        cache.invalidateFile(filename);
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
