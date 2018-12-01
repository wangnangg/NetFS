#!/bin/bash
set -e
blocks="4 16 128"
caches="64 256 512"
flushes="1000 10000 100000"
for b in $blocks
do
    for c in $caches
    do
        for f in $flushes
        do
            export BLOCK_SIZE=$b
            export CACHE_SIZE=$c
            export FLUSH_INTERVAL=$f

            NAME=$BLOCK_SIZE-$CACHE_SIZE-$FLUSH_INTERVAL
            screen -L -Logfile client-$NAME.log -d -m ./remote_mount.sh
            iozone -f nfs_mount/iozone.tmp -i 0 -i 1 -i 2 -i 8 -g 1G -Rcea -b ./result-$NAME.wks
            fusermount3 -u ./nfs_mount
        done
    done
done

