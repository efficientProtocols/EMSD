#!/bin/csh
# RCS Version: $Id: showLog.sh,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $
#

if ( "$1" == "" ) then 
echo Usage: "$0 {1|2}"
exit
endif

set SYSENV=${CURENVBASE}/results/systems/`uname -n`
set BINDIR=${CURENVBASE}/results/arch/`arch`

echo SYSENV=${SYSENV}

${BINDIR}/bin/eropscop -f ${SYSENV}/log/o$1.log

