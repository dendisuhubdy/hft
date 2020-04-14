#/bin/bash
#cd /root/lib-hft
cd /root/hft
for f in `find . -name "*.h" -or -name "*.cpp"`
do
	sed -i "s/FineOneValid(/FindOneValid(/g" $f
done
