#!/bin/ksh
#
# RCS Version: $Id: run.performer.sh,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $


BINDIR=${CURENVBASE}/results/winnt/x86/bin
#BINDIR=${CURENVBASE}/results/sol2/bin
#BINDIR=${CURENVBASE}/results/linux/bin
#BINDIR=.

TRACEOPTIONS="-T G_,ffff  -T ESRO_,ffff  -T IMQ_,ffff LOPS_,ffff -T EROP_,ffff -T FSM_,01 -T UDP_,ffff  -T SCH_,01 -T DU_,0400"

#TRACEOPTIONS=""


${BINDIR}/performer.cb.one -c erop.ini -l 15 ${TRACEOPTIONS}
#${BINDIR}/performer.cb.one -c erop.ini -l 15 -o testPDUlog.010726.log ${TRACEOPTIONS}


