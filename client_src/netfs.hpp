#pragma once
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdint>
#include <string>
#include <vector>

/* all file operations return errno
 * all methods have no throw guarantee
 */
class NetFS
{
public:
    NetFS(uint32_t ip, uint16_t port);

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
};
