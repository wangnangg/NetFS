#!/bin/bash
set -e
root=..
int_test=.
make -C ${root}
mkdir -p ${int_test}/tmp
mkdir -p ${int_test}/nfs_root
${root}/build/debug/server 2>&1 | tee server_out
