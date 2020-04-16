#!/bin/bash
export LD_LIBRARY_PATH=/usr/local/lib
ulimit -c unlimited

cd /today
/today/bin/simplearb >> /today/log/simplearb.log 2>&1 &!
