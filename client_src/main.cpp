/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.
*/

/** @file
 *
 * minimal example filesystem using high-level API
 *
 * Compile with:
 *
 *     gcc -Wall hello.c `pkg-config fuse3 --cflags --libs` -o hello
 *
 * ## Source code ##
 * \include hello.c
 */

#define FUSE_USE_VERSION 31

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <memory>
#include <stdexcept>
#include "execinfo.h"
#include "fuse.h"
#include "netfs.hpp"

/*
 * Command line options
 *
 * We can't set default values for the char* fields here because
 * fuse_opt_parse would attempt to free() them when the user specifies
 * different values on the command line.
 */
static struct options
{
    const char *hostname;
    const char *port;
    const char *block_size;      // in kb
    const char *cache_size;      // in MB
    const char *evict_count;     // number of blocks to evict when cache full
    const char *flush_interval;  // num of writes before flushing
    int show_help;
} options;

#define OPTION(t, p)                      \
    {                                     \
        t, offsetof(struct options, p), 1 \
    }
static const struct fuse_opt option_spec[] = {
    OPTION("--hostname=%s", hostname),
    OPTION("--port=%s", port),
    OPTION("--block_size=%s", block_size),
    OPTION("--cache_size=%s", cache_size),
    OPTION("--evict_count=%s", evict_count),
    OPTION("--flush_interval=%s", flush_interval),
    OPTION("-h", show_help),
    OPTION("--help", show_help),
    FUSE_OPT_END};

static void *nfs_init(struct fuse_conn_info *conn, struct fuse_config *cfg)
{
#ifndef NDEBUG
    std::cout << "nfs_init" << std::endl;
#endif
    (void)conn;
    cfg->kernel_cache = 0;
    size_t k = 1 << 10;
    size_t block_size = atoi(options.block_size);
    if (block_size == 0)
    {
        block_size = 4;
    }
    size_t cache_size = atoi(options.cache_size);
    if (cache_size == 0)
    {
        cache_size = 256;
    }
    size_t max_entry = cache_size * k / block_size;
    if (max_entry == 0)
    {
        max_entry = 1024;
    }
    size_t evict_count = atoi(options.evict_count);
    if (evict_count == 0)
    {
        evict_count = 100;
    }
    size_t flush_interval = atoi(options.flush_interval);
    if (flush_interval == 0)
    {
        flush_interval = 10000;
    }
    std::cout << "server: " << options.hostname << std::endl;
    std::cout << "port: " << options.port << std::endl;
    std::cout << "cache block size: " << block_size << " KB" << std::endl;
    std::cout << "cache size: " << max_entry * block_size / k << " MB"
              << std::endl;
    std::cout << "cache evict count: " << evict_count << std::endl;
    std::cout << "flush interval: " << flush_interval << std::endl;
    auto netfs = new NetFS(options.hostname, options.port, block_size * k,
                           max_entry, evict_count, flush_interval);
    return netfs;
}

static int nfs_getattr(const char *path, struct stat *stbuf,
                       struct fuse_file_info *fi)
{
#ifndef NDEBUG
    std::cout << "nfs_getattr: " << path << std::endl;
#endif
    (void)fi;
    NetFS *fs = (NetFS *)fuse_get_context()->private_data;
    return -fs->stat(path, *stbuf);
}

static int nfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info *fi,
                       enum fuse_readdir_flags flags)
{
#ifndef NDEBUG
    std::cout << "nfs_readdir: " << path << std::endl;
#endif
    (void)offset;
    (void)fi;
    (void)flags;
    NetFS *fs = (NetFS *)fuse_get_context()->private_data;

    std::vector<std::string> dirs;
    int err = fs->readdir(path, dirs);
    if (err != 0)
    {
        return -err;
    }

    for (const auto &d : dirs)
    {
        filler(buf, d.c_str(), NULL, 0, (fuse_fill_dir_flags)0);
    }
    return 0;
}

static int nfs_read(const char *path, char *buf, size_t size, off_t offset,
                    struct fuse_file_info *fi)
{
#ifndef NDEBUG
    std::cout << "nfs_read: " << path << ", offset: " << offset
              << ", size: " << size << std::endl;
#endif
    (void)fi;
    NetFS *fs = (NetFS *)fuse_get_context()->private_data;
    size_t read_size;
    int err = fs->read(path, offset, size, buf, read_size);
    if (err != 0)
    {
        return -err;
    }
    return read_size;
}

static int nfs_write(const char *path, const char *buf, size_t size,
                     off_t offset, struct fuse_file_info *fi)
{
#ifndef NDEBUG
    std::cout << "nfs_write: " << path << ", offset: " << offset
              << ", size: " << size << std::endl;
#endif
    (void)fi;
    NetFS *fs = (NetFS *)fuse_get_context()->private_data;
    int err = fs->write(path, offset, buf, size);
    if (err != 0)
    {
        return -err;
    }
    else
    {
        return size;
    }
}

int nfs_open(const char *path, struct fuse_file_info *fi)
{
#ifndef NDEBUG
    std::cout << "nfs_open" << path << std::endl;
#endif
    (void)fi;
    NetFS *fs = (NetFS *)fuse_get_context()->private_data;
    int err = fs->access(path);
    if (err == 0 && (fi->flags & O_TRUNC))
    {
        err = fs->truncate(path, 0);
    }
    return -err;
}
int nfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
#ifndef NDEBUG
    std::cout << "nfs_create" << std::endl;
