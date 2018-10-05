#!/bin/ksh

#
# RCS Revision: $Id: getaddons.sh,v 1.1 2002/10/25 23:18:36 mohsen Exp $
#

#
# Description:
#
# NOTYET: Later may be we can addon a 
# category type to each of these. For example
#   developer, release, full, mini
#
#  Then vorde.mini and vorde.developer and vorde.full will 
#   get the same module for different purposes.
#
#

#
# Author: Mohsen Banan.
# 
#

if [ -d tools ] ;  then
  . tools/ocp-lib.sh 
  echo "In `pwd` getting ..."
else
  echo "tools directory should be in `pwd` -- Wrong Base Directory?"
  exit
fi


#
# DEFAULTS
#

systemName=`uname -n`
devlOsType=`uname -s`

# 
supportedPkgsList="NSE-MULPUB OCP VORDE MAILMEANYWHERE ESRO EMSD-UA EMSD-SA BIGPUB"

usage() {
    printf >&2 "Usage: $0 [ -T traceLevel ] [ -u ]  [-s subjectPkg] [-a action]\n"
    printf >&2 "Usage: subjectPkg is one of: all ${supportedPkgsList}\n"
    printf >&2 "Usage: action is one of: ${actionList}\n"
    exit 1
}


#PATH=/usr/sbin:/usr/bin:/sbin

# Get options (defaults immediately below):

badUsage=
traceLevel=

targetConfigFiles=""

G_argsOrig=$*
TM_trace 9 ${G_argsOrig}

while getopts s:T:u? c
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
    s)
        subjectPkg="${OPTARG}"
	;;
    u | \?)
	badUsage=TRUE
	;;
    esac
done



#G_validateOption ${action}  "${actionValidList}"

#G_validateRunOS "${runOSValidList}"

if [ ${badUsage} ]
then
    usage
    exit 1
fi

shift `expr $OPTIND - 1`

G_argv=$*
TM_trace 9 Args=$*

#
# Now that we have all the parameters, get the job done.
#

nedaEnv="/tmp"

checkoutIfNotThere()
{
  if [ -d ${1} ] ;  then
    echo "Dir $1 already there -- ${2} NOT checked out" 
  else
    # NOTYET test for exit codes 
    cvs checkout ${2} 
  fi
}

actionNotYet()
{
  logActivitySeparator
  print -r -- " NOTYET "
}

actionCheckout ()
{
  dateTag=`date +%y%m%d%H%M%S`
  # /usr/devenv/nedaLogs
  mkdir -p ${nedaEnv}/nedaLogs
  logFileName=${nedaEnv}/nedaLogs/${dateTag}.${G_name}.${subjectPkg}.checkout.LOG
  errFileName=${nedaEnv}/nedaLogs/${dateTag}.${G_name}.${subjectPkg}.checkout.ERR

  echo To see the output:
  echo "tail -f ${logFileName}"
  echo "tail -f ${errFileName}"
  echo "" 

  print -r -- " `date`  -- ${G_argsOrig} -- in ${nedaEnv} -- by: `id`" 1> ${logFileName} 2> ${errFileName} 


  logActivitySeparator 1>> ${logFileName} 2>> ${errFileName} 
  print -r -- " `date`  -- CVS Checkout of  ${subjectPkg} " 1>> ${logFileName} 2>> ${errFileName} 


  case ${subjectPkg} in

    'OCP')
      checkoutIfNotThere src/multiocp ocpMul 1>> ${logFileName} 2>> ${errFileName} 
      ;;

    'VORDE')

      checkoutIfNotThere src/multiocp ocpMul 1>> ${logFileName} 2>> ${errFileName} 

      checkoutIfNotThere pkgSpecs/VORDE vordeMulPub.pkgSpecs 1>> ${logFileName} 2>> ${errFileName}

      checkoutIfNotThere install/VORDE vordeMulPub.install 1>> ${logFileName} 2>> ${errFileName}

      checkoutIfNotThere src/kpathsea  kpathsea 1>> ${logFileName} 2>> ${errFileName} 

      checkoutIfNotThere src/cpp cpp 1>> ${logFileName} 2>> ${errFileName}

      checkoutIfNotThere src/vorde vordeMulPubSrc 1>> ${logFileName} 2>> ${errFileName}

      checkoutIfNotThere src/vorde/nedaApps  vordeMulNedaApps 1>> ${logFileName} 2>> ${errFileName}
      ;;

    'MAILMEANYWHERE')

      checkoutIfNotThere install/MAILMEANYWHERE mailMeAnyWhereMulPub.install 1>> ${logFileName} 2>> ${errFileName}

      checkoutIfNotThere examples/MAILMEANYWHER mailMeAnyWhereMulPub.examples 1>> ${logFileName} 2>> ${errFileName} 


      checkoutIfNotThere openSrc baseOpenSrc 1>> ${logFileName} 2>> ${errFileName} 

      checkoutIfNotThere openSrc/other/fetchMail  mailMeAnyWhereSrcFetchMail 1>> ${logFileName} 2>> ${errFileName} 

      checkoutIfNotThere openSrc/other/procMail mailMeAnyWhereSrcProcMail 1>> ${logFileName} 2>> ${errFileName} 
      ;;

    'ESRO') 
      actionNotYet
      ;;

    'EMSD-UA') 
      actionNotYet
      ;;

    'EMSD-SA') 
      actionNotYet
      ;;
    *)
      EH_problem "Unknown pkg: ${subjectPkg}"
      exit 1
      ;;
  esac
}

actionProcessor ()
{
  actionCheckout
}


if [ "${subjectPkg}" = "all" ]
then
  for i in ${supportedPkgsList}
  do
    subjectPkg=$i
    echo ${subjectPkg}
    actionProcessor
  done
else
  actionProcessor
fi



