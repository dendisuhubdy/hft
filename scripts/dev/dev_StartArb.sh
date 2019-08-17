#!/bin/sh
export LD_LIBRARY_PATH=/usr/local/lib

cd ~/today
~/today/bin/arbmaker >> ~/today/log/arbmaker.log &!
