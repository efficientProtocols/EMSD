#
# RCS Revision: $Id: buildallhead.sh,v 1.1 2002/10/25 23:47:40 mohsen Exp $
#

#
# Description:
#
#  Example: 
#
#       For WIN95/WINNT
#            ./buildall.sh -f os-winnt.sh -f win32.sh
#
#       For WINCE 
#           buildall.sh -f os-wince -f cpu-mips  -f win32
#           buildall.sh -f os-wince -f cpu-sh3  -f win32
#           buildall.sh -f os-wince-emulation  -f win32
#
#

#
# Author: Mohsen Banan.
# 
#

#set -v

if test `uname -s` = 'CYGWIN_NT-4.0'               # CygWin doesn't like it
then
    devlOsType='CygWin'
else
    devlOsType=`uname -s`
fi

usage() {
    printf >&2 "Usage: $0 [ -T traceLevel ] [ -u ] [-f flavor] \n"
    printf >&2 "Usage: flavor is one of {cc,gcc,purify}\n"
    exit 1
}

#
# DEFAULTS
#
prodFlavor=""

systemName=`uname -n`



###


#PATH=/usr/sbin:/usr/bin:/sbin

# Get options (defaults immediately below):

badUsage=
traceLevel=

TM_trace 9 $*

while getopts f:T:u? c
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
	prodFlavor="${prodFlavor} ${OPTARG}"
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

shift `expr $OPTIND - 1`

# Choose the default prodFlavor base on this development system

if [ "${prodFlavor}" = "" ]
then
  case ${devlOsType} in
    'CYGWIN_NT-5.0')
      #prodFlavor="os-winnt.sh pdt-current.sh  cygwin.sh"
      prodFlavor="os-winnt.sh pdt-current.sh  dev-cyggcc.sh"
      ;;
    'Windows_NT')   
      #prodFlavor=ocp+dos+bc45.sh
      #prodFlavor=ocp+ae+bc45.sh
      prodFlavor="os-winnt.sh pdt-current.sh  win32.sh"

      #prodFlavor="os-wince.sh cpu-mips.sh pdt-uadevl.sh win32.sh"
      #prodFlavor="os-wince.sh cpu-sh3.sh pdt-uadevl.sh win32.sh"
      ;;
    'UWIN-NT')   
      prodFlavor="os-winnt.sh pdt-current.sh  uwin32.sh"
      ;;
    'Windows_95')
      #prodFlavor=ocp+dos+bc45.sh
      prodFlavor=ocp+ae+bc45.sh
      ;;
    'SunOS')
      prodFlavor="pdt-current.sh ocp+sol2+gcc.sh"
      # NOTYET
      SHELL=/bin/ksh
      ;;
    'Linux')
      prodFlavor="pdt-current.sh ocp+linux+gcc.sh"
      # NOTYET
      SHELL=/bin/ksh
      ;;
    'DOS')
      echo "Unsupported System: ${devlOsType}"	
	;;
    *)
      echo "Unsupported System: ${devlOsType}"
      exit 1
      ;;
  esac
fi


#
# Now Validate that the said prodFlavor is valid
#

#
# First make sure that it is a generally valid flavor
#

mkpFlavor=""
for i in ${prodFlavor}
do
  if   test -f "${CURENVBASE}/tools/${i}"
  then
    mkpFlavor="${mkpFlavor} ${i}"
  elif  test -f "${CURENVBASE}/tools/${i}.sh"
  then
    mkpFlavor="${mkpFlavor} ${i}.sh"
  else
    echo "No such flavor or file"
  fi
done

TM_trace 9 "${mkpFlavor}"

#
# Now make sure it is one that we support here
#
for i in ${mkpFlavor}
do
  TM_trace 9 $i
  case ${i} in
    'ocp+ae+bc45.sh'|'ocp+dos+bc45.sh'|'win32.sh')
      if [ "${devlOsType}" != "Windows_NT" ]
      then
	echo "${mkpFlavor} is the wrong flavor for ${devlOsType} system"
	exit 1
      fi
      ;;
    'ocp+sol2+gcc.sh')
      if [ "${devlOsType}" != "SunOS" ]
      then
	EH_problem "${mkpFlavor} is the wrong flavor for ${devlOsType} system"
	exit 1
      fi
      ;;
    'ocp+linux+gcc.sh')
      ;;
    'uwin32.sh')
      if [ "${devlOsType}" != "UWIN-NT" ]
      then
	EH_problem "${mkpFlavor} is the wrong flavor for ${devlOsType} system"
	exit 1
      fi
      ;;
    'cygwin.sh'|'dev-cygmsc.sh'|'dev-cyggcc.sh')
      if [ "${devlOsType}" != "CYGWIN_NT-5.0" ]
      then
	EH_problem "${mkpFlavor} is the wrong flavor for ${devlOsType} system"
	exit 1
      fi
      ;;
    'os-winnt.sh'|'os-wince.sh'|'os-win95.sh')
      ;;
    'cpu-x86.sh'|'cpu-mips.sh'|'cpu-sh3.sh')
      ;;
    'pdt-uadevl.sh' | 'pdt-vorde.sh' | 'pdt-current.sh' )
      ;;
    'purify')
      EH_oops "NOTYET"
      exit
      ;;
    *)
      EH_problem "XXX - Unknown prodFlavor: ${mkpFlavor}"
      exit 1
      ;;
  esac
done



#
# We have got it
# 
# Now let's go ahead and build a command line using
# mkpFlavor

MKP="${CURENVBASE}/tools/mkp.sh"
buildAllCmd="./buildall.sh"
if [ ${traceLevel} ]
then
    MKP="${MKP} -T ${traceLevel}"
    buildAllCmd="${buildAllCmd} -T ${traceLevel}"
fi
for i in ${mkpFlavor}
do
  TM_trace 9 ${MKP}
  MKP="${MKP} -f ${i}"
  buildAllCmd="${buildAllCmd} -f ${i}"
done

#fi

###### DO NOT TOUCH ANYTHING ABOVE THIS LINE ######

