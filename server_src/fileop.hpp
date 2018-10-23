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
    int open(const std::string& fpath, int flags, mode_t mode);
    int stat(const std::string& fpath, struct stat& stbuf);
    int readdir(const std::string& fpath, std::vector<std::string>& dirnames);
    int read(const std::string& fpath, off_t offset, size_t size, char* buf,
             size_t& total_read);
    int write(const std::string& fpath, off_t offset, const char* buf,
              size_t size);
    int truncate(const std::string& fpath, off_t offset);
    int unlink(const std::string& filename);
    int rmdir(const std::string& filename);
    int mkdir(const std::string& filename, mode_t mode);
};
