#!/bin/bash

dd if=./dd_out | pv | dd of=/dev/null
