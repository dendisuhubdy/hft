#/bin/bash
cd /root/lib-hft
for f in `find . -name "*.h" -or -name "*.cpp" -or -name "*.hpp"`
do
	echo handling $f
        sed -i "s/data_sub/data_sender/g" $f
        sed -i "s/data_pub/data_recver/g" $f
        sed -i "s/order_sub/order_sender/g" $f
        sed -i "s/order_pub/order_recver/g" $f
done
