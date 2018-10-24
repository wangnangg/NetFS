#!/bin/bash
set -e
config=${2:-debug}
filter=${1:-*}

python3 genmake.py
echo make build/${config}/utest config=${config}
make build/${config}/utest config=${config}

echo varlgrinding unit tests
valgrind --error-exitcode=1 ./build/${config}/utest --gtest_filter="$filter"
