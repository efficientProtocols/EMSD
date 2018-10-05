#!/bin/ksh
#
# RCS Revision: $Id: kitPrep.sh,v 1.1 1999/11/15 00:25:55 mohsen Exp $
#

#
# Description:
#       Sample File for a do-some-stuff script
#
#  Example:
#
#       For WIN95/WINNT
#            ./doSomeStuff.sh -f os-winnt.sh -f win32.sh
#
#       For WINCE
#           doSomeStuff.sh -f os-wince -f cpu-mips  -f win32
#
#
#

#
# Author: Mohsen Banan.
#
#

#set -v

. ${CURENVBASE}/tools/ocp-lib.sh


#
# DEFAULTS
#
doAction="pre"
doActionValidList="pre install"


runOSValidList="SunOS Windows_NT"
#runOSValidList="Windows_NT"

#
# DEFAULTS
#
usage() {
    printf >&2 "Usage: $0 [ -T traceLevel ] [ -u ] [-f flavor] [-s status] [-a arch]\n"
    printf >&2 "Usage: flavor is one of: ${targetProdFlavorValidList}\n"
    printf >&2 "Usage: status is one of: ${targetProdStatusValidList}\n"
    printf >&2 "Usage: arch is one of: ${targetArchTypeValidList}\n"
    exit 1
}

systemName=`uname -n`
devlOsType=`uname -s`


###


#PATH=/usr/sbin:/usr/bin:/sbin

# Get options (defaults immediately below):

badUsage=
traceLevel=

targetConfigFiles=""

TM_trace 9 $*

while getopts d:f:T:u? c
do
    case $c in
    T)
	case $OPTARG in
	*[!0-9]* )
	    echo "$0: -T requires a numeric option"
 	    badUsage=TRUE
	    ;;
        [0-9]* )
	    traceLevel=$OPTARG
	    ;;
	esac
	;;
    f)
	targetConfigFiles="${targetConfigFiles} ${OPTARG}"
	;;
    d)
	doAction="${OPTARG}"
	;;
    u | \?)
	badUsage=TRUE
	;;
    esac
done

if [ "${targetConfigFiles}" != "" ]
then
  for i in ${targetConfigFiles}
  do
    if test -f "$i"
    then
      . $i
    elif  test -f "${CURENVBASE}/tools/targets/$i"
    then
      . ${CURENVBASE}/tools/targets/$i
    elif  test -f "${CURENVBASE}/tools/targets/$i.sh"
    then
      . ${CURENVBASE}/tools/targets/$i.sh
    else
      echo "No such flavor or file"
      exit 1
    fi
  done

else
  . ${CURENVBASE}/tools/target.sh
fi

targetConfigCmd=""
for i in ${targetConfigFiles}
do
  targetConfigCmd="${targetConfigCmd} -f ${i}"
done


G_validateOption ${targetProdFlavor} "${targetProdFlavorValidList}"

G_validateOption ${targetProdStatus} "${targetProdStatusValidList}"

G_validateOption ${targetArchType}  "${targetArchTypeValidList}"

G_validateRunOS "${runOSValidList}"

if [ ${badUsage} ]
then
    usage
    exit 1
fi

shift `expr $OPTIND - 1`

#
# Now that we have all the parameters, get the job done.
#

distBaseDir="${CURENVBASE}/install/wce/dist"


safeCopyFile()
{
  # $1=srcAbsolutePath  and $2=dstDirectory
  if [ -f ${1} ] ;  then
    cp -p -v ${1} ${2}/`basename ${1}`.${targetArchType}
  else
    print -r -- Skipping $1
  fi
}

logActivitySeperator()
{
  print -r -- ""
  print -r -- "-------------------------------------------------------"
}


