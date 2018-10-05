#!/bin/ksh

#
# RCS Revision: $Id: ocp-general.sh,v 1.40 2004/02/17 02:38:18 mohsen Exp $
#

#
# Description:
#

dateTag=`date +%y%m%d%H%M%S`


if [[ "${BASH_VERSION}X" != "X" ]] ; then
  shopt -s expand_aliases
  #shopt

  alias integer='typeset -i'
fi

if [[ "${BASH_VERSION}X" != "X" ]] ; then
  alias OrderedArray='typeset -a'
  alias Array='typeset -a'
  alias ListFuncs='typeset -F | sed -e s/"declare -f "//' 
  alias TagVar='typeset'
  alias TagVarList='typeset +f | cut -d= -f1'
else
  alias OrderedArray='typeset'
  alias Array='typeset -A'
  alias ListFuncs='typeset +f'
  alias TagVar='typeset -t'
  alias TagVarList='typeset +t' 
fi

function opBasePathSet {
  case ${opRunOsType} in
    'SunOS')
      export PATH="${opBase}/nedaBin"
      export PATH="${PATH}:${opBase}/bin:${opBase}/lib:${opBase}/usrEnv"
      export PATH="${PATH}:/opt/sfw/bin:/usr/local/bin"
      export PATH="${PATH}:/usr/5bin:/usr/ccs/bin:/usr/sbin:/bin:/usr/bin"
      if [[ "${opRunOsRev}_" == "5.7_" ]] ; then 
	# temporary, just to get perl
	export PATH="${PATH}:/usr/public/foundation/bin"
      fi
      export PATH="${PATH}:."
      ;;
    'Linux')
      export PATH="${opBase}/nedaBin"
      export PATH="${PATH}:${opBase}/bin:${opBase}/lib:${opBase}/usrEnv"
      export PATH="${PATH}:/usr/local/bin:/usr/local/sbin"
      export PATH="${PATH}:/usr/5bin:/usr/ccs/bin:/usr/sbin:/bin:/sbin:/usr/bin"
      export PATH="${PATH}:."
      ;;
    'Windows_NT' | 'UWIN-NT' | 'CYGWIN_NT-5.0')
      export PATH="${opBase}/nedaBin"
      export PATH="${PATH}:${opBase}/bin:${opBase}/lib:${opBase}/usrEnv"
       export PATH="${PATH}:/bin:/usr/local/bin:/usr/bin:/usr/bin/X11:/usr/games:/sbin:/usr/sbin:/usr/local/sbin"
      ;;
    *)
      EH_problem "$0 not Suported on ${opRunOsType}"
      ;;	
  esac

#   if [[ "${CURENVBASE}_" != "_" ]] ; then
#     export PATH="${CURENVBASE}/tools:${CURENVBASE}/tools/bin:${PATH}"
#   fi
}


function hookRun {
  typeset hookFuncName=""

  #EH_assert [[ $# -eq 1 ]]

  hookFuncName=`ListFuncs | egrep "^$1$"`

  if [ "${hookFuncName}X" != "X" ] ;   then
    ${hookFuncName}
  fi
}


# Mail Address Parsing

