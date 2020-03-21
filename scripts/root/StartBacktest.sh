#!/bin/bash

export LD_LIBRARY_PATH=/usr/local/lib

date=`date  +"%Y-%m-%d"`

cd ~/hft
make
cd ~/hft/build/bin
./backtest > backtest.log 2>&1

sleep 5
mv backtest_out/order_$date.dat order_backtest.dat
mv backtest_out/mid_$date.dat mid_backtest.dat
cp order_backtest.dat mid_backtest.dat /today
