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

    int open(const std::string& filename, int flags, mode_t mode);
    int stat(const std::string& filename, struct stat& statbuf);

    int readdir(const std::string& filename, std::vector<std::string>& dirs);

    int read(const std::string& filename, off_t offset, size_t size,
             char* buf, size_t& total_read);

    int write(const std::string& filename, off_t offset, const char* buf,
              size_t size);

    int truncate(const std::string& filename, off_t offset);

    int unlink(const std::string& filename);
    int rmdir(const std::string& filename);

private:
    void sendMsg(const Msg& msg);
    std::unique_ptr<Msg> recvMsg();
};
