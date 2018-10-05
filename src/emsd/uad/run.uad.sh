#!/bin/sh
# p0 = child = listner
# ./emsd-uad -T IMQ_,ffff -T ESRO_,ffff -n 192.168.0.5 -a -X -p0 $1
 ./emsd-uad -T IMQ_,ffff -T ESRO_,ffff -n 192.168.0.5 -a $1
