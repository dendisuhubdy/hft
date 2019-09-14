#!/bin/bash

cd /today
./bin/backtest > backtest.log 2>&1 &!
cp order_backtest.dat mid_backtest.dat /today
