#!/bin/ksh

#
# RCS Revision: $Id: buildall.sh,v 1.2 1999/09/25 21:19:30 mohsen Exp $
#

. ${CURENVBASE}/tools/ocp-lib.sh

. ${CURENVBASE}/tools/buildallhead.sh


###### DO NOT EDIT ABOVE THIS LINE ######


HERE=`pwd`

TM_trace 9 ${MKP}

#
# First Build everything that is common and portable
#

buildAndRecord ${HERE}/genrelid "${MKP}"
buildAndRecord ${HERE}/cpright "${MKP}"
buildAndRecord ${HERE}/relidusr "${MKP}"

exit 0
