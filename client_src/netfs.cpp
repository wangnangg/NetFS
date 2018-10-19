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
    std::cout << "open" << std::endl;
    MsgOpen msg(id++, flags, filename);
    sendMsg(msg);
    if (filename != "/hello")
    {
        return ENOENT;
    }
    if ((flags & O_ACCMODE) != O_RDONLY) return EACCES;
    // auto resp = recvMsg();
    // auto ptr = dynamic_cast<MsgOpenResp*>(reps.get());
    return 0;
}
int NetFS::stat(const std::string& filename, struct stat& stbuf)
{
    std::cout << "stat" << std::endl;
    memset(&stbuf, 0, sizeof(struct stat));
    if (filename == "/")
    {
        stbuf.st_mode = S_IFDIR | 0755;
        stbuf.st_nlink = 2;
    }
    else if (filename == "/hello")
    {
        stbuf.st_mode = S_IFREG | 0444;
        stbuf.st_nlink = 1;
        stbuf.st_size = content.size();
    }
    else
    {
        return ENOENT;
    }
    return 0;
}

int NetFS::readdir(const std::string& filename, std::vector<Dirent>& dirs)
{
    std::cout << "readdir" << std::endl;
    if (filename != "/")
    {
        return ENOENT;
    }
    dirs.push_back(Dirent{"."});
    dirs.push_back(Dirent{".."});
    dirs.push_back(Dirent{"hello"});
    return 0;
}

// size is changed to reflect the actual size read, size of 0 indicates
// EOF
int NetFS::read(const std::string& filename, off_t offset, char* buf,
                size_t& size)
{
    std::cout << "read" << std::endl;
    if (filename != "/hello")
    {
        size = 0;
        return ENOENT;
    }
    std::cout << "offset size: " << offset << ", " << size << std::endl;
    if (offset < 0 || (size_t)offset >= content.size())
    {
        size = 0;
        return 0;
    }
    if (offset + size > content.size())
    {
        size = content.size() - offset;
    }
    auto start = content.begin() + offset;
    auto end = start + size;
    std::copy(start, end, buf);
    std::cout << "copied: " << size << std::endl;
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
