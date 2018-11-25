#include "cache.hpp"
#include <gtest/gtest.h>
#include <pwd.h>
#include <stdio.h>
#include <unistd.h>
#include <stdexcept>

static const char* getUserName()
{
    uid_t uid = geteuid();
    struct passwd* pw = getpwuid(uid);
    if (pw)
    {
        return pw->pw_name;
    }

    return "";
}

static std::string tmpFilename(const std::string& name)
{
    return (std::string("/tmp/") + getUserName() + "." + name).c_str();
}

static std::vector<char> readAll(const std::string& fname)
{
    auto fpath = tmpFilename(fname);
    FILE* fp = fopen(fpath.c_str(), "r");
    assert(fp);
    size_t max = 1000;
    std::vector<char> buf(max);
    int read_size = fread(&buf[0], sizeof(char), 1000, fp);
    assert(read_size >= 0);
    buf.resize(read_size);
    return buf;
}

static void beforeChange(const std::string& fname, FileTime& time,
                         bool& stale)
{
    struct stat stbuf;
    int err = stat(fname.c_str(), &stbuf);
    assert(err == 0);
    if (time != makeFileTime(stbuf))
    {
        stale = true;
    }
}

static void afterChange(const std::string& fname, FileTime& time)
{
    struct stat stbuf;
    int err = stat(fname.c_str(), &stbuf);
    assert(err == 0);
    time = makeFileTime(stbuf);
}

static int writeContent(const std::string& fname, size_t offset,
                        const char* data, size_t size, FileTime& time,
                        bool& stale)
{
    std::cout << "write to " << fname << " at " << offset
              << " of size: " << size;
    std::string str(data, size);
    std::cout << ". content: " << str << std::endl;
    auto fpath = tmpFilename(fname);
    beforeChange(fpath, time, stale);
    FILE* fp = fopen(fpath.c_str(), "r+");
    int err = fseek(fp, offset, SEEK_SET);
    assert(err == 0);
    err = fwrite(data, 1, size, fp);
    assert(err == (int)size);
    fclose(fp);
    afterChange(fpath, time);
    return 0;
}

static int writeAttr(const std::string& fname, FileAttr& attr, bool& stale)
{
    std::cout << "truncating file: " << fname << " to size: " << attr.size
              << std::endl;
    auto fpath = tmpFilename(fname);
    beforeChange(fpath, attr.time, stale);
    int res = truncate(fpath.c_str(), attr.size);
    if (res < 0)
    {
        perror("truncate");
    }
    assert(res == 0);
    afterChange(fpath, attr.time);
    return 0;
}

static int readContent(const std::string& fname, size_t offset, char* data,
                       size_t size, size_t& read_size)
{
    auto fpath = tmpFilename(fname);
    FILE* fp = fopen(fpath.c_str(), "r");
    int err = fseek(fp, offset, SEEK_SET);
    assert(err == 0);
    read_size = fread(data, sizeof(*data), size, fp);
    assert(read_size >= 0);
    if (read_size < size)
    {
        assert(feof(fp));
    }
    return 0;
}

static int readAttr(const std::string& fname, FileAttr& attr)
{
    auto fpath = tmpFilename(fname);
    struct stat stbuf;
    int res = stat(fpath.c_str(), &stbuf);
    if (res < 0)
    {
        return errno;
    }
    attr.mode = stbuf.st_mode;
    attr.size = stbuf.st_size;
    return 0;
}

static void createFile(const std::string& fname)
{
    auto fpath = tmpFilename(fname);
    FILE* fp = fopen(fpath.c_str(), "w");
    fclose(fp);
}

TEST(cache, read_hit_1)
{
    Cache cache(1 << 4, writeContent, writeAttr, readContent, readAttr);
    std::vector<char> data;
    for (int i = 40; i < 120; i++)
    {
        data.push_back(i);
    }
    std::string fname = "cache_read_hit_1";
    createFile(fname);
    int err = cache.write(fname, 0, &data[0], 20);
    if (err > 0)
    {
        std::cout << strerror(err) << std::endl;
    }
    ASSERT_EQ(err, 0);
    std::vector<char> read_data(1000);
    size_t read_size;
    err = cache.read(fname, 0, &read_data[0], 15, read_size);
    ASSERT_EQ(err, 0);
    ASSERT_EQ(read_size, 15);
    for (int i = 0; i < 15; i++)
    {
        ASSERT_EQ(data[i], i + 40);
    }
    ASSERT_TRUE(cache.isLastReadHit());
    err = cache.read(fname, 15, &read_data[15], 15, read_size);
    ASSERT_EQ(err, 0);
    ASSERT_EQ(read_size, 5);
    for (int i = 15; i < 15 + 5; i++)
    {
        ASSERT_EQ(data[i], i + 40);
    }
    ASSERT_TRUE(!cache.isLastReadHit());
}

