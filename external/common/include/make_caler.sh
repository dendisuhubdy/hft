#!/bin/bash
g++ -fPIC -shared -std=c++11 -lboost_python -I/usr/include/python2.7 -I/root/anaconda2/include/python2.7 -o caler.so caler.cpp -lconfig++ -lcommontools
cp caler.so ~/hft/external/common/lib/cpp_py
