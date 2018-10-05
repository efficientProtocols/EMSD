#!/bin/ksh

#
# RCS Revision: $Id: buildall.sh,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $
#
. ${CURENVBASE}/tools/ocp-lib.sh

. ${CURENVBASE}/tools/buildallhead.sh


###### DO NOT EDIT ABOVE THIS LINE ######

HERE=`pwd`

echo ${MKP}

TM_trace 9 ${MKP}

BIN_PATH=.

#
# First Build everything that is common and portable
#

#
# Do all the Flavor specific work
#
      buildAndRecord ${HERE} "${MKP}"

exit 0

