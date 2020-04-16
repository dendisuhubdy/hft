#!/bin/bash

date=`date  +"%Y-%m-%d" -d  "-1 days"`
date=2020-04-10
echo $date
rsync -avz -e "ssh -i /root/.ssh/ali_key" -r root@101.132.173.17:/running/$date/future$date.dat.gz ~
