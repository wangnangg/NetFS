#include "netfs.hpp"
#include <netdb.h>
#include <netinet/in.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <cassert>
#include <cstring>
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

NetFS::NetFS(const std::string& hostname, const std::string& port)
    : id(0), writer(), reader()
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
const std::string content = "hello world!\n";

int NetFS::open(const std::string& filename, int flags)
{
    MsgOpen msg(id++, flags, filename);
    sendMsg(msg);
    auto resp = recvMsg();
    auto ptr = dynamic_cast<MsgOpenResp*>(resp.get());
    assert(ptr);
    assert(ptr->id == msg.id);
    return ptr->error;
}
int NetFS::stat(const std::string& filename, struct stat& stbuf)
{
    MsgStat msg(id++, filename);
    sendMsg(msg);
    auto resp = recvMsg();
    auto ptr = dynamic_cast<MsgStatResp*>(resp.get());
    assert(ptr);
    assert(ptr->id == msg.id);
    if (ptr->error != 0)
    {
        return ptr->error;
    }
    memset(&stbuf, 0, sizeof(struct stat));
    stbuf.st_size = ptr->stat.size;
    stbuf.st_mode = ptr->stat.mode;
    return 0;
}

int NetFS::readdir(const std::string& filename, std::vector<Dirent>& dirs)
{
    MsgReaddir msg(id++, filename);
    sendMsg(msg);
    auto resp = recvMsg();
    auto ptr = dynamic_cast<MsgReaddirResp*>(resp.get());
    assert(ptr);
    assert(ptr->id == msg.id);
    if (ptr->error != 0)
    {
        return ptr->error;
    }
    for (auto n : ptr->dir_names)
    {
        dirs.push_back({n});
    }
    return 0;
}

// size is changed to reflect the actual size read, size of 0 indicates
// EOF
int NetFS::read(const std::string& filename, off_t offset, char* buf,
                size_t& size)
{
    MsgRead msg(id++, filename, offset, size);
    sendMsg(msg);
    auto resp = recvMsg();
    auto ptr = dynamic_cast<MsgReadResp*>(resp.get());
    assert(ptr);
    assert(ptr->id == msg.id);
    if (ptr->error != 0)
    {
        return ptr->error;
    }
    assert(ptr->data.size() <= size);
    size = ptr->data.size();
    std::copy(ptr->data.begin(), ptr->data.end(), buf);
    return 0;
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
