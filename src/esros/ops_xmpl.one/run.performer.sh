#!/bin/csh
# RCS Version: $Id: run.performer.sh,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $

set SYSENV=${CURENVBASE}/results/systems/`uname -n`
set BINDIR=${CURENVBASE}/results/arch/`arch`
echo SYSENV=${SYSENV}

# $BINDIR/bin/performer.one -l 15 

#$BINDIR/bin/performer.one -l 15 -o ${SYSENV}/log/stresspo.log -e ${SYSENV}/log/stresspe.log -T G_,ffff -T ESRO_,ffff  -T LOPS_,ffff -T EROP_,ffff -T FSM_,01 -T UDP_,ffff    #-T SCH_,01 -T DU_,0400

$BINDIR/bin/performer.one -l 15 -T G_,ffff -T ESRO_,ffff -T LOPS_,ffff -T EROP_,ffff -T FSM_,01 -T UDP_,ffff  -T DU_,0400 # -T SCH_,01

