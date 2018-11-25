#include "fileop.hpp"

int loadTime(const std::string& fname, FileTime& time)
{
    struct stat stbuf;
    int err = stat(fname.c_str(), &stbuf);
    if (err == 0)
    {
        time = makeFileTime(stbuf);
        return 0;
    }
    else
    {
        return err;
    }
}

int FileOp::access(const std::string& fpath, FileTime& time)
{
    auto filename = _root + fpath;

    int err = ::access(filename.c_str(), R_OK | W_OK);
    if (err < 0)
    {
        return errno;
    }
    err = loadTime(filename, time);
    if (err < 0)
    {
        return errno;
    }

    return 0;
}

int FileOp::creat(const std::string& fpath)
{
    auto filename = _root + fpath;

    int fd = ::open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 00777);
    if (fd < 0)
    {
        return errno;
    }
    ::close(fd);
    return 0;
}

int FileOp::stat(const std::string& fpath, struct stat& stbuf)
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

int FileOp::statfs(FsStat& stat)
{
    struct statvfs stbuf;
    int err = ::statvfs(_root.c_str(), &stbuf);
    if (err < 0)
    {
        return errno;
    }
    stat = makeFsStat(stbuf);
    return 0;
}

int FileOp::readdir(const std::string& fpath,
                    std::vector<std::string>& dirnames)
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
    closedir(dirs);
    return errno;
}

int FileOp::read(const std::string& fpath, off_t offset, size_t size,
                 char* buf, size_t& total_read)
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
        ::close(fd);
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

int FileOp::write(const std::string& fpath, off_t offset, const char* buf,
                  size_t size, FileTime& before_change,
                  FileTime& after_change)
{
    auto filename = _root + fpath;
    int err = loadTime(filename, before_change);
    if (err < 0)
    {
        return errno;
    }

    int fd = ::open(filename.c_str(), O_WRONLY | O_CREAT, 0766);
    if (fd < 0)
    {
        return errno;
    }
    err = ::lseek(fd, offset, SEEK_SET);
    if (err < 0)
    {
        ::close(fd);
        return errno;
    }
    ssize_t written_size = ::write(fd, buf, size);
    ::close(fd);

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

    err = loadTime(filename, after_change);
    if (err < 0)
    {
        return errno;
    }

    return 0;
}

int FileOp::truncate(const std::string& fpath, off_t offset,
                     FileTime& before_change, FileTime& after_change)
{
    auto filename = _root + fpath;
    int err = loadTime(filename, before_change);
    if (err < 0)
    {
        return errno;
    }
    int res = ::truncate(filename.c_str(), offset);
    if (res < 0)
    {
        return errno;
    }

    err = loadTime(filename, after_change);
    if (err < 0)
    {
        return errno;
    }

    return 0;
}

int FileOp::unlink(const std::string& fpath)
{
    auto filename = _root + fpath;
    int res = ::unlink(filename.c_str());
    if (res < 0)
    {
        return errno;
    }
    else
    {
        return res;
    }
}
int FileOp::rmdir(const std::string& fpath)
{
    auto filename = _root + fpath;
    int res = ::rmdir(filename.c_str());
    if (res < 0)
    {
        return errno;
    }
    else
    {
        return res;
    }
}

int FileOp::mkdir(const std::string& fpath, mode_t mode)
{
    auto filename = _root + fpath;
    int res = ::mkdir(filename.c_str(), mode);
    if (res < 0)
    {
        return errno;
    }
    else
    {
        return res;
    }
}

int FileOp::rename(const std::string& from, const std::string& to,
                   unsigned int flags)
{
    int err = ::rename((_root + from).c_str(), (_root + to).c_str());
    if (err)
    {
        return errno;
    }
    return 0;
}
