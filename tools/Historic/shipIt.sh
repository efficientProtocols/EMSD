#!/sbin/sh

#
# RCS Revision: $Id: shipIt.sh,v 1.1 2002/10/25 23:47:44 mohsen Exp $
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
    printf >&2 "Usage: $0 [ -T traceLevel ] [ -u ] [-c]\n"
    printf >&2 "Usage: -c for clean\n"
    exit 1
}

#PATH=/usr/sbin:/usr/bin:/sbin

# Get options (defaults immediately below):

badUsage=
traceLevel=
cleanFirst=

while getopts cT:u? c
do
    case $c in
    c)
	cleanFirst=TRUE
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

if [ ${cleanFirst} ]
then
    ./cleanall.sh
fi

osType=`uname -s`
case ${osType} in
'SunOS')   
	zipCmd=gzip
	;;
'Windows_NT') 
        #. win32setenv.sh 
	zipCmd=mkszip
	;;
'Windows_95')
	zipCmd=mkszip
	;;
'DOS')
	zipCmd=notyet
	;;
esac



here=`pwd`
here=`basename ${here}`



outFile=`date +%y%m%d%H%M%S`.${here}.tar.gz
cd ..
echo "Running: tar cf - ${here} | ${zipCmd} > ${outFile}"
tar cf - ${here} | ${zipCmd} > ${outFile}
ls -l ${outFile} 
        
    