#!/bin/csh
# RCS Version: $Id: run.invoker.sh,v 1.2 2002/10/25 19:37:35 mohsen Exp $

set SYSENV=${CURENVBASE}/results/systems/`uname -n`
set BINDIR=${CURENVBASE}/results/arch/`arch`

$BINDIR/bin/invoker -l 12 -r 13 -p 2002 -s /tmp/SP -n 192.168.0.14 -T G_,ffff  -T ESRO_,ffff  -T IMQ_,ffff # -T SCH_,ffff -T FSM_,ffff 
