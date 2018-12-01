#!/bin/bash
set -e
host=${1:-localhost}
root=..
config=${2:-release}
make config=${config} -C ${root}
${root}/build/${config}/client --hostname=${host} --port=55555 --block_size=${BLOCK_SIZE} --cache_size=${CACHE_SIZE} --flush_interval=${FLUSH_INTERVAL} -f -s -o auto_unmount ./nfs_mount