getRelInfo()
{
  print -r -- "Getting Relevant Information From releaseNotes"

  RELID_extractInfo ${targetReleaseNotesFile}
  . ${CURENVBASE}/tools/target.sh

#    case ${targetProdStatus} in
#     'beta')
#
#       ;;
#     'rel')
#       EH_problem "NOTYET"
#       ;;
#     *)
#       EH_problem "Unknown: ${i}"
#       exit 1
#       ;;
#   esac


LOG_message "   RELID_PROD_NAME=${RELID_PROD_NAME}"
LOG_message "   RELID_REVNUM=${RELID_REVNUM}"
LOG_message "   RELID_Rel_Date=${RELID_REL_DATE}"
LOG_message "   RELID_CVS_Tag=${RELID_CVS_TAG}"
LOG_message "   RELID_BUILD_DATE=${RELID_BUILD_DATE}"
LOG_message "   RELID_BUILD_USER=${RELID_BUILD_USER}"
LOG_message "   RELID_BUILD_HOST=${RELID_BUILD_HOST}"
LOG_message "   RELID_BUILD_DIR=`DOS_toFrontSlash ${CURENVBASE}`"

}

makeTarFiles()
{
  print -r -- "Putting the Distribution Disk in a tar File"

  #tarFileName=${RELID_PROD_NAME}-${RELID_REVNUM}-wce-${targetArchType}.tar
  tarFileName=${targetDistFileBase}-${targetProdStatus}-${RELID_REVNUM}-wce-${targetArchType}.tar

  cd ${distBaseDir}

  tar cf ${tarFileName} ${RELID_PROD_NAME}-${RELID_REVNUM}-wce-${targetArchType}

  ls -l ${tarFileName}

  tar tvf ${tarFileName}

  #rm -r -f ${tarFileName}
}




ETWP_putDocsOnFtpSite()
{
  TM_trace 9 ""

  mkdir -p ${targetFtpInstallDir}
  cd ${CURENVBASE}

  cp ./doc/etwp-wce/UserGuide.ps ${targetFtpInstallDir}/Neda-ETWP-wce-UserGuide.ps
  cp ./doc/etwp-wce/UserGuide.pdf ${targetFtpInstallDir}/Neda-ETWP-wce-UserGuide.pdf
  cp ./doc/etwp-wce/Neda-ETWP-wce-UserGuide.info ${targetFtpInstallDir}/Neda-ETWP-wce-UserGuide.info

  cp ./doc/etwp-wce/readme.tty ${targetFtpInstallDir}/Neda-ETWP-wce-readme.tty
  cp ./doc/etwp-wce/readme.ps ${targetFtpInstallDir}/Neda-ETWP-wce-readme.ps
  cp ./doc/etwp-wce/readme.pdf ${targetFtpInstallDir}/Neda-ETWP-wce-readme.pdf

  cp ./doc/etwp-wce/announce.tty ${targetFtpInstallDir}/Neda-ETWP-wce-announce.tty
  cp ./doc/etwp-wce/announce.ps ${targetFtpInstallDir}/Neda-ETWP-wce-announce.ps
  cp ./doc/etwp-wce/announce.pdf ${targetFtpInstallDir}/Neda-ETWP-wce-announce.pdf

  # NOTYET License

}

ETWP_putRelNotesOnFtpSite()
{
  TM_trace 9 ""

  mkdir -p ${targetFtpInstallDir}
  cd ${CURENVBASE}

  specificProductRelNotesBase=${targetProdName}-${targetOS}-${targetProdStatus}

  cp ./relnotes/${specificProductRelNotesBase}.tty ${targetFtpInstallDir}/Neda-${targetProdName}-${targetOS}-ReleaseNotes.tty
  cp ./relnotes/${specificProductRelNotesBase}.ps ${targetFtpInstallDir}/Neda-${targetProdName}-${targetOS}-ReleaseNotes.ps
  cp ./relnotes/${specificProductRelNotesBase}.pdf ${targetFtpInstallDir}/Neda-${targetProdName}-${targetOS}-ReleaseNotes.pdf

}


