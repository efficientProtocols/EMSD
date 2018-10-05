#!/bin/ksh

#
# RCS Revision: $Id: buildall.sh,v 1.6 2002/03/22 08:30:56 mohsen Exp $
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

for i in ${mkpFlavor}
do
  echo $i
  case ${i} in
    'ocp+sol2+gcc.sh')
      #
      buildAndRecord ${HERE}/sf_/unix "${MKP}"
      buildAndRecord ${HERE}/os/sol2  "${MKP}"
      buildAndRecord ${HERE}/sch_/unix "${MKP}"
      buildAndRecord ${HERE}/tmr_/unix.djl "${MKP}"
      buildAndRecord ${HERE}/outstrm_/outfile "${MKP}"
      buildAndRecord ${HERE}/nvm_/unix "${MKP}"
      ;;
    'ocp+linux+gcc.sh')
      #
      buildAndRecord ${HERE}/sf_/unix "${MKP}"
      buildAndRecord ${HERE}/os/linux  "${MKP}"
      buildAndRecord ${HERE}/sch_/unix "${MKP}"
      buildAndRecord ${HERE}/tmr_/unix.djl "${MKP}"
      buildAndRecord ${HERE}/outstrm_/outfile "${MKP}"
      buildAndRecord ${HERE}/nvm_/unix "${MKP}"
      ;;
    'ocp+dos+bc45.sh')
      ;;
    'os-winnt.sh')
      buildAndRecord ${HERE}/sf_/unix "${MKP}"
      buildAndRecord ${HERE}/os/nt-4.0  "${MKP}"
      buildAndRecord ${HERE}/os/win32/diraccess  "${MKP}"
      buildAndRecord ${HERE}/sch_/unix "${MKP}"
      #buildAndRecord ${HERE}/tmr_/wce-1.0 "${MKP}"
      buildAndRecord ${HERE}/tmr_/winnt4 "${MKP}"
      buildAndRecord ${HERE}/outstrm_/outfile "${MKP}"
      buildAndRecord ${HERE}/nvm_/unix "${MKP}"
      ;;
    'os-wince.sh')
      buildAndRecord ${HERE}/sf_/unix "${MKP}"
      buildAndRecord ${HERE}/os/wce-1.0  "${MKP}"
      buildAndRecord ${HERE}/sch_/unix "${MKP}"
      buildAndRecord ${HERE}/tmr_/wce-1.0 "${MKP}"
      buildAndRecord ${HERE}/outstrm_/outfile "${MKP}"
      buildAndRecord ${HERE}/nvm_/unix "${MKP}"
      ;;
    'os-win95.sh')
      buildAndRecord ${HERE}/sf_/unix "${MKP}"
      buildAndRecord ${HERE}/os/wce-1.0  "${MKP}"
      buildAndRecord ${HERE}/sch_/unix "${MKP}"
      buildAndRecord ${HERE}/tmr_/wce-1.0 "${MKP}"
      buildAndRecord ${HERE}/outstrm_/outfile "${MKP}"
      buildAndRecord ${HERE}/nvm_/unix "${MKP}"
      ;;
    'win32.sh')
      ;;
    'ocp+ae+bc45.sh')   
      ;;
    'purify')
      echo "NOTYET"
      exit
      ;;
    'os-winnt.sh'|'os-wince.sh'|'os-win95.sh')
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

