#! /bin/ksh
#
# RCS: $Id: win32lib.sh,v 1.11 2003/03/28 20:01:26 pinneke Exp $
#
# Syntax:
# Description:
#

#
#

. ${CURENVBASE}/tools/win32sdk.libSh

if [[ "${BASH_VERSION}X" != "X" ]] ; then
 . ${OSMTROOT}/lib/printBash.sh
fi

if [ -f ${OSMTROOT}/bin/opConfig.sh ] ; then
  .  ${OSMTROOT}/bin/opConfig.sh 
else
  echo "${OSMTROOT}/bin/opConfig.sh not found.\n"
  exit 1
fi

. ${opBase}/lib/ocp-lib.sh
. ${opBase}/lib/doLib.sh

badUsage=
traceLevel=

targetCPU=""    # MANDATORY
targetOS=""     # MANDATORY


while getopts s:vc:n:T:u? c
do
    case $c in
    c)
        targetCPU=$OPTARG
        ;;
    s)
        targetOS=$OPTARG
        ;;
    v)
	G_verbose="verbose"
	;;
    n)
	case ${OPTARG} in 
	    "runOnly")
		G_runMode="runOnly"
		;;
	    "showOnly")
		G_runMode="showOnly"
		;;
	    "showRun")
		G_runMode="showRun"
		;;
	    "runSafe")
		G_runMode="runSafe"
		;;
	    "showProtected")
		G_runMode="showProtected"
		;;
	    "showRunProtected")
		G_runMode="showRunProtected"
		;;
	    *)
		badUsage=TRUE
		;;
	esac
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
    set -v
fi

shift `expr $OPTIND - 1`

baseFileName=`basename $1`

shift

prefixFileName=${baseFileName%%\.*}
suffixFileName=${baseFileName##*\.}

case ${suffixFileName} in
'lib')
    libOutFile="${prefixFileName}.lib"
    ;;
'a')
    # Convert Connonical .a libraries to .lib
    libOutFile="${prefixFileName}.lib"
    ;;
*)
    echo ignoring ${baseFileName}
    ;;
esac

objList=""

for d in $*
do
        #echo "$d";
        baseFileName=`basename $d`
        prefixFileName=${baseFileName%%\.*}
        suffixFileName=${baseFileName##*\.}

        case ${suffixFileName} in
        'o')
            objList="${objList} ${baseFileName}"
            ;;
        'obj')
            objList="${objList} ${baseFileName}"
            ;;
        'lib')
            libList="${libList} ${baseFileName}"
            ;;
        'a')
            # Convert Connonical .a libraries to .lib
            libList="${libList} ${prefixFileName}.lib"
            ;;
        *)
            echo ignoring ${baseFileName}
            ;;
        esac
done

CURENVBASE_winVersionPATH

targetCPU_analyze

libOut="${libOutDir}/${libOutFile}"
#libOut="${libOutFile}"

if [ -f ${libOut} ]
then
  augment=${libOut}
fi

LIB32="link.exe -lib"
LIB32_FLAGS=""
LIB32_OUTPUT="-out:\"${libOut}\""
#NOLOGO="-nologo"
NOLOGO=""
DEF_FLAGS=""    # From the MS Template, don't know what for

rspFile="`basename ${libOutFile} .lib`.rsp"
lstFile="`basename ${libOutFile} .lib`.lst"

#### OLD MODEL: generate a .rsp file first ####
# cat > ${rspFile} << __EOF__
# ${NOLOGO} ${LIB32_OUTPUT} ${LIB32_FLAGS} ${DEF_FLAGS} ${augment} ${objList}
# __EOF__

#rm  ${libOutFile}

#opDoComplain ${LIB32} @${rspFile}
opDoComplain eval "${LIB32} ${NOLOGO} ${LIB32_OUTPUT} ${LIB32_FLAGS} ${DEF_FLAGS} ${augment} ${objList}"


# NOTYET -- We need to test for exit status here

ls -l ${libOut}

#rm -- ${rspFile}








