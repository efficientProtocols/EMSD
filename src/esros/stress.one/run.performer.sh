#!/bin/csh
# RCS Version: $Id: run.performer.sh,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $

set SYSENV=${CURENVBASE}/results/systems/`uname -n`
set BINDIR=${CURENVBASE}/results/arch/`arch`

echo SYSENV=${SYSENV}

${BINDIR}/bin/stress_p.one -c ${SYSENV}/config/erop_p.ini -l 15 -f 3 -g ${SYSENV}/log/stress_po_cbone.log -h ${SYSENV}/log/stress_pe_cbone.log -T G_,ffff # -T ESRO_,ffff -T EROP_,ffff
