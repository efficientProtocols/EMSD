#!/bin/ksh

#
# RCS Revision: $Id: buildall.sh,v 1.5 2002/02/12 23:37:40 pinneke Exp $
#

. ${CURENVBASE}/tools/ocp-lib.sh

. ${CURENVBASE}/tools/buildallhead.sh


###### DO NOT EDIT ABOVE THIS LINE ######


HERE=`pwd`

TM_trace 9 ${MKP}

#
# First Build everything that is common and portable
#

buildAndRecord ${HERE}/bo_/portable "${MKP}"
buildAndRecord ${HERE}/du_/unhosted "${MKP}"
buildAndRecord ${HERE}/inet/unix "${MKP}"
buildAndRecord ${HERE}/sap_/portable "${MKP}"
buildAndRecord ${HERE}/udp_pco "${MKP}"

buildAndRecord ${HERE}/asn "${MKP}"


case `uname`  in
  'SunOS')
    buildAndRecord ${HERE}/upq "${MKP}"
    ;;
  'Windows_NT')
    # NOTYET
    ;;
  *)
    #EH_oops
    #exit 1
    ;;
esac

exit 0
