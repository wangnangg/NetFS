#!/bin/bash
set -e
root=..
int_test=.
make -C ${root}
mkdir -p ${int_test}/tmp
mkdir -p ${int_test}/nfs_root

tmux \
  new-session  "${root}/build/debug/server 2>&1 | tee server_out" \; \
  split-window "sleep 1.0; ${root}/build/debug/client -s -f -o auto_unmount ${int_test}/tmp 2>&1 | tee client_out" \; \
  split-window "read; cd ${int_test}/tmp; bash" \; \
  select-layout even-vertical
