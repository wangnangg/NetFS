# FUSE Info

## Operations

From docs:

The file system operations:

Most of these should work very similarly to the well known UNIX file system operations.  A major exception is that instead of returning an error in `errno`, the operation should return the negated error value (`-errno`) directly.

All methods are optional, but some are essential for a useful filesystem (e.g. `getattr`).  `open`, `flush`, `release`, `fsync`, `opendir`, `releasedir`, `fsyncdir`, `access`, `create`, `truncate`, `lock`, `init` and `destroy` are special purpose methods, without which a full featured filesystem can still be implemented.

In general, all methods are expected to perform any necessary permission checking. However, a filesystem may delegate this task to the kernel by passing the `default_permissions` mount option to `fuse_new()`. In this case, methods will only be called if the kernel's permission check has succeeded.

Almost all operations take a path which can be of any length.

### struct fuse_operations

| Name          | Arguments                              | comment                                                      | need?                    |
| ------------- | -------------------------------------- | ------------------------------------------------------------ | ------------------------ |
| `getattr`     | `path, stat, fi`                       | `stat` is the output; contains mode, length, etc.            | :heavy_check_mark:       |
| `readlink`    | `path`, `buf`, `buf_size`              | `buf` is filled with link target                             | :black_circle:           |
| `mknod`       | `path`, `mode`, `dev`                  | create special files like char, block, etc.                  | :heavy_multiplication_x: |
| `mkdir`       | `path`, `mode`                         | create a directory                                           | :heavy_check_mark:       |
| `unlink`      | `path`                                 | remove a file                                                | :heavy_check_mark:       |
| `rmdir`       | `path`                                 | rm a directory                                               | :heavy_check_mark:       |
| `symlink`     | `target`, `linkpath`                   | create a symbolic link                                       | :black_circle:           |
| `rename`      | `oldpath`,`newpath`                    | rename                                                       | :heavy_check_mark:       |
| `link`        | `oldpath`, `newpath`                   | create a hard link                                           | :black_circle:           |
| `chmod`       | `path`, `mode`, `fi`                   | chmod                                                        | :heavy_multiplication_x: |
| `chown`       | `path`, `uid`, `gid`, `fi`             | chown                                                        | :heavy_multiplication_x: |
| `truncate`    | `path`, `size`, `fi`                   | change file size                                             | :heavy_check_mark:       |
| `open`        | `path`, `fi`                           | see [docs](#open)                                            | :heavy_check_mark:       |
| `read`        | `path`, `buf`, `size`, `offset`,`fi`   | read                                                         | :heavy_check_mark:       |
| `write`       | `path`,`buf`,`size`,`fi`               | write                                                        | :heavy_check_mark:       |
| `statfs`      | `path`, `statvfs`                      | info of this filesystem                                      | :black_circle:           |
| `flush`       | `path`, `fi`                           | called before close. see [docs](#flush)                      | :heavy_check_mark:       |
| `release`     | `path`, `fi`                           | called when no open fd for a file exists                     | :heavy_check_mark:       |
| `fsync`       | `path`, `?`, `fi`                      | write back dirty data                                        | :heavy_check_mark:       |
| `setxattr`    | ...                                    | extended attr                                                | :heavy_multiplication_x: |
| `getxattr`    | ...                                    |                                                              | :heavy_multiplication_x: |
| `listxattr`   | ...                                    |                                                              | :heavy_multiplication_x: |
| `removexattr` | ...                                    |                                                              | :heavy_multiplication_x: |
| `opendir`     | `path`, `fi`                           | open directory                                               | :heavy_check_mark:       |
| `readdir`     | `path`, `buf`, `filler`, `fi`, `flags` | read directory entries                                       | :heavy_check_mark:       |
| `releasedir`  | `path`, `fi`                           | release directory                                            | :heavy_check_mark:       |
| `fsyncdir`    | `path`, `?`, `fi`                      | write back dirty data                                        | :heavy_check_mark:       |
| `init`        | `connection`,`config`                  | a chance to pass private data to context                     | :heavy_check_mark:       |
| `destroy`     | `private_data`                         | filesystem exits                                             | :heavy_check_mark:       |
| `access`      | `path`, `?`                            | check permission (not called if we let kernel handles permissions) | :heavy_multiplication_x: |
| `create`      | `path`, `mode`, `fi`                   | create and open                                              | :heavy_check_mark:       |
| `lock`        | `path`, `fi`, `cmd`, `flock`           | lock file                                                    | :black_circle:           |
| `utimens`     | `path`, `time`, `fi`                   | change access and modification time                          | :black_circle:           |
| `bmap`        | ...                                    | block mapping                                                | :heavy_multiplication_x: |
| `ioctl`       | ...                                    | the ultimate cmd                                             | :heavy_multiplication_x: |
| `poll`        | ...                                    | poll io events                                               | :heavy_multiplication_x: |
| `write_buf`   | ...                                    | similar to write                                             | :heavy_multiplication_x: |
| `read_buf`    | ...                                    | similar to read                                              | :heavy_multiplication_x: |
| `flock`       | ...                                    | bsd locking                                                  | :black_circle:           |
| `fallocate`   | ...                                    | reserve space for files                                      | :heavy_multiplication_x: |

### Explanation from Docs

#### open

Open a file

Open flags are available in fi->flags. The following rules apply.

- Creation (O_CREAT, O_EXCL, O_NOCTTY) flags will be filtered out / handled by the kernel.
- Access modes (O_RDONLY, O_WRONLY, O_RDWR) should be used by the filesystem to check if the operation is permitted. If the `-o default_permissions` mount option is given, this check is already done by the kernel before calling [open()](http://libfuse.github.io/doxygen/structfuse__operations.html#a14b98c3f7ab97cc2ef8f9b1d9dc0709d) and may thus be omitted by the filesystem.
- When writeback caching is enabled, the kernel may send read requests even for files opened with O_WRONLY. The filesystem should be prepared to handle this.
- When writeback caching is disabled, the filesystem is expected to properly handle the O_APPEND flag and ensure that each write is appending to the end of the file.
- When writeback caching is enabled, the kernel will handle O_APPEND. However, unless all changes to the file come through the kernel this will not work reliably. The filesystem should thus either ignore the O_APPEND flag (and let the kernel handle it), or return an error (indicating that reliably O_APPEND is not available).

Filesystem may store an arbitrary file handle (pointer, index, etc) in fi->fh, and use this in other all other file operations (read, write, flush, release, fsync).

Filesystem may also implement stateless file I/O and not store anything in fi->fh.

There are also some flags (direct_io, keep_cache) which the filesystem may set in fi, to change the way the file is opened. See [fuse_file_info](http://libfuse.github.io/doxygen/structfuse__file__info.html) structure in <fuse_common.h> for more details.

If this request is answered with an error code of ENOSYS and FUSE_CAP_NO_OPEN_SUPPORT is set in `fuse_conn_info.capable`, this is treated as success and future calls to open will also succeed without being send to the filesystem process.

#### flush

Possibly flush cached data

BIG NOTE: This is not equivalent to [fsync()](http://libfuse.github.io/doxygen/structfuse__operations.html#a92bdd6f43ba390a54ac360541c56b528). It's not a request to sync dirty data.

Flush is called on each close() of a file descriptor. So if a filesystem wants to return write errors in close() and the file has cached dirty data, this is a good place to write back data and return any errors. Since many applications ignore close() errors this is not always useful.

NOTE: The [flush()](http://libfuse.github.io/doxygen/structfuse__operations.html#ad4ec9c309072a92dd82ddb20efa4ab14) method may be called more than once for each [open()](http://libfuse.github.io/doxygen/structfuse__operations.html#a14b98c3f7ab97cc2ef8f9b1d9dc0709d). This happens if more than one file descriptor refers to an opened file due to dup(), dup2() or fork() calls. It is not possible to determine if a flush is final, so each flush should be treated equally. Multiple write-flush sequences are relatively rare, so this shouldn't be a problem.

Filesystems shouldn't assume that flush will always be called after some writes, or that if will be called at all.



### Summary

Operations we must implement:

| Name         | Priority |
| ------------ | -------- |
| `getattr`    | 1        |
| `mkdir`      | 2        |
| `unlink`     | 2        |
| `rmdir`      | 2        |
| `rename`     | 2        |
| `truncate`   | 2        |
| `create`     | 2        |
| `open`       | 1        |
| `read`       | 1        |
| `write`      | 1        |
| `flush`      | 1        |
| `fsync`      | 2        |
| `release`    | 2        |
| `opendir`    | 1        |
| `readdir`    | 1        |
| `releasedir` | 1        |
| `fsyncdir`   | 2        |
| `init`       | 1        |
| `destroy`    | 1        |

