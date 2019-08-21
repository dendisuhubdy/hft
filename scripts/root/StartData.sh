#!/bin/bash
export LD_LIBRARY_PATH=/usr/local/lib

cd /today
/today/bin/ctpdata >> /today/log/data.log 2>&1 &!
/today/bin/data_proxy 2>&1 &!
/today/bin/mid_data 2>&1 &!
