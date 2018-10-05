#!/bin/ksh

#
# RCS Revision: $Id: buildall.sh,v 1.3 2001/08/01 20:19:03 mohsen Exp $
#

. ${CURENVBASE}/tools/ocp-lib.sh

. ${CURENVBASE}/tools/buildallhead.sh

###### DO NOT TOUCH ANYTHING ABOVE THIS LINE ######

HERE=`pwd`

#
# Do all the Flavor specific work
#

buildAndRecord ${HERE}/multiocp "${buildAllCmd}"

buildAndRecord ${HERE}/kpathsea "${buildAllCmd}"

buildAndRecord ${HERE}/cpp "${buildAllCmd}"

buildAndRecord ${HERE}/this "${buildAllCmd}"

buildAndRecord ${HERE}/vorde "${buildAllCmd}"

buildAndRecord ${HERE}/esros "${buildAllCmd}"

buildAndRecord ${HERE}/mts_ua "${buildAllCmd}"

exit 0
