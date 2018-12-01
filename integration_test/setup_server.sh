#!/bin/bash
set -e
root=..
int_test=.
config=${1:-release}
make config=${config} -C ${root}
mkdir -p ${int_test}/tmp
mkdir -p ${int_test}/nfs_root
${root}/build/${config}/server 2>&1 | tee server_out
