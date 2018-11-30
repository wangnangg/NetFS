#!/bin/bash
set -e
host=${1:-localhost}
root=..
int_test=.
config=${2:-release}
make config=${config} -C ${root}
tmux \
  new-session "${root}/build/${config}/client --hostname=${host} --port=55555 -f -s -o auto_unmount ${int_test}/tmp 2>&1 | tee client_out" \; \
  split-window "read; cd ${int_test}/tmp; bash" \; \
  select-layout even-vertical
