#!/bin/ksh

#
# RCS Revision: $Id: buildall.sh,v 1.1.1.1 1999/09/19 21:59:31 mohsen Exp $
#

. ${CURENVBASE}/tools/ocp-lib.sh

. ${CURENVBASE}/tools/buildallhead.sh


###### DO NOT EDIT ABOVE THIS LINE ######

HERE=`pwd`

echo ${MKP}

TM_trace 9 ${MKP}


#
# First Build everything that is common and portable
#

#
# Do all the Flavor specific work
#
      buildAndRecord ${HERE} "${MKP}"

exit 0

