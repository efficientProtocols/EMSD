#!/bin/csh
# RCS Version: $Id: run.invoker.sh,v 1.2 2002/10/25 19:37:41 mohsen Exp $

set SYSENV=${CURENVBASE}/results/systems/`uname -n`
set BINDIR=${CURENVBASE}/results/arch/`arch`

echo SYSENV=${SYSENV}


#${BINDIR}/bin/stress_i -l 14 -r 15 -p 2002 -d 1000 -t 20000 -s /tmp/SP -n 192.168.0.10 -o err1.log -T G_,ffff  -T ESRO_,ffff  -T IMQ_,ffff # -T SCH_,ffff

${BINDIR}/bin/stress_i -m 100 -l 14 -r 15 -p 2002 -d 2000 -t 20000 -s /tmp/SP -n 192.168.0.14 -o err1.log 

