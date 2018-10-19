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
#include "net.hpp"
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
    const char *filename;
    const char *contents;
    const char *ipv4addr;
    const char *port;
    int show_help;
} options;

#define OPTION(t, p)                      \
    {                                     \
        t, offsetof(struct options, p), 1 \
    }
static const struct fuse_opt option_spec[] = {
    OPTION("--name=%s", filename),
    OPTION("--contents=%s", contents),
    OPTION("--ip=%s", ipv4addr),
    OPTION("-i=%s", ipv4addr),
    OPTION("--port=%s", port),
    OPTION("-p=%s", port),
    OPTION("-h", show_help),
    OPTION("--help", show_help),
    FUSE_OPT_END};

static void *hello_init(struct fuse_conn_info *conn, struct fuse_config *cfg)
{
    (void)conn;
    cfg->kernel_cache = 0;
    auto netfs = new NetFS(options.ipv4addr, options.port);
    return netfs;
}

static int hello_getattr(const char *path, struct stat *stbuf,
                         struct fuse_file_info *fi)
{
    (void)fi;
    NetFS *fs = (NetFS *)fuse_get_context()->private_data;
    return -fs->stat(path, *stbuf);
}

static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi,
                         enum fuse_readdir_flags flags)
{
    (void)offset;
    (void)fi;
    (void)flags;
    NetFS *fs = (NetFS *)fuse_get_context()->private_data;

    std::vector<NetFS::Dirent> dirs;
    int res = fs->readdir(path, dirs);
    if (res != 0)
    {
        return -res;
    }

    for (const auto &d : dirs)
    {
        filler(buf, d.name.c_str(), NULL, 0, (fuse_fill_dir_flags)0);
    }
    return 0;
}

static int hello_open(const char *path, struct fuse_file_info *fi)
{
    NetFS *fs = (NetFS *)fuse_get_context()->private_data;
    int res = fs->open(path, fi->flags);
    return -res;
}

static int hello_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
{
    (void)fi;
    NetFS *fs = (NetFS *)fuse_get_context()->private_data;
    int res = fs->read(path, offset, buf, size);
    if (res != 0)
    {
        return -res;
    }
    return size;
}

static struct fuse_operations hello_oper = {
    .getattr = hello_getattr,
    .open = hello_open,
    .read = hello_read,
    .readdir = hello_readdir,
    .init = hello_init,
};

static void show_help(const char *progname)
{
    printf("usage: %s [options] <mountpoint>\n\n", progname);
    printf(
        "File-system specific options:\n"
        "    --name=<s>          Name of the \"hello\" file\n"
        "                        (default: \"hello\")\n"
        "    --contents=<s>      Contents \"hello\" file\n"
        "                        (default \"Hello, World!\\n\")\n"
        "    --ip=<s>\n"
        "    -i=<s>              server IPv4 address\n"
        "    --port=<s>\n"
        "    -p=<s>              server port number\n"
        "\n");
}

int main(int argc, char *argv[])
{
    int ret;
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

    /* Set defaults -- we have to use strdup so that
       fuse_opt_parse can free the defaults if other
       values are specified */
    options.filename = strdup("hello");
    options.contents = strdup("Hello World!\n");
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

    ret = fuse_main(args.argc, args.argv, &hello_oper, NULL);

    // free fails when -h, don't know why
    // fuse_opt_free_args(&args);
    return ret;
}
