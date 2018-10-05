#!/bin/ksh
#
# RCS: $Id: targetVarsExpand.sh,v 1.3 2002/11/01 01:24:03 mohsen Exp $
#
# Syntax:
# Description:
#
#

#
# Customize the following on a per-product basis
# Default Values are set below.
#


#
# Author: Mohsen Banan.
#
#
#  NOTYET
#    - The Info For The BuildEnv should also come here. 
#    - Break it into one or more common functions
#          Say targetVarsGet, relIdVarsGet, buildEnvVarsGet
# 

#set -v

. ${CURENVBASE}/tools/lib/ocp-lib.sh


#
# DEFAULTS
#
#targetReleaseNotesFile=${CURENVBASE}/relnotes/ocpdevl-mul-beta.tex
inFile=
#STATUS_STRING="released on"
outFile=

runOSValidList="SunOS Windows_NT"
#runOSValidList="Windows_NT"

#
# DEFAULTS
#

usage() {
    print >&2 "Usage: $0 [-f targetConfigFile] [-i inFile] [-o outFile]"
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

while getopts o:i:f:T:u? c
do
    case $c in
    'T')
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
    i)
      # input file
	inFile="${OPTARG}"
	;;
    o)
	outFile="${OPTARG}"
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


if [ "${osType}" = "SunOS" ]
then
  LOG_message "targetReleaseNotesSpecificFile is set to: $targetReleaseNotesSpecificFile"
fi

G_validateOption ${targetArchType}  "${targetArchTypeValidList}"

#G_validateRunOS "${runOSValidList}"

#if [ "${inFile}" = "" ] ; then
#  EH_problem "Must specify inFile"
#    usage
#     exit 1
# fi

# if [ "${outFile}" = "" ] ; then
#   EH_problem "Must specify outFile"
#     usage
#     exit 1
# fi

if [ ${badUsage} ]
then
    usage
    exit 1
fi

shift `expr $OPTIND - 1`

#
# Now that we have all the parameters, get the job done.
#


#
# grovel around for the release information
#


RELID_extractInfo ${targetReleaseNotesSpecificFile}

# We need to source target.sh again becasue some of the 
# vars in target.sh use RELID_
. ${CURENVBASE}/tools/target.sh


if [ "${osType}" != "SunOS" ]
then
  CURENVBASE=`DOS_toFrontSlash ${CURENVBASE}`
fi

#
#


sed -e "s@%RELID_PROD_NAME%@${RELID_PROD_NAME}@g" \
    -e "s@%RELID_REVNUM%@${RELID_REVNUM}@g" \
    -e "s@%RELID_Rel_Date%@${RELID_REL_DATE}@g" \
    -e "s@%RELID_CVS_Tag%@${RELID_CVS_TAG}@g" \
    -e "s@%RELID_BUILD_DATE%@${RELID_BUILD_DATE}@g" \
    -e "s@%RELID_BUILD_USER%@${RELID_BUILD_USER}@g" \
    -e "s@%RELID_BUILD_HOST%@${RELID_BUILD_HOST}@g" \
    -e "s@%RELID_BUILD_DIR%@${CURENVBASE}@g" \
    -e "s@%targetProd%@${targetProd}@g" \
    -e "s@%targetProdName%@${targetProdName}@g" \
    -e "s@%targetReleaseNotesSpecificFile%@${targetReleaseNotesSpecificFile}@g" \
    -e "s@%targetProdFlavor%@${targetProdFlavor}@g" \
    -e "s@%targetOS%@${targetOS}@g" \
    -e "s@%targetArchType%@${targetArchType}@g" \
    -e "s@%targetProdStatus%@${targetProdStatus}@g" \
    -e "s@%targetDistMethod%@${targetDistMethod}@g" \
    -e "s@%targetDistFileBase%@${targetDistFileBase}@g" \
    -e "s@%targetDistUser%@${targetDistUser}@g" \
    -e "s@%targetFtpInstallDirRelative%@${targetFtpInstallDirRelative}@g" \
    -e "s@%targetFtpInstallDir%@${targetFtpInstallDir}@g" \
    -e "s@%targetWebInstallDirRelative%@${targetWebInstallDirRelative}@g" \
    -e "s@%targetWebInstallDir%@${targetWebInstallDir}@g" \
  < ${inFile} > ${outFile}


