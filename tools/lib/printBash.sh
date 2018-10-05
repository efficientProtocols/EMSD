#!/bin/bash

#
# RCS Revision: $Id: 
#

# Description:
# 
# This function is written to create a bash print
# command so that it is compatible with ksh93
# style print.
#
# This function is not complete yet.
#

#set -x
if [[ "${BASH_VERSION}X" != "X" ]] ; then

  #echo ${BASH_VERSION}

  function print {
    OPTIND=1
    typeset outFileDesc=1 nFlag=0 rFlag=0 sFlag=0 fFlag=0 badUsage=FALSE

    while getopts "nrsu:f:" c
    do
      case $c in
	n) 
	   nFlag=1
	   ;;
	r)
	   rFlag=1
	   ;;
	s)
	   sFlag=1
	   ;;
	u) 
	   case "$OPTARG" in
	     *[!1-9]* )
		  echo "$0: -u requires a numeric option"
		  badUsage=TRUE
		  ;;
	     [1-9]* )
		  outFileDesc=${OPTARG}
		  ;;
	   esac
	   ;;
	f)
	   fFlag=1
	   ;;
	'*'|\?)
	   echo "Unknown print option: $c"
	   badUsage=TRUE
	   ;;
      esac
    done

shift `expr $OPTIND - 1`

  case ${outFileDesc} in
    '1')
	 if [ ${nFlag} == "1" ] ; then
	   echo -n "$@"
	 else
	   echo "$@"
	 fi
	 ;;
    *)
         echo $@ 1>&${outFileDesc}
	 ;;
  esac

   if [ "${badUsage}X" == "TRUEX" ] ; then
     echo "Usage: print [-nrs] [-u unit] [-f format] [arguments]"
     return 1
   fi

  }
fi


function printTest {
  print "Print without any options."
  print -- "Print without 2 dashes."
  print -n "Print without Newline."
  print -- "Print without 2 dashes."
  print -r "Print without 2 dashes \n."
  print "Print without any options."
  print "Print without any options."
  print -f "%s\t%s\n -- -f Not working yet"
}

#printTest
