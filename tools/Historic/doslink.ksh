#! /bin/sh
#
# RCS: $Id: doslink.ksh,v 1.1 2002/10/25 23:47:41 mohsen Exp $
#
# Syntax:
# Description:
#

#echo --- ${KSHCURENVBASE}

usage() {
    printf >&2 "Usage: $0 [-T traceLevel] [-u] [-m MODEL] -o linkOutput  objs ... libs ...\n"
    printf >&2 "Usage: MODEL is one of {SMALL,MEDIUM,LARGE}, LARGE is default\n"
    exit 1
}


badUsage=
traceLevel=
model=LARGE

while getopts o:m:T:u? c
do
    case $c in
    o)
	targetFile=$OPTARG
	;;
    m)
	model=$OPTARG
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

if [ ${traceLevel} ]
then
    echo "Trace Level set to: ${traceLevel}"
    # Make Shell be Verbose
    set -v
fi

case ${model} in
'SMALL' )
    modelLetter="s"
    modelLib="lib.sml"
    ;;
'MEDIUM' )
    modelLetter="m"
    modelLib="lib.mdm"
    ;;
'LARGE' )
    modelLetter="l"
    modelLib="lib.lrg"
    ;;
* )
    echo "$0: invalid Model: ${model}"
    badUsage=TRUE 
    ;;
esac

if [ ${badUsage} ]
then
    usage
    exit 1
fi


shift `expr $OPTIND - 1`

objList="c0${modelLetter}"

sysLibList="c${modelLetter}.lib"

for d in $*
do
	echo "$d";
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

# NOTYET, LIBSPATH should be more configurable.
#
echo " /L${KSHCURENVBASE}\\\\results\\\\ae_dos\\\\${modelLib};${KSHCURENVBASE}\\\\src\\\\cscdpd.pkd\\\\tools;c:\\\\bc45\\\\lib /m /s /v" > tlink.cfg

cat > $$.rsp << __EOF__
${objList}
`basename ${targetFile}`.exe
`basename ${targetFile} .rom`.map
${libList} ${sysLibList}
__EOF__

tlink  @$$.rsp

# NOTYET -- We need to test for exit status here
rm -- $$.rsp








