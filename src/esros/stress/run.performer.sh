#!/bin/csh
# RCS Version: $Id: run.performer.sh,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $

set SYSENV=${CURENVBASE}/results/systems/`uname -n`
set BINDIR=${CURENVBASE}/results/arch/`arch`

echo SYSENV=${SYSENV}

# ${BINDIR}/bin/stress_p -l 15 -s /tmp/SP -T G_,ffff  -T ESRO_,ffff  -T IMQ_,ffff 
${BINDIR}/bin/stress_p -l 15 -s /tmp/SP -t
