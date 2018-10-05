#!/bin/sh
#
# RCS: $Id: run.mailer.sh,v 1.2 2002/10/25 19:37:30 mohsen Exp $
#

echo EMSDBASE=${EMSDBASE}
echo EMSDRUN=${EMSDRUN}

RUNENV=${EMSDRUN}

CONFIG_DIR=${RUNENV}/config

echo CONFIG_DIR=${CONFIG_DIR}

${EMSDRUN}/bin/emsd-mailer -T IMQ_,ffff -T ESRO_,ffff -n 192.168.0.5 -a $1

echo ""
echo "To see the logfile, run:"
echo tail -f ${EMSDRUN}/log/emsd-mts.log

