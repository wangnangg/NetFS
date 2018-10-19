#pragma once
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <vector>

/* A class to rule them (file operations) all
 */
class FileOp
{
    std::string hellopath = "/hello";
    std::string content = "hello world! -- from server";

public:
    int open(const std::string& filename, int flags)
    {
        if (filename != hellopath)
        {
            return ENOENT;
        }
        if ((flags & O_ACCMODE) != O_RDONLY) return EACCES;
        return 0;
    }
    int stat(const std::string& filename, struct stat& stbuf)
    {
        memset(&stbuf, 0, sizeof(struct stat));
        if (filename == "/")
        {
            stbuf.st_mode = S_IFDIR | 0755;
            stbuf.st_nlink = 2;
        }
        else if (filename == hellopath)
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

    int readdir(const std::string& filename,
                std::vector<std::string>& dirnames)
    {
        if (filename != "/")
        {
            return ENOENT;
        }
        dirnames.push_back(".");
        dirnames.push_back("..");
        dirnames.push_back("hello");
        return 0;
    }

    int read(const std::string& filename, off_t offset, size_t size,
             std::vector<char>& res)
    {
        if (filename != hellopath)
        {
            res.resize(0);
            return ENOENT;
        }
        if (offset < 0 || (size_t)offset >= content.size())
        {
            res.resize(0);
            return 0;
        }
        if (offset + size > content.size())
        {
            size = content.size() - offset;
        }
        res.resize(size);
        auto start = content.begin() + offset;
        auto end = start + size;
        std::copy(start, end, res.begin());
        return 0;
    }
};
