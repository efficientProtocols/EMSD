#!/bin/csh
# RCS Version: $Id: run.invoker.sh,v 1.2 2002/10/25 19:37:38 mohsen Exp $

set SYSENV=${CURENVBASE}/results/systems/`uname -n`
set BINDIR=${CURENVBASE}/results/arch/`arch`
echo SYSENV=${SYSENV}

#$BINDIR/bin/invoker.one -l 14 -r 15 -p 2002 -n 192.168.0.5 -o ${SYSENV}/log/stressio.log -e ${SYSENV}/log/stressie.log -T G_,ffff  -T ESRO_,ffff  -T IMQ_,ffff LOPS_,ffff -T EROP_,ffff -T FSM_,01 -T UDP_,ffff  #-T SCH_,01 -T DU_,0400

$BINDIR/bin/invoker.one -l 14 -r 15 -p 2002 -n 192.168.0.25  -T G_,ffff  -T ESRO_,ffff  -T IMQ_,ffff LOPS_,ffff -T EROP_,ffff -T FSM_,01 -T UDP_,ffff  #-T SCH_,01 -T DU_,0400