TEST(cache, read_hit_2)
{
    Cache cache(1 << 4, writeContent, writeAttr, readContent, readAttr);
    std::vector<char> data;
    for (int i = 40; i < 120; i++)
    {
        data.push_back(i);
    }
    std::string fname = "cache_read_hit_2";
    createFile(fname);
    int err = cache.write(fname, 0, &data[0], 20);
    if (err > 0)
    {
        std::cout << strerror(err) << std::endl;
    }
    ASSERT_EQ(err, 0);
    cache.flush(fname);
    cache.invalidate(fname);
    std::vector<char> read_data(1000);
    size_t read_size;
    err = cache.read(fname, 0, &read_data[0], 17, read_size);
    for (int i = 0; i < 17; i++)
    {
        ASSERT_EQ(data[i], i + 40);
    }
    ASSERT_EQ(err, 0);
    ASSERT_EQ(read_size, 17);
    ASSERT_FALSE(cache.isLastReadHit());
    err = cache.read(fname, 0, &read_data[0], 20, read_size);
    for (int i = 0; i < 20; i++)
    {
        ASSERT_EQ(data[i], i + 40);
    }
    ASSERT_EQ(err, 0);
    ASSERT_EQ(read_size, 20);
    ASSERT_TRUE(cache.isLastReadHit());
}

TEST(cache, flush)
{
    Cache cache(1 << 4, writeContent, writeAttr, readContent, readAttr);
    std::vector<char> data;
    for (int i = 0; i < 26; i++)
    {
        data.push_back(i + 'A');
    }
    std::string fname = "cache_flush";
    createFile(fname);
    int err = cache.write(fname, 0, &data[0], 13);
    ASSERT_EQ(err, 0);
    err = cache.write(fname, 13, &data[13], 13);
    ASSERT_EQ(err, 0);
    cache.flush(fname);

    auto read_data = readAll(fname);
    ASSERT_EQ(read_data.size(), 26);
    for (int i = 0; i < 26; i++)
    {
        ASSERT_EQ(read_data[i], data[i]);
    }
}

TEST(cache, stat)
{
    Cache cache(1 << 4, writeContent, writeAttr, readContent, readAttr);
    std::string fname = "cache_stat";
    createFile(fname);
    int err = cache.truncate(fname, 12);
    ASSERT_EQ(err, 0);
    err = cache.flush(fname);
    ASSERT_EQ(err, 0);
    cache.invalidate(fname);
    FileAttr attr;
    err = cache.stat(fname, attr);
    ASSERT_EQ(err, 0);
    ASSERT_EQ(attr.size, 12);
}

TEST(cache, evict)
{
    Cache cache(1 << 4, writeContent, writeAttr, readContent, readAttr);
    std::string fname = "cache_evict";
    std::vector<char> data;
    for (int i = 40; i < 120; i++)
    {
        data.push_back(i);
    }
    createFile(fname);
    int err = cache.write(fname, 0, &data[0], 20);
    if (err > 0)
    {
        std::cout << strerror(err) << std::endl;
    }
    ASSERT_EQ(err, 0);
    ASSERT_EQ(cache.countCachedBlocks(), 2);
    auto rd = readAll(fname);
    ASSERT_EQ(rd.size(), 0);

    std::vector<char> read_data(30);
    size_t read_size;
    err = cache.read(fname, 0, &read_data[0], 1, read_size);
    ASSERT_EQ(err, 0);
    ASSERT_TRUE(cache.isLastReadHit());

    std::cout << "start evicting..." << std::endl;
    err = cache.evictBlocks(3);
    ASSERT_EQ(err, 0);
    ASSERT_EQ(cache.countCachedBlocks(), 0);
    rd = readAll(fname);
    ASSERT_EQ(rd.size(), 20);
    for (int i = 0; i < 20; i++)
    {
        ASSERT_EQ(rd[i], data[i]);
    }

    err = cache.read(fname, 0, &read_data[0], 1, read_size);
    ASSERT_EQ(err, 0);
    ASSERT_FALSE(cache.isLastReadHit());
    ASSERT_EQ(cache.countCachedBlocks(), 1);
    err = cache.read(fname, 0, &read_data[0], 1, read_size);
    ASSERT_TRUE(cache.isLastReadHit());
}
