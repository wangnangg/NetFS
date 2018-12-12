ECE590-020: Enterprise Storage Architecture -- Fall 2018

Course Project: Network File System with Caching

Group Number: 3

Group Members: Connor Grehlinger (cmg88), Nan Wang (nw83),
Jeff Lasser (jcl35), Noah Pritt (ncp14)


[Coverage Report](https://cmg88.pages.oit.duke.edu/ece590-020-storage-project-group3)




## Compile

### First time setup

Upon first git clone, `libfuse` , which resides in `fuse-3`, must be properly compiled. This is achieved by

```bash
./prepare.sh
```

Install POCO library for networking

```bash
sudo apt-get install libpoco-dev
```

Install pkg-config

```bash
sudo apt-get install pkg-config
```

Install tmux

```bash
sudo apt-get install tmux
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

`googletest` is a nice framework for unit testing. Source files of unit tests should be in `utest_src` directory. File `utest_src/example.cpp` illustrates how to write a unit test. Again, if new source files are added, then `./genmake.py` should be run to regenerate `makefile`. Then, unit tests can be built with `make utest` or simply `make`. An executable `build/debug/utest` (or `build/release/utest`, depending on the configuration) will be generated, executing which will run all unit tests.  Typical output will look like the following

```
./build/debug/utest
[==========] Running 2 tests from 1 test case.
[----------] Global test environment set-up.
[----------] 2 tests from example
[ RUN      ] example.test1
[       OK ] example.test1 (0 ms)
[ RUN      ] example.test2
[       OK ] example.test2 (0 ms)
[----------] 2 tests from example (0 ms total)

[----------] Global test environment tear-down
[==========] 2 tests from 1 test case ran. (0 ms total)
[  PASSED  ] 2 tests.
```

It is possible to only run selected tests by passing `--gtest_filter` to `build/debug/utest`. Details can be found by checking help info: `./build/debug/utest --help` or docs. 
