#!/bin/ksh
#
# RCS Revision: $Id: texBuild.sh,v 1.1 2002/10/25 23:47:44 mohsen Exp $
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

runOSValidList="SunOS Windows_NT"
#runOSValidList="Windows_NT"

#
# DEFAULTS
#
usage() {
    printf >&2 "Usage: $0 [ -T traceLevel ] [ -u ] [-f flavor] [-s status] [-a arch]\n"
    printf >&2 "Usage: flavor is one of: ${targetProdFlavorValidList}\n"
    printf >&2 "Usage: status is one of: ${targetProdStatusValidList}\n"
    printf >&2 "Usage: arch is one of: ${targetArchTypeValidList}\n"
    exit 1
}

systemName=`uname -n`
devlOsType=`uname -s`


###


#PATH=/usr/sbin:/usr/bin:/sbin

# Get options (defaults immediately below):

badUsage=
traceLevel=

action=all
actionValidList="all UserGuide.ps UserGuide.html"

targetConfigFiles=""

TM_trace 9 $*

while getopts a:p:f:T:u? c
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
	targetConfigFiles="${targetConfigFiles} ${OPTARG}"
	;;
    a)
        action="${OPTARG}"
	;;
    p)
	targetDistPasswd="${OPTARG}"
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

targetConfigCmd=""
for i in ${targetConfigFiles}
do
  targetConfigCmd="${targetConfigCmd} -f ${i}"
done


G_validateOption ${targetProdFlavor} "${targetProdFlavorValidList}"

G_validateOption ${targetProdStatus} "${targetProdStatusValidList}"

G_validateOption ${targetArchType}  "${targetArchTypeValidList}"

#G_validateOption ${action}  "${actionValidList}"

G_validateRunOS "${runOSValidList}"

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


logActivitySeparator()
{
  print -r -- ""
  print -r -- "-------------------------------------------------------"
}


rmIfThere()
{
  while getopts v c
  do
    case $c in
      'v')
	rmIfThereVerbose=TRUE
	;;
      *)
	EH_problem "rmIfThere: BAD OPTION"
	exit
	;;
    esac
  done

  shift `expr $OPTIND - 1`

  for f in $* 
  do
      if [ -f ${f} ] ;  then
	if [ -n ${rmIfThereVerbose} ] ; then 
	  echo Removing $f
	fi
	/bin/rm $f
      else
	TM_trace 9 -- Skipping $f
      fi
  done

}

cleanKeepResults()
{
  # $* says what the subjects are
  for subject in ${G_argv}
  do
    print -r -- "Cleaning But Keeping Results For ${subject}"
    TM_trace 9 "${subject}";

    subjectBaseName=`FN_prefix ${subject}`
    subjectExtensionName=`FN_extension ${subject}`
    TM_trace 9 "Subject Name is  ${subjectBaseName}"
    TM_trace 9 "Subject Extension is  ${subjectExtensionName}"
	
    case ${subjectExtensionName} in
      'ttytex')
	rmIfThere -v  ${subjectBaseName}.aux ${subjectBaseName}.dlog\
	              ${subjectBaseName}.dvi  ${subjectBaseName}.toc \
	              ${subjectBaseName}.log ${subjectBaseName}.tex \
	 	      ${subjectBaseName}.idx ${subjectBaseName}.lof \
	 	      ${subjectBaseName}.lot
 	;;
      'texinfo')
	;;
      'tex')
	EH_oops "NOTYET"
	exit
	;;
      *)
	EH_problem "Unknown: ${i}"
	exit 1
	;;
    esac

  done
}

realclean()
{
  # $* says what the subjects are
  for subject in ${G_argv}
  do
    print -r -- "Cleaning Everything For ${subject}"
    TM_trace 9 "${subject}";

    subjectBaseName=`FN_prefix ${subject}`
    subjectExtensionName=`FN_extension ${subject}`
    TM_trace 9 "Subject Name is  ${subjectBaseName}"
    TM_trace 9 "Subject Extension is  ${subjectExtensionName}"
	
    case ${subjectExtensionName} in
      'ttytex')
	rmIfThere -v  ${subjectBaseName}.aux ${subjectBaseName}.dlog\
	              ${subjectBaseName}.dvi  ${subjectBaseName}.toc \
	              ${subjectBaseName}.log ${subjectBaseName}.tex \
	 	      ${subjectBaseName}.idx ${subjectBaseName}.lof \
	 	      ${subjectBaseName}.lot \
	              ${subjectBaseName}.tty ${subjectBaseName}.ps \
	              ${subjectBaseName}.pdf
 	;;
      'texinfo')
	;;
      'tex')
	EH_oops "NOTYET"
	exit
	;;
      *)
	EH_problem "Unknown: ${i}"
	exit 1
	;;
    esac

  done
}


ttytexToTtyDvi()
{
  # $1 is the module name
  print -r -- "Building the $1.dvi File For TTY Generation"

  #rmIfThere $1.aux $1.dlog $1.dvi  $1.toc $1.log

  # This module must be built with latex 209
  . /opt/public/latex2e-95/nedaConfig/setLatex-209.sh

  sed -e "s@%ttyStyleFile%@txt-old,@g" \
      -e "s@%commentWhenTty%@%@g" \
      -e "s@%commentWhenPs%@@g" \
      $1.ttytex | expand | targetVarsExpand.sh > $1.tex
  latex2dvi $1.tex
  ls -l $1.dvi
}