#endif
    (void)fi;
    (void)mode;
    NetFS *fs = (NetFS *)fuse_get_context()->private_data;
    int err = fs->create(path);
    return -err;
}

int nfs_truncate(const char *path, off_t offset, struct fuse_file_info *fi)
{
#ifndef NDEBUG
    std::cout << "nfs_truncate" << std::endl;
#endif
    (void)fi;
    NetFS *fs = (NetFS *)fuse_get_context()->private_data;
    int err = fs->truncate(path, offset);
    return -err;
}

int nfs_unlink(const char *path)
{
#ifndef NDEBUG
    std::cout << "nfs_unlink" << std::endl;
#endif
    NetFS *fs = (NetFS *)fuse_get_context()->private_data;
    int err = fs->unlink(path);
    return -err;
}

int nfs_rmdir(const char *path)
{
#ifndef NDEBUG
    std::cout << "nfs_rmdir" << std::endl;
#endif
    NetFS *fs = (NetFS *)fuse_get_context()->private_data;
    int err = fs->rmdir(path);
    return -err;
}

int nfs_mkdir(const char *path, mode_t mode)
{
#ifndef NDEBUG
    std::cout << "nfs_mkdir" << std::endl;
#endif
    (void)mode;
    NetFS *fs = (NetFS *)fuse_get_context()->private_data;
    int err = fs->mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);
    return -err;
}

int nfs_flush(const char *path, struct fuse_file_info *)
{
#ifndef NDEBUG
    std::cout << "nfs_flush" << std::endl;
#endif
    NetFS *fs = (NetFS *)fuse_get_context()->private_data;
    int err = fs->flush(path);
    return -err;
}

int nfs_statfs(const char *path, struct statvfs *buf)
{
#ifndef NDEBUG
    std::cout << "nfs_statfs" << std::endl;
#endif
    (void)path;
    NetFS *fs = (NetFS *)fuse_get_context()->private_data;
    int err = fs->statfs(*buf);
    return -err;
}

int nfs_chmod(const char *, mode_t, struct fuse_file_info *)
{
#ifndef NDEBUG
    std::cout << "nfs_chmod (no op)" << std::endl;
#endif

    return 0;
}

int nfs_chown(const char *, uid_t, gid_t, struct fuse_file_info *)
{
#ifndef NDEBUG
    std::cout << "nfs_chown (no op)" << std::endl;
#endif
    return 0;
}

int nfs_rename(const char *from, const char *to, unsigned int flags)
{
#ifndef NDEBUG
    std::cout << "nfs_rename" << std::endl;
#endif
    NetFS *fs = (NetFS *)fuse_get_context()->private_data;
    int err = fs->rename(from, to, flags);
    return -err;
}

static struct fuse_operations nfs_oper;

static void show_help(const char *progname)
{
    printf("usage: %s [options] <mountpoint>\n\n", progname);
    printf(
        "File-system specific options:\n"
        "    --hostname=<s>              server hostname\n"
        "    --port=<s>                  server port number\n"
        "    --block_size=<i>            cache block size (in KB)\n"
        "    --cache_size=<i>             cache size (in MB)\n"
        "    --evict_count=<i>           number of blocks to evict when "
        "cache is full\n"
        "    --flush_interval=<i>        flush interval (in number of "
        "writes)\n"
        "\n");
}

int main(int argc, char *argv[])
{
    nfs_oper.getattr = nfs_getattr;
    nfs_oper.read = nfs_read;
    nfs_oper.write = nfs_write;
    nfs_oper.readdir = nfs_readdir;
    nfs_oper.init = nfs_init;
    nfs_oper.create = nfs_create;
    nfs_oper.open = nfs_open;
    nfs_oper.truncate = nfs_truncate;
    nfs_oper.unlink = nfs_unlink;
    nfs_oper.rmdir = nfs_rmdir;
    nfs_oper.mkdir = nfs_mkdir;
    nfs_oper.flush = nfs_flush;
    nfs_oper.statfs = nfs_statfs;
    nfs_oper.chmod = nfs_chmod;
    nfs_oper.chown = nfs_chown;
    nfs_oper.rename = nfs_rename;

    int ret;
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

    /* Set defaults -- we have to use strdup so that
       fuse_opt_parse can free the defaults if other
       values are specified */
    options.hostname = strdup("localhost");
    options.port = strdup("55555");
    options.block_size = strdup("");
    options.evict_count = strdup("");
    options.cache_size = strdup("");
    options.flush_interval = strdup("");

    /* Parse options */
    if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1) return 1;

    /* When --help is specified, first print our own file-system
       specific help text, then signal fuse_main to show
       additional help (by adding `--help` to the options again)
       without usage: line (by setting argv[0] to the empty
       string) */
    if (options.show_help)
    {
        show_help(argv[0]);
        assert(fuse_opt_add_arg(&args, "--help") == 0);
        args.argv[0] = (char *)"";
    }

    ret = fuse_main(args.argc, args.argv, &nfs_oper, NULL);

    // free fails when -h, don't know why
    // fuse_opt_free_args(&args);
    return ret;
}