function MA_domainPart {
  print ${1##*@}
}

function MA_localPart {
 print ${1%%@*}
}

# AttrType=AttrValue Parsing  "leftSide=rightSide"

ATTR_leftSide() {
    # Escapes all non alphanumeric characters.
    typeset key=`echo "$1" | perl -p -e 'if (/([^=]+)=(.+)*/) { my $a=$1; $a =~ s/([^A-Za-z0-9])/\\\\$1/g; $_=$a; } else { die "Cannot find key"; }'` || exit
    #print ${1%%=*}
    print $key
}

ATTR_rightSide() {
    #print ${1#*=}
    # Escapes all non alphanumeric characters.
    typeset value=`echo "$1" | perl -p -e 'if (/([^=]+)=(.+)/) { my $a=$2; $a =~ s/([^A-Za-z0-9])/\\\\$1/g; $_=$a; } else { $_=""; }'` || exit
    print $value
}




#
# File Name Manipulation (FN_)
#
# /dirsPart/nonDirsPart
# /dirsPart/prefix.extension
# 
FN_prefix() {
  baseFileName=`basename $1`
  print ${baseFileName%%\.*}
}

FN_extension() {
  baseFileName=`basename $1`
  print ${baseFileName##*\.}
}

FN_dirsPart() {
    print `dirname $1`
}

FN_nonDirsPart() {
    print `basename $1`
}



function FN_fileDefunctMake {
  # $1 is the file that we no longer want active in the system
  # $2 is the name under which we want to keep it as

  if test -f $1 -o -L $1
  then
    if test -f $2
    then
	# The target should not have existed before
	print "$2 exists: skiping defuncting of $1"
    else
      	mv $1 $2
	if [ ! -L $2 ] ; then
	  # Only if not a symbolic link
	  chmod 0000 $2
	fi
	ls -l $2
    fi
  else
    print "No $1: skiping defuncting of $1"
  fi

}

function FN_dirDefunctMake {
  # $1 is the directory that we no longer want active in the system
  # $2 is the name under which we want to keep it as

  if test -d $1 ; then
    if test -d $2 ; then
      # The target should not have existed before
      print "$2 exists: skiping defuncting of $1"
    else
      mv $1 $2
      if [ ! -L $2 ] ; then
	# Only if not a symbolic link
	chmod 0000 $2
      fi
      ls -ld $2
    fi
  else
    print "No $1: skiping defuncting of $1"
  fi

}

FN_FileCreateIfNotThere() {
    # $1 is the directory that will be created, if not there

    if [ !  -f "${1}" ] ;  then
	touch ${1}
    fi
}

FN_dirCreateIfNotThere() {
    # $1 is the directory that will be created, if not there

    if test -d "${1}"
    then
	#ls -ld ${1}
	#print "${1} exists"
	return 1
    else
	mkdir "${1}"
	#print "${1} created"
	#ls -ld ${1}
	return 0
    fi
}

FN_dirCreatePathIfNotThere() {
    # $1 is the directory that will be created, if not there

    if test -d "${1}"
    then
	#ls -ld ${1}
	#print "$0: ${1} exists"
	return 1
    else
	mkdir -p "${1}"
	#print "${1} created"
	#ls -ld ${1}
	return 0
    fi
}

FN_fileSymlinkSafeMake() {
  # $1 is the source/origin  name (should exist)
  # $2 is the target name (gets created)

  if  test $# -ne 2 ; then
    EH_problem "$0 requires two args: Args=$*"
    return 1
  fi

  if test -a $1 ;  then
    if test -a $2 ;  then
	# The target should not have existed before
	print "$2 exists: skiping $0 of $1"
	ls -l $2
    else
      	ln -s $1 $2
	ls -l $2
    fi
  else
    print "No $1: skiping $0 of $1"
  fi

}


FN_fileSymlinkUpdate() {
  # $1 is the source/origin  name (should exist)
  # $2 is the target name (gets created)

  if  test $# -ne 2 ; then
    EH_problem "$0 requires two args: Args=$*"
    return 1
  fi


  if test -a $1 ;   then  # Src file exists
    if test  -h $2 ;     then
      print "$2 already a symlink, removing it"
      ls -l $2
      /bin/rm $2
      print "running: ln -s $1 $2 "
      ln -s $1 $2
    elif test -a $2 ; then
      # The target should not have existed before
      print "$2 exists: skiping $0 of $1"
      ls -l $2
    else
      ln -s $1 $2
      ls -l $2
    fi
  else
    print "No $1: skiping $0 of $1"
  fi
}

function FN_pathAbsoluteNameGet {
  # $1 is the absolute path dir

  if [[ -a $1 ]] ; then
    typeset dirsPart=`FN_dirsPart $1`
    print ${dirsPart}
    return 0
  else
    EH_problem "$1 does not exist"
    return 1
  fi
}

function FN_fileSymlinkEndGet {

  # $1 is origin file name

  if  test $# -ne 1 ; then
    EH_problem "$0 requires one args: Args=$*"
    return 1
  fi

  if [[ -L $1 ]] ; then
    typeset linksEnd=`ls -l ${1} | cut -d '>' -f2 | cut -d' ' -f2`
    integer x=`expr ${linksEnd} : "/*"`
    if [[ $x -eq 0 ]] ; then
      linksEnd=`FN_pathAbsoluteNameGet $1`/${linksEnd}
    fi
    if [[ -L ${linksEnd} ]] ; then
      FN_fileSymlinkEndGet ${linksEnd}
      return 0
    else
      print "${linksEnd}"
      return 0
    fi
  elif [[ -f $1 ]] ; then
    print $1
    return 1    
  else
    EH_problem "Not a symlink and does not exist"
    return 1
  fi
}

function FN_multiArchSymLinks {

  if  test $# -ne 3 ; then
    EH_problem "$0 requires three args: Args=$@"
    return 1
  fi

  typeset originPoint=${1}
  typeset intermediatePoint=${2} 
  typeset endPoint=${3}

  if  [[ ! -a ${endPoint} ]] ; then
    EH_problem "${endPoint} Does Not Exist"
    return 1
  fi

  mkdir -p `FN_dirsPart ${intermediatePoint}`
  FN_fileSymlinkUpdate  ${endPoint}  ${intermediatePoint}

  if test -L ${originPoint} ; then
    linksEnd=`ls -l ${originPoint} | cut -d '>' -f2 | cut -d' ' -f2`
      
    TM_trace 7 "link dest= ${linksEnd}  expected= ${intermediatePoint}"

    if [[ "${linksEnd}" != "${intermediatePoint}" ]] ; then
      EH_problem "Updating Symlink: ${originPoint} ${intermediatePoint}"
      FN_fileSymlinkUpdate  ${intermediatePoint} ${originPoint} 
    fi
  elif  test -a ${originPoint} ; then
      # A file exists -- We cant make a link

    print "PROBLEM: `ls -ld ${originPoint}` \n EXISTS, WATCHOUT!"
    return 1
  else
    FN_fileSymlinkSafeMake ${intermediatePoint} ${originPoint} 
  fi
}


function FN_fileSafeCopy {
  # $1 is the source name
  # $2 is the target name

  if  test  $# -ne 2 ; then
    EH_problem "$0 requires two args: Args=$*"
    return 1
  fi

  if test -f $1 ;  then
    if test -f $2 ; then
	# The target should not have existed before
	print "$2 exists: Skiping $0 of $1 $2"
    else
      	cp -p $1 $2
	ls -l $2
    fi
  else
    print "No $1: Skipping $0 of $1 $2"
  fi

}

opVerbose() {
  return 0
}

FN_fileCopy() {
  # $1 is the source name
  # $2 is the target name

  if test $# -ne 2 ; then
    EH_problem "$0 requires two args: Args=$*"
    return 1
  fi

  if test -f $1 ;  then
    if test -f $2 ; then
	TM_trace 7 "$2 exists: overwriting it"
    fi

    cp -p $1 $2
    if opVerbose ; then
      ls -l $2
    fi
  else
    print "No $1: Skipping $0 of $1 $2"
  fi

}

function FN_fileSafeKeep {
  # $* is list of the source name
  typeset thisOne=""
  for thisOne in $* ;  do
    if test -f ${thisOne} -o -L ${thisOne} ;   then
      if [ "${dateTag}X" == "X" ] ; then 
	#TM_trace 7 "not found"
	dateTag=`date +%y%m%d%H%M%S`
      fi
      if [[ "${G_verbose}_" == "verbose_" ]] ; then
	print "$0: ${thisOne}.${dateTag}"
      fi
      mv ${thisOne} ${thisOne}.${dateTag}
    fi
  done
}

function FN_dirSafeKeep {
  # $1 is the source name

  #EH_assert [[ $# -eq 1 ]]

  if test -d $1 ;   then
    if [ "${dateTag}X" == "X" ] ; then 
      #TM_trace 7 "not found"
      dateTag=`date +%y%m%d%H%M%S`
    fi
    mv $1 $1.${dateTag}
  else
     if [[ "${G_verbose}_" == "verbose_" ]] ; then
	print -u2 "$1: not a directory, $0 skipped"
     fi
  fi
}

FN_lineIsInFile() {
    # $1 is string to check for
    # $2 is the file
    #
  # returns 0 if line Is In File 
  # returns 1 if line In NOT in file.

  #print  "$1"
  #print  "$2"

    #NOTYET, Verify number of args. Should be three

    FN_egrep "$1" "$2" > /tmp/$$id

    if [ -s /tmp/$$id ]
    then
	#print "String $1 already in $2"
	return 0
    else
	return 1
    fi
    /bin/rm /tmp/$$id
}

FN_lineRemoveFromFile() {
    # $1 is string to check for
    # $2 is the file
    #
  # returns 0 if line Is In File 
  # returns 1 if line In NOT in file.

  #print  "$1"
  #print  "$2"

  #NOTYET, Verify number of args. Should be three
  
  dateTag=`date +%y%m%d%H%M%S`
  cp ${2} ${2}.$dateTag
  FN_egrep -v "$1" "$2" > /tmp/$$id
  /bin/cp /tmp/$$id $2
  /bin/rm /tmp/$$id
  return 0
}

FN_blockRemoveFromLine() {

  # This function will remove message block
  # from a file
  # NOTYET TESTING.  MAYBE NOT WORKING

  # cat $3 | sed '/$1/,/$2/d' > /tmp/${3}.$$

  # $1 = start of the string (regex mode)
  # $2 = end of string
  # $3 = file name


  cat $3 | sed /$1/,/$2/ d > /tmp/${3}.$$
  mv /tmp/${3}.$$ $3

}

FN_lineAddToFile() {
    # $1 is string to check for
    # $2 is the string to add at the end of the fie
    # $3 is the file
    #

  #print  "$1"
  #print  "$2"
  #print  "$3"
    
    #NOTYET, Verify number of args. Should be three

    FN_egrep "$1" "$3" > /tmp/$$id

    if [ -s /tmp/$$id ]
    then
	#print "String $1 already in $3"
	#cat /tmp/$$id
	#print "$0: skiping it"
	return 1
    else
	#print "Adding $2 to $3"
	#print $2 >> $3
        print $2 >> $3
	return 0
    fi
    /bin/rm /tmp/$$id
}

function FN_textReplace {
#
# $1 -- regexp of text to replace.
# $2 -- replacement text
# $3 is the file
# To replace a line, make sure $1 is of this format: ^text.*$
# 
  print  "$1"
  print  "$2"
  print  "$3"
    
    #NOTYET, Verify number of args. Should be three
    typeset replacee=`echo "$2"|perl -p -e 's/\//\\\\\\//g' ` #escape the '/'
    perl -pi -e "s/$1/$replacee/" "$3" 2> /dev/null
    perl -p -e "/$replacee/ and die \"123FOUND321\"" "$3" 2>&1 | 
    grep "123FOUND321" > /dev/null
}


function FN_textReplaceOrAdd {
#
# $1 -- regexp of text to replace.
# $2 -- replacement text
# $3 is the file
# To replace a line, make sure $1 is of this format: ^text.*$
# 

    if [[ $# -ne 3 ]] ; then
	EH_problem "Bad usage: $# expected 3"
	return 1
    fi
    FN_lineIsInFile "$1" "$3"
    if [[ $? == 0 ]] ; then
	FN_textReplace "$1" "$2" "$3" || return $?
    else
	( print "$2" >> "$3" ) || return $?
    fi
}

function FN_fileInstall {
#
# Ensure that we use FSF's install command
# Options are passed straight to install
# Tips: common options: -m mode -u user -g group frompath topath
#

    typeset cmdPath=""
    if [[ "X${opRunOsType}" == "XSunOS" ]] ; then
	cmdPath="/opt/sfw/bin/install"
    elif [[ "X${opRunOsType}" == "XLinux" ]] ; then
	cmdPath="install"
    fi
    ${cmdPath} "$@"
}    

function FN_grep {
  #
  # Ensure that we use grep command that supports 
  # "-F", "-v", and "-q"
  #

  typeset cmdPath=""
  if [[ "X${opRunOsType}" == "XSunOS" ]] ; then
    cmdPath="/usr/xpg4/bin/grep"
  elif [[ "X${opRunOsType}" == "XLinux" ]] ; then
    cmdPath="grep"
  fi
  ${cmdPath} "$@"
}    

function FN_egrep {
  #
  # Ensure that we use egrep command that supports 
  # "-v", "-q"
  #

  typeset cmdPath=""
  if [[ "X${opRunOsType}" == "XSunOS" ]] ; then
    cmdPath="/usr/xpg4/bin/grep"
  elif [[ "X${opRunOsType}" == "XLinux" ]] ; then
    cmdPath="grep"
  fi
  ${cmdPath} "-E" "$@"
}


function FN_tempFile {
    typeset tmp="/tmp/${G_myName}.$$.$RANDOM"
    #while [[ -e "$tmp" ]] ; do
    while test -e "$tmp"  ; do
	tmp="/tmp/${G_myName}.$$.$RANDOM"
    done
    print -- "$tmp"
}
    
function FN_fileIsAbsolutePath {

  # $1 = given file

  typeset givenFile=$1

  typeset after=${givenFile#/}

  if [[ "${givenFile}" != "${after}" ]] ; then
    TM_trace 7 "${givenFile} is an absolute path"
    return 0
  else
    return 1
  fi

}

USER_isInPasswdFile() {
    # $1 is string to check for
    # $2 is the string to add at the end of the fie
    # $3 is the file
    #

    #NOTYET, Verify number of args. Should be three

    FN_egrep "^$1:" /etc/passwd > /tmp/$$id

    if [ -s /tmp/$$id ]
    then
	print "User -- $1 -- Is In /etc/passwd file"
       /bin/rm /tmp/$$id
	return 0
    else
	print "User -- $1 -- Is NOT In /etc/passwd file"
	/bin/rm /tmp/$$id
	return 11
    fi

}

USER_loginGivenHomeDir() {
    # $1 is Home Dir
    # outputs loginName and returns 0 if found
    # returns 1 if not found

    #NOTYET, Verify number of args. Should be three

  #TM_trace 7 "$*"

     loginName=`cat /etc/passwd | FN_egrep ":$1:" | cut -d':' -f 1` 

       if [ "${loginName}X" == "X" ] ; then 
	 #TM_trace 7 "not found"
	 return 1
       else
	 print ${loginName}
	 return 0
	 fi

}


USER_nextLoginNameGet() {
  # $1 category sa-, qvd-

  # nextLoginNumber

  return 11
}


PN_fileVerify () {
  ls -l $1
}

function FN_fileRmIfThere {
	#print $*
	#typeset -ft PN_rmIfThere
	PN_rmIfThere $*
}

PN_rmIfThere()
{
	rmIfThereVerbose=FALSE

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
	if [ ${rmIfThereVerbose} == TRUE ] ; then 
	  print Removing $f
	fi
	/bin/rm $f
      else
	#TM_trace 9 -- Skipping $f
	# NOTYET, TM_trace is not working ...
	rmIfThereVerbose=${rmIfThereVerbose}
	#print ""
      fi
  done

}


function IS_inList {
  # $1=target --  $2=validList

    if [[ "$1_" == "_" ]] ; then
	return 1
    fi

    if [[ "$2_" == "_" ]] ; then
	return 1
    fi


  typeset retVal=1
  typeset i=""
  typeset allInput="$@"
  typeset validList=`echo ${allInput} | cut -d ' ' -f2-${#allInput}`
  #print $validList
  for i in $validList ;  do
    if [[ "${i}_" == "$1_" ]] ; then
      #print "_${i}_" "_$1_"
      retVal=0
      break
    fi
  done

  
  return ${retVal}
}

function LIST_isIn {
  # $1=target --  $2=validList
  if [[ $# -ne 2 ]] ; then
    print -u2 "$0: expected 2 args, got $#"
    return 1
  fi

  typeset retVal=1
  typeset i=""

  #print $2
  for i in $2 ;  do
    if [[ "${i}_" == "$1_" ]] ; then
      #print "_${i}_" "_$1_"
      retVal=0
      break
    fi
  done
  
  return ${retVal}
}



function LIST_getLast {
  #set -x
  if test $# -ge 1 ; then
    shiftValue=`expr $#  - 1`
    #print ${shiftValue} $* >> /tmp/ZZ
    shift ${shiftValue} 
    #print ${shiftValue} $* >> /tmp/ZZ
  fi
  print $*
}

function LIST_getFirst {
  print $1
  return 
}

function LIST_set {
  activeList=$*
  #print ${activeList}
}

function LIST_minus {
  newList=""
  for subjectInList in ${activeList} ; do
    if [ "${1}X" != "${subjectInList}X" ] ; then
      newList="${newList} ${subjectInList}"
    fi
  done
  activeList=${newList}
  #print ${activeList}
}

function LIST_setMinusResult {
  print ${activeList}
}

# ksh syntax sometimes demands something
# so, we provide doNothing
doNothing() {
  typeset x=""
}

# doStderrToStdout can be used as a one-liner to ignore stdout and 
# replace stderr with stdout. No tmp files.

# set -x
# if [[ "${BASH_VERSION}X" == "X" ]] ; then
# function doStderrToStdout {
#   integer retVal=0

#   if [[ $# -lt 1 ]] ; then
#     EH_problem "$0 requires at least one arg: Args=$*"
#     return 1
#   fi

#                        # Commented for pedagogical purposes

#   cat |&               # co-process, to take in stderr  
#   exec 2>&p            # our stderr is now co-process's input 
#   $* 1>/dev/null       # run the command, ignore stdout, stderr goes to co-process
#   retVal=$?            # keep exit code of the command
#   exec 2>&-            # close our stderr stream
#   exec 0<&p            # co-process's stdout becomes our stdin
#   cat                  # we put our stdin to our stdout
#   return ${retVal}     # we return the exit code of the command as exit code
# }
# fi




G_validateOption()
{
  # $1=target
  # $2=validList

  targetIsValid="FALSE"

  for i in $2
  do
    #TM_trace 9 $i
    if [ "${i}" = "$1" ] ; then
      #TM_trace 9 "target=$1"
      targetIsValid="TRUE"
      break
    fi
  done

  if [ "${targetIsValid}" = "FALSE" ] ; then
    #EH_problem "XXX - Unknown archType: ${archType}"
    print "Unknown option: ${1}"
    usage
    exit 1
  fi
}

# Verify We are running Solaris or Linux
G_abortIfNotSupportedOs() {
  osType=`uname -s`
  case ${osType} in
    'SunOS')
      #TM_trace 7 "$0 running on ${osType} ...."
      ;;
    'Linux')
      #TM_trace 7 "$0 running on ${osType} ...."
      ;;
    'CYGWIN_NT-5.0')
      #TM_trace 7 "$0 running on ${osType} ...."
      ;;

    # NOTYET, Linux to be added
    *)
      uname
	print "$0 not Suported on ${osType}"
	exit
  esac

}

G_abortIfNotRunningAsRoot() {
  # Verify Being Run As Root

  id | grep root > /tmp/$$id

  if [ ! -s /tmp/$$id ]
  then
    id
    print "$0 should be run as root"
    exit 1
  fi
  /bin/rm /tmp/$$id
}

G_returnIfNotRunningAsRoot() {
  # Verify Being Run As Root

  id | grep root > /tmp/$$id

  if [ ! -s /tmp/$$id ]
  then
    print "$0 should be run as root"
    id
    return 1
  fi
  /bin/rm /tmp/$$id
}


G_validateRunOS()  
{
  # $1=validList

  runOsType=`uname -s`

  isValid="FALSE"

  for i in $1
  do
    #TM_trace 9 $i
    if [ "${i}" = "${runOsType}" ] ; then
      isValid="TRUE"
      break
    fi
  done

  if [ "${isValid}" = "FALSE" ] ; then
    print "Can't RUN this script on ${runOsType}"
    exit 1
  fi
}


# Usage: opDoRet opLinuxOnly || return $?

function opLinuxOnly {
  case ${opRunOsType} in
  "Linux")
     return 0
     ;;
  *)
     EH_problem "Unsupported OS: ${opRunOsType}"
     return 1
     ;;
  esac
}

function opSolarisOnly {
  case ${opRunOsType} in
  "SunOS")
     return 0
     ;;
  *)
     EH_problem "Unsupported OS: ${opRunOsType}"
     return 1
     ;;
  esac
}

