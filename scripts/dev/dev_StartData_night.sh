#!/bin/sh
export LD_LIBRARY_PATH=/usr/local/lib

cd ~/today
~/today/bin/ctpdata > ~/today/log/data_night.log &!
~/today/bin/data_proxy &!
~/today/bin/mid_data &!
