#/bin/bash
#cd /root/lib-hft
cd /root/hft
for f in `find . -name "*.h" -or -name "*.cpp"`
do
	sed -i "s/strategy_container.h>/strategy_container.hpp>/g" $f
done
