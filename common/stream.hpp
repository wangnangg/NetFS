#pragma once
#include <unistd.h>
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <system_error>
#include <vector>

/* a buffered reader for reading from a file descriptor (or a socket really)
 */
class FdReader
{
    FILE* _f;

public:
    FdReader() : _f(nullptr) {}
    FdReader(int fd) : _f(nullptr)
    {
        _f = fdopen(fd, "r");
        if (_f == nullptr)
        {
            throw std::system_error(errno, std::system_category());
        }
    }

    // rule of five
    ~FdReader()
    {
        if (_f != nullptr)
        {
            fclose(_f);
        }
    }

    // copy is disabled as two instances reading from the same socket is not
    // useful in our case
    FdReader(const FdReader&) = delete;
    FdReader& operator=(const FdReader&) = delete;

    FdReader(FdReader&& reader)
    {
        _f = reader._f;
        reader._f = nullptr;
    }
    FdReader& operator=(FdReader&& reader)
    {
        std::swap(this->_f, reader._f);
        return *this;
    }

    // requesting at most *size* data to be read into *buf*
    // return the actual number of bytes read
    // read will try to read as many as *size*, unless EOF, in which case, the
    // return value may be smaller. error are generated throw system_error
    // exception
    size_t read(char* buf, size_t size)
    {
        size_t res = fread(buf, 1, size, _f);
        if (res < size)
        {
            if (ferror(_f))
            {
                throw std::system_error(errno, std::system_category());
            }
            else
            {
                assert(feof(_f));
                return res;
            }
        }
        assert(res == size);
        return res;
    }
};

/* a buffered writer for writing to a file descriptor (or a socket really)
 */
class FdWriter
{
    FILE* _f;

public:
    FdWriter() : _f(nullptr) {}
    FdWriter(int fd) : _f(nullptr)
    {
        _f = fdopen(fd, "w");
        if (_f == nullptr)
        {
            throw std::system_error(errno, std::system_category());
        }
    }

    // rule of five
    ~FdWriter()
    {
        if (_f != nullptr)
        {
            fclose(_f);
        }
    }

    // copy is disabled as two instances writing to the same socket is not
    // useful in our case
    FdWriter(const FdWriter&) = delete;
    FdWriter& operator=(const FdWriter&) = delete;

    FdWriter(FdWriter&& rhs)
    {
        _f = rhs._f;
        rhs._f = nullptr;
    }
    FdWriter& operator=(FdWriter&& rhs)
    {
        std::swap(this->_f, rhs._f);
        return *this;
    }

    // requesting at most *size* data to be written from *buf* to the fd
    // return the actual number of bytes read
    // write will try to write as many as *size*, unless error, in that case,
    // return value may be smaller. error are generated throw system_error
    // exception
    size_t write(char* buf, size_t size)
    {
        size_t res = fwrite(buf, 1, size, _f);
        if (res < size)
        {
            assert(ferror(_f));
            throw std::system_error(errno, std::system_category());
        }
        assert(res == size);
        return res;
    }
};
