#!/bin/csh
# RCS Version: $Id: run.esrossi.sh,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $
#

set SYSENV=${CURENVBASE}/results/systems/`uname -n`
set BINDIR=${CURENVBASE}/results/arch/`arch`

echo SYSENV=${SYSENV}

#${BINDIR}/bin/esrossi.one -c ${SYSENV}/config/erop.ini -d ${CURENVBASE}/results/data/esros/scenarios -T G_,ffff

#${BINDIR}/bin/esrossi.one -c ${SYSENV}/config/erop.ini -d ${CURENVBASE}/results/data/esros/scenarios

# /dos/u/neda/devenv/esrosFull.mul.yohanus/src/esros/scenarios/
${BINDIR}/bin/esrossi.one -c ${SYSENV}/config/erop.ini -d ${CURENVBASE}/src/esros/scenarios