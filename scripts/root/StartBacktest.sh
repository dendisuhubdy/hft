#!/bin/bash

export LD_LIBRARY_PATH=/usr/local/lib

cd ~/hft
make
cd ~/hft/build/bin
./backtest > backtest.log 2>&1 &!
mv order.dat order_backtest.dat
cp order_backtest.dat mid_backtest.dat /today
