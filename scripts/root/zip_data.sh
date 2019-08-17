#!/bin/sh
export LD_LIBRARY_PATH=/usr/local/lib

cd /today
gzip data_binary.dat

cd /today/log

gzip data.log
gzip data_night.log
