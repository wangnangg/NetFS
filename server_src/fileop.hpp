#pragma once
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>

/* A class to rule them (file operations) all
 */
class FileOp
{
public:
    int open(const std::string& filename, int flags)
    {
        if (filename != "/hello")
        {
            return ENOENT;
        }
        if ((flags & O_ACCMODE) != O_RDONLY) return EACCES;
        return 0;
    }
};
