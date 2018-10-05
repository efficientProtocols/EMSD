#!/bin/ksh

#
# RCS Revision: $Id: buildall.sh,v 1.2 2002/10/25 07:46:34 mohsen Exp $
#

. ${CURENVBASE}/tools/ocp-lib.sh

. ${CURENVBASE}/tools/buildallhead.sh

###### DO NOT TOUCH ANYTHING ABOVE THIS LINE ######

HERE=`pwd`

#
# Do all the Flavor specific work
#

# ./uashpine.one

buildAndRecord ${HERE}/uacore "${MKP}"

buildAndRecord ${HERE}/uashdvem.one "${MKP}"

buildAndRecord ${HERE}/uashpine.one "${MKP}"

exit 0
