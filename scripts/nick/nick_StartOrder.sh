#!/bin/sh
export LD_LIBRARY_PATH=/usr/local/lib

cd ~/today
~/today/bin/ctporder >> ~/today/log/order.log &!
~/today/bin/order_proxy &!
