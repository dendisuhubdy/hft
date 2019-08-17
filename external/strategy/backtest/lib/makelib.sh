#!/bin/bash

g++ -std=c++11 -fPIC -shared -o libbacktest.so ~/hft/src/backtest/strategy.cpp -I ~/hft/external/common/include  -lcommontools -lzmq -lconfig++

sudo cp libbacktest.so /usr/local/lib
