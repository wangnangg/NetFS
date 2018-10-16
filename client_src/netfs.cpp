#include "netfs.hpp"
#include <sys/errno.h>
#include <sys/file.h>
#include <cassert>
#include <cstring>
#include <iostream>

NetFS::NetFS(uint32_t ip, uint16_t port)
{
    std::cout << "server addr: " << ip << ":" << port << std::endl;
}
const std::string content = "hello world!\n";
int NetFS::open(const std::string& filename, int flags)
{
    std::cout << "open" << std::endl;
    if (filename != "/hello")
    {
        return ENOENT;
    }
    if ((flags & O_ACCMODE) != O_RDONLY) return EACCES;
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
