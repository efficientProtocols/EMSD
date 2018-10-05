#!/bin/ksh

#
# RCS Revision: $Id: buildall.sh,v 1.1.1.1 2002/10/24 19:50:03 mohsen Exp $
#

. ${CURENVBASE}/tools/ocp-lib.sh

. ${CURENVBASE}/tools/buildallhead.sh

###### DO NOT TOUCH ANYTHING ABOVE THIS LINE ######

HERE=`pwd`

#
# Do all the Flavor specific work
#



buildAndRecord ${HERE}/dup "${MKP}"
buildAndRecord ${HERE}/emsd "${MKP}"

buildAndRecord ${HERE}/emsdscope "${MKP}"
buildAndRecord ${HERE}/mtsua "${MKP}"

buildAndRecord ${HERE}/ua "${buildAllCmd}"

buildAndRecord ${HERE}/emsdmgr "${MKP}"
buildAndRecord ${HERE}/mailer "${MKP}"
buildAndRecord ${HERE}/mts "${MKP}"
buildAndRecord ${HERE}/mts.one "${MKP}"

#buildAndRecord ${HERE}/mts.one.old "${MKP}"

#buildAndRecord ${HERE}/uad "${MKP}"

