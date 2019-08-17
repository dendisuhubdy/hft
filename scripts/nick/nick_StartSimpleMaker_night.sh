#!/bin/sh
export LD_LIBRARY_PATH=/usr/local/lib

cd ~/today
~/today/bin/simplemaker >> ~/today/log/simplemaker_night.log &!
