#pragma once
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include "msg.hpp"
#include "serial.hpp"
#include "stream.hpp"

/* all file operations return errno
 * all methods have no throw guarantee
 */
class NetFS
{
    int id;
    FdWriter writer;
    FdReader reader;

public:
    NetFS(const std::string& hostname, const std::string& port);

    int open(const std::string& filename, int flags);
    int stat(const std::string& filename, struct stat& statbuf);

    struct Dirent
    {
        std::string name;
    };
    int readdir(const std::string& filename, std::vector<Dirent>& dirs);

    // size is changed to reflect the actual size read, size of 0 indicates
    // EOF
    int read(const std::string& filename, off_t offset, char* buf,
             size_t& size);

private:
    void sendMsg(const Msg& msg);
    std::unique_ptr<Msg> recvMsg();
};
