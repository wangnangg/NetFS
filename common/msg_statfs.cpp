#include "msg_statfs.hpp"

FsStat makeFsStat(const struct statvfs& st)
{
    FsStat fs;
    fs.bsize = st.f_bsize;
    fs.frsize = st.f_frsize;
    fs.blocks = st.f_blocks;
    fs.bfree = st.f_bfree;
    fs.bavail = st.f_bavail;
    fs.files = st.f_files;
    fs.ffree = st.f_ffree;
    fs.namemax = st.f_namemax;
    return fs;
}
