#include "time.hpp"

TimeSpec makeTimeSpec(const struct timespec& ts)
{
    TimeSpec res;
    res.time_sec = ts.tv_sec;
    res.time_nsec = ts.tv_nsec;
    return res;
}

FileTime makeFileTime(const struct stat& st)
{
    FileTime ft;
    ft.atime = makeTimeSpec(st.st_atim);
    ft.mtime = makeTimeSpec(st.st_mtim);
    ft.ctime = makeTimeSpec(st.st_ctim);
    return ft;
}

char* time2str(const time_t* when, long ns)
{
    char* ans = (char*)malloc(128 * sizeof(*ans));
    char temp1[64];
    const struct tm* t = localtime(when);
    strftime(temp1, 512, "%Y-%m-%d %H:%M:%S", t);
    snprintf(ans, 128, "%s.%09ld", temp1, ns);
    return ans;
}

std::ostream& operator<<(std::ostream& os, const TimeSpec& t)
{
    struct timespec tim;
    tim.tv_sec = t.time_sec;
    tim.tv_nsec = t.time_nsec;
    char* str = time2str(&tim.tv_sec, tim.tv_nsec);
    os << str;
    free(str);
    return os;
}

bool operator==(const TimeSpec& t1, const TimeSpec& t2)
{
    return t1.time_sec == t2.time_sec && t1.time_nsec == t2.time_nsec;
}

bool operator!=(const TimeSpec& t1, const TimeSpec& t2)
{
    return !(t1 == t2);
}
bool operator==(const FileTime& t1, const FileTime& t2)
{
    return t1.mtime == t2.mtime && t1.ctime == t2.ctime;
}

bool operator!=(const FileTime& t1, const FileTime& t2)
{
    return !(t1 == t2);
}
