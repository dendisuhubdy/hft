#!/bin/sh
export LD_LIBRARY_PATH=/usr/local/lib

cd ~/today
~/today/bin/easy_strat >> ~/today/log/easy_strat_night.log &!
