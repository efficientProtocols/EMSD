#!/bin/ksh
# RCS Version: $Id: run.esrossi.sh,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $
#

devlOsType=`uname -s`

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

#${BINDIR}/bin/esrossi -T G_,ffff
# ${BINDIR}/bin/esrossi -d ${CURENVBASE}/results/data/esros/scenarios -T G_,ffff -T ESRO_,ffff -T IMQ_,ffff  $*

# /dos/u/neda/devenv/FullNSE-MulPub/results/sol2/bin/
${BINDIR}/bin/esrossi -d ${CURENVBASE}/results/data/esros/scenarios -T G_,ffff 

