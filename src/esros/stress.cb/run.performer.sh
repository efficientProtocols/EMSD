#!/bin/ksh
# RCS Version: $Id: run.performer.sh,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $

devlOsType=`uname -s`

case ${devlOsType} in
  'SunOS')
    BINDIR=${CURENVBASE}/results/sol2
    ;;
  'Windows_NT')
    BINDIR=${CURENVBASE}/results/winnt/x86
    ;;
  *)
    echo "$0 not Suported on ${devlOsType}"
    exit
esac

echo "BINDIR=${BINDIR}"

${BINDIR}/bin/stress_p.cb.one -c erop_p.ini -l 15 -f 3 -T G_,ffff  -T ESRO_,ffff -T EROP_,ffff

#${BINDIR}/bin/stress_p.one -c ${SYSENV}/config/erop_p.ini -l 15 -f 3 -g ${SYSENV}/log/stress_po_cbone.log -h ${SYSENV}/log/stress_pe_cbone.log -T G_,ffff # -T ESRO_,ffff -T EROP_,ffff
