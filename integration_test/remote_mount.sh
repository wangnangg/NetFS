#!/bin/bash
set -e
root=..
int_test=.
config=release
make config=${config} -C ${root}
tmux \
  new-session "${root}/build/${config}/client --ip=vcm-6624.vm.duke.edu --port=55555 -f -d -s -o auto_unmount ${int_test}/tmp 2>&1 | tee client_out" \; \
  split-window "sleep 2.0; cd ${int_test}/tmp; bash" \; \
  select-layout even-vertical
