#!/bin/ksh
#
# RCS Revision: $Id: wcedownload.sh,v 1.2 2002/10/25 19:38:08 mohsen Exp $
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
prodFlavor="baseua"
prodFlavorValidList="baseua internal"

doAction="exist"
doActionValidList="delete exist update"

archType="mips"
archTypeValidList="sh3 mips"

runOSValidList="SunOS Windows_NT"
#runOSValidList="Windows_NT"

#
# DEFAULTS
#
usage() {
    printf >&2 "Usage: $0 [ -T traceLevel ] [ -u ] [-f flavor] [-a arch] [-d doAction]\n"
    printf >&2 "Usage: flavor is one of: ${prodFlavorValidList}\n"
    printf >&2 "Usage: arch is one of: ${archTypeValidList}\n"
    printf >&2 "Usage: doAction is one of: ${doActionValidList}\n"
    exit 1
}

systemName=`uname -n`
devlOsType=`uname -s`


###


#PATH=/usr/sbin:/usr/bin:/sbin

# Get options (defaults immediately below):

badUsage=
traceLevel=

TM_trace 9 $*

while getopts d:a:f:T:u? c
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
	prodFlavor="${OPTARG}"
	;;
    d)
	doAction="${OPTARG}"
	;;
    a)
	archType="${OPTARG}"
	;;
    u | \?)
	badUsage=TRUE
	;;
    esac
done

G_validateOption ${prodFlavor} "${prodFlavorValidList}"

G_validateOption ${archType}  "${archTypeValidList}"

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

echo "prodFlavor is ${prodFlavor} archType is ${archType}"
echo "command line args are $*"


PFILE=pfile

doDelete () {
  case ${prodFlavor} in
    'baseua')
      ${PFILE} EXIST   "WCE:\windows\uashwce_one.exe"
      ${PFILE} DELETE   "WCE:\windows\uashwce_one.exe"

      ${PFILE} EXIST   "WCE:\windows\emsd_mtsp.dll"
      ${PFILE} DELETE   "WCE:\windows\emsd_mtsp.dll"

      EH_problem "NOTYET, /emsd/install/regsetup.exe"
      ;;
    *)
      EH_problem "Unknown: ${doAction}"
      exit 1
      ;;
  esac
}

doExist () {
  case ${prodFlavor} in
    'baseua')
      ${PFILE} EXIST   "WCE:\windows\uashwce_one.exe"

      ${PFILE} EXIST   "WCE:\windows\emsd_mtsp.dll"
      ;;
    *)
      EH_problem "Unknown: ${doAction}"
      exit 1
      ;;
  esac
}

doUpdate () {
  case ${prodFlavor} in
    'baseua')
      EH_problem "NOTYET"
      ;;
    *)
      EH_problem "Unknown: ${doAction}"
      exit 1
      ;;
  esac
}

case ${doAction} in
  'delete')
    doDelete
    ;;
  'exist')
    doExist
    ;;
  'update')
    doUpdate
    ;;
  *)
    EH_problem "Unknown: ${doAction}"
    exit 1
    ;;
esac




