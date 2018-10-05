#!/bin/ksh

#
# RCS Revision: $Id: buildall.sh,v 1.1.1.1 2002/10/08 18:57:31 mohsen Exp $
#

. ${CURENVBASE}/tools/ocp-lib.sh

. ${CURENVBASE}/tools/buildallhead.sh

###### DO NOT TOUCH ANYTHING ABOVE THIS LINE ######

HERE=`pwd`

#
# Do all the Flavor specific work
#

	  buildAndRecord ${HERE}/erop_eng "${MKP}"
	  buildAndRecord ${HERE}/erop_sh "${MKP}"

buildAndRecord ${HERE}/prvdr "${MKP}"
buildAndRecord ${HERE}/prvdr_sh "${MKP}"
buildAndRecord ${HERE}/user_sh "${MKP}"
buildAndRecord ${HERE}/user_sh.cb "${MKP}"


	  #buildAndRecord ${HERE}/prvdr_sh "${MKP}"
  	  #buildAndRecord ${HERE}/user_sh "${MKP}"

	  buildAndRecord ${HERE}/ops_xmpl.cb "${MKP}"
	  buildAndRecord ${HERE}/pdu_log "${MKP}"
	  buildAndRecord ${HERE}/stress.cb "${MKP}"

	  #buildAndRecord ${HERE}/tester.one "${MKP}"


buildAndRecord ${HERE}/ops_xmpl "${MKP}"
buildAndRecord ${HERE}/ops_xmpl.cb.one "${MKP}"
buildAndRecord ${HERE}/ops_xmpl.one "${MKP}"
buildAndRecord ${HERE}/stress "${MKP}"
buildAndRecord ${HERE}/stress.cb.one "${MKP}"
buildAndRecord ${HERE}/stress.one "${MKP}"
buildAndRecord ${HERE}/tester "${MKP}"
