#!/bin/bash

dd if=/dev/urandom | pv | dd of=./dd_out