ttytexToDvi()
{
  # $1 is the module name
  print -r -- "Building the $1.dvi File"

  #rmIfThere $1.aux $1.dlog $1.dvi  $1.toc $1.log

  # This module must be built with latex 209
  . /opt/public/latex2e-95/nedaConfig/setLatex-209.sh

  sed -e "s@%ttyStyleFile%@@g" \
      -e "s@%commentWhenTty%@@g" \
      -e "s@%commentWhenPs%@%@g" \
      $1.ttytex | expand | targetVarsExpand.sh > $1.tex
  latex2dvi $1.tex
  ls -l $1.dvi
}

ttytexToHtml()
{
  # $1 is the module name
  print -r -- "Building HTML Files For $1"

  sed -e "s@%ttyStyleFile%@@g" \
      -e "s@%commentWhenTty%@%@g" \
      -e "s@%commentWhenPs%@@g" \
      $1.ttytex | expand | targetVarsExpand.sh > $1.tex
  latex2html $1.tex
  echo "file:`pwd`/$1/$1.html"
}

ttytexToSimpleHtml()
{
  # $1 is the module name
  print -r -- "Building HTML Files For $1"

  sed -e "s@%ttyStyleFile%@@g" \
      -e "s@%commentWhenTty%@%@g" \
      -e "s@%commentWhenPs%@@g" \
    $1.ttytex | expand | targetVarsExpand.sh > $1.tex
  latex2html -no_navigation -info "" announce.tex
  echo "file:`pwd`/$1/$1.html"
}



putItOnTheWeb()
{
if [ "${targetDistPasswd}" = "" ] ; then
    EH_problem "Must Specify the FTP User Passwd"
    exit 1
fi

ftp -n -i << _EOF_
open ftp.neda.com
user ${targetDistUser} ${targetDistPasswd}
binary
hash
cd ~ftp/pub/products/MobileMsgWare/WinCE/${targetProdStatus}
lcd  `DOS_toFrontSlash ${distBaseDir}`
put ${exeZipFileName}
put ${tarFileName}
pwd
ls -l
close
quit
_EOF_
}

DVIPS="dvips -f"

buildUptoAction ()
{
  
  # Conventional Build
  # ${action} says what the target is
  # $* says what the subjects are
  for subject in ${G_argv}
  do
    TM_trace "${subject}";

    subjectBaseName=`FN_prefix ${subject}`
    subjectExtensionName=`FN_extension ${subject}`
    TM_trace 9 "Subject Name is  ${subjectBaseName}"
    TM_trace 9 "Subject Extension is  ${subjectExtensionName}"
	
    case ${subjectExtensionName} in
      'ttytex')
	case ${action} in
	  'tty')
	    ttytexToTtyDvi ${subjectBaseName}
            dvi2txt ${subjectBaseName}.dvi > ${subjectBaseName}.tty
	    ls -l ${subjectBaseName}.tty
	    ;;
	  'dvi')
	    ttytexToDvi ${subjectBaseName}
	    ;;
	  'xdvi')
	    ttytexToDvi ${subjectBaseName}
            xdvi ${subjectBaseName}.dvi
	    ;;
	  'ps')
	    ttytexToDvi ${subjectBaseName}
            ${DVIPS} ${subjectBaseName}.dvi > ${subjectBaseName}.ps
	    ;;
	  'pdf')
	    ttytexToDvi ${subjectBaseName}
            ${DVIPS} ${subjectBaseName}.dvi > ${subjectBaseName}.ps
	    echo "Creating  ${subjectBaseName}.pdf"
	    distill < ${subjectBaseName}.ps > ${subjectBaseName}.pdf
	    ;;
	  'lpr')
	    ttytexToDvi ${subjectBaseName}
            ${DVIPS} ${subjectBaseName}.dvi > ${subjectBaseName}.ps
	    echo "Printing ${subjectBaseName}.ps"
	    lpr ${subjectBaseName}.ps 
	    ;;
	  'html')
	    ttytexToHtml ${subjectBaseName}
	    ;;
	  'simplehtml')
	    ttytexToSimpleHtml ${subjectBaseName}
	    ;;
	  *)
	    EH_problem "Unknown: ${i}"
	    exit 1
	    ;;
	esac
	;;
      'texinfo')
	;;
      'tex')
	EH_oops "NOTYET"
	exit
	;;
      *)
	EH_problem "Unknown: ${i}"
	exit 1
	;;
    esac

  done
}


# Now do it all

    logActivitySeparator


case ${action} in
  'clean') 
    cleanKeepResults 
    ;;
  'realclean') 
    realclean 
    ;;
  'dvi')
    buildUptoAction
    ;;
  'xdvi')
    buildUptoAction
    ;;
  'tty')
    buildUptoAction
    ;;
  'ps'|'pdf')
    buildUptoAction
    ;;
  'lpr')
    buildUptoAction
    ;;
  'html'|'simplehtml')
    buildUptoAction
    ;;
  *)
    EH_problem "Unknown action: ${action}"
    exit 1
    ;;
esac

