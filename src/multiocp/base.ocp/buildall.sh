#!/bin/ksh

#
# RCS Revision: $Id: buildall.sh,v 1.5 2002/03/22 08:30:52 mohsen Exp $
#

. ${CURENVBASE}/tools/ocp-lib.sh

. ${CURENVBASE}/tools/buildallhead.sh


###### DO NOT EDIT ABOVE THIS LINE ######


HERE=`pwd`

TM_trace 9 ${MKP}

#
# First Build everything that is common and portable
#


buildAndRecord ${HERE}/pf_/portable "${MKP}"
buildAndRecord ${HERE}/getopt/portable "${MKP}"
buildAndRecord ${HERE}/alloc/hosted "${MKP}"
buildAndRecord ${HERE}/buf/hosted "${MKP}"
buildAndRecord ${HERE}/cfg/hosted "${MKP}"
buildAndRecord ${HERE}/eh_/unix "${MKP}"
buildAndRecord ${HERE}/pn_/unix "${MKP}"
buildAndRecord ${HERE}/profile/hosted "${MKP}"
buildAndRecord ${HERE}/qu_/portable "${MKP}"
buildAndRecord ${HERE}/seq_/hosted "${MKP}"
buildAndRecord ${HERE}/str/hosted "${MKP}"
buildAndRecord ${HERE}/log_/ontopof "${MKP}"
buildAndRecord ${HERE}/bs_/portable "${MKP}"
buildAndRecord ${HERE}/fsm_/portable "${MKP}"
buildAndRecord ${HERE}/lic_/hosted "${MKP}"
buildAndRecord ${HERE}/sbm_/hosted "${MKP}"

buildAndRecord ${HERE}/mm_/hosted "${MKP}"
# ./mm_/hosted


buildAndRecord ${HERE}/toEng/portable "${MKP}"

#
# Now do all the Flavor specific work
#

#
# Do all the Flavor specific work
#

for i in ${mkpFlavor}
do
  case ${i} in
    'ocp+sol2+gcc.sh')
      buildAndRecord ${HERE}/tm_/hosted "${MKP}"
      ;;
    'ocp+linux+gcc.sh')
      buildAndRecord ${HERE}/tm_/hosted "${MKP}"
      ;;
    'ocp+dos+bc45.sh')
      ;;
    'os-winnt.sh')
      buildAndRecord ${HERE}/tm_/hosted "${MKP}"
      ;;
    'os-wince.sh')
      buildAndRecord ${HERE}/tm_/wce "${MKP}"
      buildAndRecord ${HERE}/vsprintf/portable "${MKP}"
      ;;
    'os-win95.sh')
      ;;
    'win32.sh')
      ;;
    'ocp+ae+bc45.sh')   
      ;;
    'purify')
      echo "NOTYET"
      exit
      ;;
    'cpu-x86.sh'|'cpu-mips.sh'|'cpu-sh3.sh')
      ;;
    'pdt-uadevl.sh' | 'pdt-vorde.sh' | 'pdt-current.sh')
      ;;
    *)
      EH_oops
      exit 1
      ;;
  esac
done

exit 0
