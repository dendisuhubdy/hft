cpp = g++ -std=c++11
makelib = -fPIC -shared
include = -I /root/lib-hft/include/util -I /root/lib-hft/include/struct -I /root/lib-hft/include/core -I /root/lib-hft/include/
#link = -lconfig++ -lzmq -lboost_date_time -lboost_python -lcommun -ldater -ltimecontroller -lcaler -lcommontool -lcontractor -lhistoryworker -ltimer 
link=-lzmq -lconfig++ -lz -lnick

all:
	$(cpp) $(include) main_backtest.cpp /root/hft/src/simplearb/strategy.cpp $(link)
