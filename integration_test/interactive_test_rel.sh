#!/bin/bash
set -e
root=..
int_test=.
make -C ${root} config=release
mkdir -p ${int_test}/tmp
mkdir -p ${int_test}/nfs_root

tmux \
  new-session  "${root}/build/release/server 2>&1 | tee server_out" \; \
  split-window "sleep 1.0; ${root}/build/release/client -s -f -o auto_unmount ${int_test}/tmp 2>&1 | tee client_out" \; \
  split-window "read; cd ${int_test}/tmp; bash" \; \
  select-layout even-vertical
