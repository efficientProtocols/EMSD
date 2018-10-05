#!/sbin/sh

#
# RCS Revision: $Id: receiveIt.sh,v 1.1 2002/10/25 23:47:44 mohsen Exp $
#

#
# Description:
#
#
#

#
# Author: Mohsen Banan.
# 
#

usage() {
    printf >&2 "Usage: $0 [ -T traceLevel ] [ -u ] [-p] inFile\n"
    printf >&2 "Usage: -p: enable preprocessing\n"
    printf >&2 "Usage: inFile: compressed tar file to receive\n"
    exit 1
}

osType=`uname -s`
case ${osType} in
'SunOS')
	#PATH=/usr/sbin:/usr/bin:/sbin
	unzipCmd=gunzip
	;;
'Windows_NT')   
	unzipCmd=uncompress
	;;
'Windows_95')
	unzipCmd=uncompress
	;;
'DOS')
	unzipCmd=notyet
	;;
esac


# Get options (defaults immediately below):

badUsage=
traceLevel=
processFirst=

while getopts cT:u? c
do
    case $c in
    p)
	processFirst=TRUE
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

if [ ${traceLevel} ]
then
    echo "Trace Level set to: ${traceLevel}"
    # Make Shell be Verbose
    set -v
fi

shift `expr $OPTIND - 1`

if [ "${1}" = "" ]
then
    echo "Need inFile -- Must provide inFile"
    usage
    exit 1
fi

inFile=$1

if [ ${processFirst} ]
then
    echo "NOTYET"
fi

here=`pwd`


# NOTYET
# 
# Confirm that the input file exists and
# is of the right kind.
#

echo "Uncompressing ${inFile} ..."

${unzipCmd} ${inFile} 
tar xvf `basename ${inFile} .gz` 
