#! /bin/ksh
#
# RCS: $Id: win32cc.sh,v 1.2 2003/03/18 01:44:20 pinneke Exp $
#
# Syntax:
# Description:
#
#set -x 
#echo ${CURENVBASE}
#echo ${OSMTROOT}

if [[ "${BASH_VERSION}X" != "X" ]] ; then
 . ${OSMTROOT}/lib/printBash.sh
fi

. $CURENVBASE/tools/pdt-current.sh
. $CURENVBASE/tools/win32sdk.libSh 

badUsage=
traceLevel=

targetCPU=""    # MANDATORY
targetOS=""     # MANDATORY
programType="CONSOLE"  # GUI, DLL, OLE

while getopts t:s:o:c:T:u? c
do
    case $c in
    o)
	targetFile=$OPTARG
	;;
    c)
	targetCPU=$OPTARG
	;;
    s)
	targetOS=$OPTARG
	;;
    t)
	programType=$OPTARG
	;;
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
     u | \?)
	badUsage=TRUE
	;;
    esac
done

if [ ${badUsage} ]
then
    usage
    exit 1
fi

if [ "${targetCPU}" = "" ] ; then
  echo "EH_OOPS"
  usage
  exit 1
fi

if [ "${targetOS}" = "" ] ; then
  echo "EH_OOPS"
  usage
  exit 1
fi

if [ ${traceLevel} ]
then
    echo "Trace Level set to: ${traceLevel}"
    # Make Shell be Verbose
    set -v
fi

shift `expr $OPTIND - 1`

#baseFileName=`basename $1`

#shift


print -r -- targetFile = ${targetFile}

case ${targetCPU} in
'SH3')   
  resultsCPU=sh3
  machineCPU=SH3
  wceLibCPU=wcesh
  ;;
'MIPS' )
  resultsCPU=mips
  machineCPU=MIPS
  wceLibCPU=wcemips
  ;;
'i386' )
  resultsCPU=x86
  machineCPU=I386
  ;;
'EMULATOR' )
  resultsCPU=x86
  machineCPU=x86
  OSVERSION=wce300
  PLATFORM="MSPocketPC"
  ;;
* )
  echo "EH_problem - $0: (1) invalid Target CPU: ${targetCPU}"
  exit 1
  ;;
esac

echo "Compiling for OS=${targetOS} CPU=${targetCPU} programType=${programType}"

