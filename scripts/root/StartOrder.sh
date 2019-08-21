#!/bin/bash
export LD_LIBRARY_PATH=/usr/local/lib

cd /today
/today/bin/ctporder >> /today/log/order.log 2>&1 &!
/today/bin/order_proxy 2>&1 &!
