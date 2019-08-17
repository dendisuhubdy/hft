#!/bin/bash
export LD_LIBRARY_PATH=/usr/local/lib

cd /today
/today/bin/ctpdata >> /today/log/data.log &!
/today/bin/data_proxy &!
/today/bin/mid_data &!
