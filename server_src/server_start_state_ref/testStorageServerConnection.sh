#!/bin/bash

trap "kill 0" EXIT

echo "./storageServer > serverlogs.txt &"
./storageServer > serverlogs.txt &
echo "Running connection to server test 10 times"

for(( i=0; i < 10; i++ ))
do
    echo "echo 'Hello Storage Server. Please response to me $[i].' | nc localhost 55555"
    echo "Hello Storage Server. Please response to me $[i]." | nc localhost 55555

done

