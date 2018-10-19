#!/bin/bash
set -e
root=..
int_test=.
make -C ${root}
mkdir -p ${int_test}/tmp

tmux \
  new-session  "${root}/build/debug/server" \; \
  split-window "sleep 0.5; ${root}/build/debug/client -f -o auto_unmount ${int_test}/tmp" \; \
  split-window "sleep 1.0; cd ${int_test}/tmp; bash" \; \
  select-layout even-vertical