DOS_toFrontSlash() {
  print -r -- $1 | sed -e "s@\\\\@/@g"
}

DOS_toBackSlash() {
  print NOTYET
}

RELID_extractInfo () {
  # $1 is RELID_RELINFO_FILE

  RELID_PROD_NAME=`grep RELID-INFO-Product-Name $1 | cut -d: -f2`
  RELID_REVNUM=`grep RELID-INFO-Rev-Number $1 | cut -d$ -f2 | cut -d' ' -f 2`
  #RELID_REVNUM="0.${RELID_REVNUM}"
  RELID_REL_DATE=`grep RELID-INFO-Rel-Date $1 | cut -d$ -f2 | cut -d' ' -f 2,3`
  RELID_CVS_TAG=`grep RELID-INFO-CVS-Tag $1 | cut -d$ -f2 | tr -s ' ' | cut -d' ' -f 2`
  RELID_BUILD_USER=${USER}
  RELID_BUILD_HOST="`uname -n` `uname -svrm`"
  RELID_BUILD_DATE=`date | tr -s ' '`

  if [ "${RELID_PROD_NAME}" = "" ]
  then
    RELID_PROD_NAME="product unspecified"
  fi

  if [ "${RELID_REVNUM}" = "" ]
  then
    RELID_REVNUM="revision unspecified"
  fi

  if [ "${RELID_REL_DATE}" = "" ]
  then
    RELID_REL_DATE="release date unspecified"
  fi

  if [ "${RELID_CVS_TAG}" = "" ]
  then
    RELID_CVS_TAG="CVS tag unspecified"
  fi
}


