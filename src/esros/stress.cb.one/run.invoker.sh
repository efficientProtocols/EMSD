#!/bin/ksh
# RCS Version: $Id: run.invoker.sh,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $

devlOsType=`uname -s`
remoteIP="192.168.0.103"

#set SYSENV=${CURENVBASE}/results/systems/`uname -n`
#set BINDIR=${CURENVBASE}/results/arch/`arch`

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
#echo "remoteIP=${remoteIP}"

${BINDIR}/bin/stress_i.cb.one -c erop_i.ini -l 14 -r 15 -p 2002 -d 3000 -t 20000 -n ${remoteIP} -v -T G_,ffff  -T ESRO_,ffff -T EROP_,ffff

#${BINDIR}/bin/stress_i.one -c ${SYSENV}/config/erop_i.ini -l 14 -r 15 -p 2002 -d 3000 -t 20000 -n 192.168.0.105 -o ${SYSENV}/log/stress_i_stat_cbone.log -g ${SYSENV}/log/stress_io_cbone.log -h ${SYSENV}/log/stress_ie_cbone.log -v