ETWP_putSoftwareOnFtpSite()
{
  TM_trace 9 ""

  mkdir -p ${targetFtpInstallDir}
  cd ${CURENVBASE}

  exeZipFileNameSh3=${targetDistFileBase}-${targetProdStatus}-${RELID_REVNUM}-wce-sh3.exe
  exeZipFileNameMips=${targetDistFileBase}-${targetProdStatus}-${RELID_REVNUM}-wce-mips.exe

  tarFileNameSh3=${targetDistFileBase}-${targetProdStatus}-${RELID_REVNUM}-wce-sh3.tar
  tarFileNameMips=${targetDistFileBase}-${targetProdStatus}-${RELID_REVNUM}-wce-mips.tar


  cp ./install/wce/dist/${exeZipFileNameSh3}  ${targetFtpInstallDir}
  ls -l ${targetFtpInstallDir}/${exeZipFileNameSh3}
  cp ./install/wce/dist/${exeZipFileNameMips}  ${targetFtpInstallDir}
  ls -l ${targetFtpInstallDir}/${exeZipFileNameMips}

  cp ./install/wce/dist/${tarFileNameSh3}  ${targetFtpInstallDir}
  ls -l ${targetFtpInstallDir}/${tarFileNameSh3}
  cp ./install/wce/dist/${tarFileNameMips}  ${targetFtpInstallDir}
  ls -l ${targetFtpInstallDir}/${tarFileNameMips} 
}

ETWP_putAnnouncementOnTheWeb()
{
  logActivitySeperator

  TM_trace 9 ""

  echo "ETWP_putAnnouncementOnTheWeb HTML files going to: ${targetWebInstallDir}"

  mkdir -p ${targetWebInstallDir}/announce

  cd ${CURENVBASE}/doc/etwp-wce/announce
  cp * ${targetWebInstallDir}/announce
}


ETWP_putUserGuideOnTheWeb()
{

  logActivitySeperator

  TM_trace 9 ""

  echo "ETWP_putUserGuideOnTheWeb HTML files going to: ${targetWebInstallDir}"

  mkdir -p ${targetWebInstallDir}/UserGuide

  cd ${CURENVBASE}
  cd ./doc/etwp-wce/html
  cp * ${targetWebInstallDir}/UserGuide
}

ETWP_putProductsSegmentOnTheWeb()
{
  productsSegmentDest=/usr/public/doc/web/htdocs/products.html

  logActivitySeperator

  TM_trace 9 ""

  echo "ETWP_putUserGuideOnTheWeb HTML files going to: ${productsSegmentDest}"

  cd ${CURENVBASE}
  cp ./doc/etwp-wce/websegment/websegment.html ${productsSegmentDest}
}


buildReleaseNotes () {
  cd ${CURENVBASE}/relnotes
  ./buildAll.sh
}

buildDocumentation () {
  cd ${CURENVBASE}/doc/etwp-wce
  ./buildAll.sh
}



doPreBuild () {
  # Give The user a chance to verify that 
  # the target.sh file is the right one.
  buildDocumentation
  buildReleaseNotes
  echo "Pre Build Activities Completed"
  echo "Now Build The Product"
}

doInstall () {
  case ${targetDistMethod} in
    'net-all')
      TM_trace 9 ""
      logActivitySeperator

      ETWP_putDocsOnFtpSite

      ETWP_putRelNotesOnFtpSite

      ETWP_putUserGuideOnTheWeb

      ETWP_putAnnouncementOnTheWeb

      ETWP_putProductsSegmentOnTheWeb

      ETWP_putSoftwareOnFtpSite
      ;;
    'disk'|'local'|'private')
      ;;
    *)
      EH_problem "Unknown: ${targetDistMethod}"
      exit 1
      ;;
  esac
}

# Now do it all

getRelInfo

case ${doAction} in
  'pre')
    doPreBuild
    ;;
  'install')
    doInstall
    ;;
  *)
    EH_problem "Unknown: ${doAction}"
    exit 1
    ;;
esac
