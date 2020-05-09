#/bin/bash
cd /root/hft
for f in `find . -name "*.h" -or -name "*.cpp" -or -name "*.hpp"`
do
	echo handling $f
        sed -i "s/util\/sender.hpp/util\/zmq_sender.hpp/g" $f
        sed -i "s/util\/recver.hpp/util\/zmq_recver.hpp/g" $f
        sed -i "s/ Recver/ ZmqRecver/g" $f
        sed -i "s/ Sender/ ZmqSender/g" $f
done
