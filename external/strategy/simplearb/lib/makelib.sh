#!/bin/bash

g++ -std=c++11 -fPIC -shared -o libsimplearb.so ~/hft/src/simplearb/strategy.cpp -I ~/hft/external/common/include  -lcommontools -lzmq -lconfig++

sudo cp libsimplearb.so /usr/local/lib
