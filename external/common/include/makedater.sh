#!/bin/bash

g++ -std=c++11 testdater.cpp Dater.cpp -lboost_date_time -lcommontools -lconfig++
