#!/bin/ksh

#
# RCS Revision: $Id: buildall.sh,v 1.9 1999/09/25 21:08:18 mohsen Exp $
#

. ${CURENVBASE}/tools/ocp-lib.sh

. ${CURENVBASE}/tools/buildallhead.sh


###### DO NOT EDIT ABOVE THIS LINE ######

HERE=`pwd`

TM_trace 9 ${MKP}

buildAndRecord ${HERE}/base.ocp "${buildAllCmd}"
buildAndRecord ${HERE}/net.ocp "${buildAllCmd}"
buildAndRecord ${HERE}/sysif.ocp "${buildAllCmd}"


exit 0

