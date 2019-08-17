#!/bin/bash
#g++ -std=c++11 testcon.cpp Contractor.cpp -lcommontools -lconfig++
g++ -fPIC -shared -std=c++11 -lboost_python -I/usr/include/python2.7 -I/root/anaconda2/include/python2.7 -o con.so Contractor.cpp -lconfig++ -lcommontools
cp con.so ~/hft/external/common/lib/cpp_py
