#pragma once

#include <stdint.h>
#include <sys/stat.h>
#include <iostream>
// make sure no padding is inserted. The layout should be identical in any
// machine.
#pragma pack(push, 1)
struct TimeSpec
{
    int64_t time_sec;
    int64_t time_nsec;
};

struct FileTime
{
    TimeSpec atime;
    TimeSpec ctime;
    TimeSpec mtime;
};
#pragma pack(pop)

TimeSpec makeTimeSpec(const struct timespec& ts);
FileTime makeFileTime(const struct stat& st);

bool operator==(const TimeSpec& t1, const TimeSpec& t2);
bool operator!=(const TimeSpec& t1, const TimeSpec& t2);
std::ostream& operator<<(std::ostream& os, const TimeSpec& t);

bool operator==(const FileTime& t1, const FileTime& t2);
bool operator!=(const FileTime& t1, const FileTime& t2);
