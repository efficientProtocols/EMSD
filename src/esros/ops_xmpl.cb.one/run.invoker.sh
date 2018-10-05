#!/bin/ksh
#
# RCS Version: $Id: run.invoker.sh,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $

BINDIR=${CURENVBASE}/results/winnt/x86/bin
#BINDIR=${CURENVBASE}/results/sol2/bin
#BINDIR=${CURENVBASE}/results/linux/bin
#BINDIR=.

TRACEOPTIONS="-T G_,ffff  -T ESRO_,ffff  -T IMQ_,ffff LOPS_,ffff -T EROP_,ffff -T FSM_,01 -T UDP_,ffff  -T SCH_,01 -T DU_,0400"

#TRACEOPTIONS=""

AfrasiabInra=192.168.0.144
FarhadInra=192.168.0.110
YankeeInra=192.168.0.105
JamshidInra=192.168.0.103
TehranInra=192.168.0.38

${BINDIR}/invoker.cb.one -c erop.ini -l 14 -r 15 -p 2002 -n ${TehranInra} ${TRACEOPTIONS}
#${BINDIR}/invoker.cb.one -c erop.ini -l 14 -r 15 -p 2002 -n 192.168.0.105 -o testPDUlog.010726.log ${TRACEOPTIONS}