logActivitySeparator()
{
  print -r -- ""
  print -r -- "-------------------------------------------------------"
}



buildAndRecord() {
  # $1 = buildDir
  # $2 = buildCmd
  cd $1
  print -r "++++++++++ Started In `pwd` at `date`  Running: $2"
  #print "++++++++++ Started In `pwd` at `date`  Running: $2"

  $2

  if [ ${status} ] ; then
    print "XXXXXXXX build failed in `pwd` with status=${status}"
    exit 1
  fi
  print -r -- "--------- Completed In `pwd` at `date`  Running: $2"
  print ""
}



#
# TESTING
#

#print "FN_dirsPart on $0 produces: `FN_dirsPart $0`"
#print "FN_nonDirsPart on $0 produces: `FN_nonDirsPart $0`"
#print "FN_prefix on $0 produces: `FN_prefix $0`"
#print "FN_extension on $0 produces: `FN_extension $0`"


#traceLevel=9
#TM_trace 3 "hello $0-$0"

#USER_loginGivenHomeDir /acct/qmaildom/com/payk

#LOG_event "something"

#EH_problem "some prob"

#EH_fatal "die"

#print "should never see this"


#local=`MA_localPart mohsen@neda.com`
#domain=`MA_domainPart mohsen@neda.com`
#print "Local is ${local} and domain is ${domain}"


#left=`ATTR_leftSide subscribe=iran-news@lists.payk.net`
#right=`ATTR_rightSide subscribe=iran-news@lists.payk.net`
#print "Left is ${left} and Right is ${right}"

