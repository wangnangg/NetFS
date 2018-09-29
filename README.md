ECE590-020: Enterprise Storage Architecture -- Fall 2018

Course Project: Network File System with Caching

Group Number: 3

Group Members: Connor Grehlinger (cmg88), Nan Wang (nw83),
Jeff Lasser (jcl35), Noah Pritt (ncp14)





## Compile

### First time setup

Upon first git clone, `libfuse` , which resides in `fuse-3`, must be properly compiled. This is achieved by

```bash
./prepare.sh
```

### After adding/removing source files or changing compilation flags

Writing `makefile` manually is a pain in the ass. `genmake.py` is a little script that generates `makefile` for us, by finding all source files in `client_src` directory and reading compilation flags from `makefile.in`.  Ideally, we do not need to touch `genmake.py`.  If files are added/removed or compilation flags are changed, `makefile` needs to be regenerated. This is accomplished by

```bash
./genmake.py
```

### After editing source files

After editing source files, binaries need to be rebuilt.

```bash
make
```

`makefile` actually comes with two set of configurations: `debug` and `release`. In latter stages, when measuring performance, we might need to compile with `release` configuration. This can be done by

```bash
make config=release
```

## Unit Test

`googletest` is a nice framework for unit testing. 