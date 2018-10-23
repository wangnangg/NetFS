#pragma once
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

/* A class to rule them (file operations) all
 * all function resembles the system call except returns errno or 0.
 */
class FileOp
{
    std::string _root;

public:
    FileOp(std::string root) : _root(std::move(root)) {}
    int stat(const std::string& fpath, struct stat& stbuf)
    {
        auto filename = _root + fpath;
        int res = ::stat(filename.c_str(), &stbuf);
        if (res < 0)
        {
            return errno;
        }
        else
        {
            return 0;
        }
    }

    int readdir(const std::string& fpath, std::vector<std::string>& dirnames)
    {
        auto filename = _root + fpath;
        DIR* dirs = ::opendir(filename.c_str());
        if (dirs == NULL)
        {
            return errno;
        }
        errno = 0;
        while (struct dirent* ent = ::readdir(dirs))
        {
            dirnames.push_back(ent->d_name);
        }
        return errno;
    }

    int read(const std::string& fpath, off_t offset, size_t size, char* buf,
             size_t& total_read)
    {
        auto filename = _root + fpath;
        int fd = ::open(filename.c_str(), O_RDONLY);
        total_read = 0;
        if (fd < 0)
        {
            return errno;
        }
        int err = ::lseek(fd, offset, SEEK_SET);
        if (err < 0)
        {
            close(fd);
            return errno;
        }
        ssize_t read_size = 0;
        total_read = 0;
        while (total_read < size)
        {
            read_size = ::read(fd, buf + total_read, size - total_read);
            if (read_size <= 0)
            {
                break;
            }
            total_read += read_size;
        }
        assert(total_read <= size);
        ::close(fd);
        if (read_size >= 0)
        {
            return 0;
        }
        else
        {
            return errno;
        }
    }

    int write(const std::string& fpath, off_t offset, const char* buf,
              size_t size)
    {
        auto filename = _root + fpath;
        int fd = ::open(filename.c_str(), O_WRONLY | O_CREAT,
                        S_IRWXU | S_IRWXG | S_IRWXO);
        if (fd < 0)
        {
            return errno;
        }
        int err = ::lseek(fd, offset, SEEK_SET);
        if (err < 0)
        {
            close(fd);
            return errno;
        }
        ssize_t written_size = ::write(fd, buf, size);
        close(fd);
        if (written_size < 0)
        {
            perror("write");
            std::cerr << "write errno" << std::endl;
            return errno;
        }
        else if (written_size < (ssize_t)size)
        {
            std::cerr << "write too small" << written_size << "v.s. " << size
                      << std::endl;
            return EDQUOT;
        }
        else
        {
            return 0;
        }
    }
};
