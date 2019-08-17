#!/bin/bash
export LD_LIBRARY_PATH=/usr/local/lib

cd /today
/today/bin/ctporder >> /today/log/order_night.log &!
/today/bin/order_proxy &!
