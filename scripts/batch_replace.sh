#/bin/bash
cd /root/hft
for f in `find . -name "*.h" -or -name "*.cpp"`
do
	echo handling $f
	for i in `ls /root/lib-hft/include/struct`
	do
                sed -i "s/\"$i/\"struct\/$i/g" $f
                sed -i "s/<$i/<struct\/$i/g" $f
	done
done
