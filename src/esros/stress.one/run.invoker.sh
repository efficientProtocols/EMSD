#!/bin/csh
# RCS Version: $Id: run.invoker.sh,v 1.2 2002/10/25 19:37:44 mohsen Exp $

set SYSENV=${CURENVBASE}/results/systems/`uname -n`
set BINDIR=${CURENVBASE}/results/arch/`arch`

echo SYSENV=${SYSENV}

${BINDIR}/bin/stress_i.one -c ${SYSENV}/config/erop_i.ini -l 14 -r 15 -p 2002 -d 3000 -t 20000 -n 192.168.0.25 -o ${SYSENV}/log/stress_i_stat_cbone.log -g ${SYSENV}/log/stress_io_cbone.log -h ${SYSENV}/log/stress_ie_cbone.log -v

