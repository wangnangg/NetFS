#!/bin/bash
set -e
make -C ..
mkdir -p ./tmp
mkdir -p ./nfs_root

tmux \
  new-session  "../build/debug/server 2>&1 | tee server_out" \; \
  split-window "sleep 1.0; gdb -q -ex 'run -s -f -o auto_unmount ./tmp' ../build/debug/client"  \; \
  split-window "read; cd ./tmp; bash" \; \
  select-layout even-vertical
