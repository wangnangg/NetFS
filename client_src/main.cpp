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
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <memory>
#include <stdexcept>
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
    const char *ipv4addr;
    const char *port;
    int show_help;
} options;

#define OPTION(t, p)                      \
    {                                     \
        t, offsetof(struct options, p), 1 \
    }
static const struct fuse_opt option_spec[] = {OPTION("--ip=%s", ipv4addr),
                                              OPTION("-i=%s", ipv4addr),
                                              OPTION("--port=%s", port),
                                              OPTION("-p=%s", port),
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
    // 16kb page, 300MB cache, 20MB write flush once
    size_t block_size = 1 << 14;
    size_t entry_count = 300 * (1 << 20) / block_size;
    size_t flush_interval = 20 * (1 << 20) / block_size;
    auto netfs = new NetFS(options.ipv4addr, options.port, block_size,
                           entry_count, flush_interval);
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
    int err = fs->open(path, fi->flags, 0);
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
    int err = fs->open(path, fi->flags, S_IRWXU | S_IRWXG | S_IRWXO);
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

static struct fuse_operations nfs_oper;

static void show_help(const char *progname)
{
    printf("usage: %s [options] <mountpoint>\n\n", progname);
    printf(
        "File-system specific options:\n"
        "    --ip=<s>\n"
        "    -i=<s>              server IPv4 address\n"
        "    --port=<s>\n"
        "    -p=<s>              server port number\n"
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

    int ret;
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

    /* Set defaults -- we have to use strdup so that
       fuse_opt_parse can free the defaults if other
       values are specified */
    options.ipv4addr = strdup("127.0.0.1");
    options.port = strdup("55555");

    /* Parse options */
    if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1) return 1;

    uint32_t ip;
    uint16_t port;
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
