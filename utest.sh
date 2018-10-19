#!/bin/bash
set -e
config=${2:-debug}
filter=${1:-*}

python3 genmake.py
echo make build/${config}/utest
make build/${config}/utest

echo ./build/${config}/utest --gtest_filter="$filter"
./build/${config}/utest --gtest_filter="$filter"
