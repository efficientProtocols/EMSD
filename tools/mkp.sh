#!/bin/ksh
#
# RCS Revision: $Id: mkp.sh,v 1.27 2003/04/15 23:22:56 mohsen Exp $
#

#
# Description:
#
# This is the make pre-processor program written in the  shell.
# 
#   For Usage, see the usage function.
#
#   mkp defines a group of parameters and passes them to make.
#   SWITCHES:
#       -f <filename> (or flavor name)
#           source <filename>. priore to invoking make.
#           If -f is not present and Env exists in the current directory
#           it is sourced. 
#       -- 
#           argument parsing is terminated remainder of command line is passed
#           to make as is.
#
#   CONFIGURATION PARAMETERS:
#       - MKPFILE  Environment Variable
#       - mkpDefault.parSh
#           NOTYET, Describe how that file is generated from newTarget.sh
#
#   MKP SPECIAL TARGETS:
#	make
#	install
#	clean
#   first the target is built, then additional appropriate action is taken.
#
#
# Program Defaults
#


#
# Author: Mohsen Banan.
# 
#

OSMTROOT=${CURENVBASE}/tools

if [ -f ${OSMTROOT}/bin/opConfig.sh ] ; then
  .  ${OSMTROOT}/bin/opConfig.sh 
else
  echo "${OSMTROOT}/bin/opConfig.sh not found.\n"
  exit 1
fi

. ${opBase}/lib/ocp-lib.sh
. ${opBase}/lib/doLib.sh

opBasePathSet

export PATH="${CURENVBASE}/tools:${CURENVBASE}/tools/bin:${PATH}"

usage() {
    printf >&2 "Usage: $0 [ -T traceLevel ] [ -u ] [-f flavor]  makeOptions makeTargets\n"
    printf >&2 "Usage: flavor is one of {cc,gcc,purify}\n"
    exit 1
}

badUsage=
traceLevel=

MKPFILE=env

#
# ### VISIBLE MAKE PARAMETERS
# Default destinations:
mkpMakeProg=make	
#
TARGETS=
# MKP FLAGS
S_MKPFILE=""
# 
umask 002

while getopts f:o:T:vc:n:u? c
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
	S_MKPFILE="${S_MKPFILE} ${OPTARG}"
	;;
    v)
	G_verbose="verbose"
	;;
    c)
	G_checkMode="${OPTARG}"
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
    o)
	OpusMake="opus"
	;;
    u | \?)
	badUsage=TRUE
	;;
    esac
done

if [[ "${badUsage}_" == "TRUE_" ]] ; then
    usage
    return 1
fi

if [[ "${traceLevel}_" != "_" ]] ; then
    echo "Trace Level set to: ${traceLevel}"
fi

shift `expr $OPTIND - 1`

#
# Now process the Argumnets One by One
#

if [[ "${G_checkMode}_" != "_" ]] ; then
    export G_checkMode="${G_checkMode}"
    #print -u2 G_checkMode="${G_checkMode}"
fi

if [[ "$S_MKPFILE" != "" ]] ; then
  for i in ${S_MKPFILE} ;  do
    if test -f "$i" ;  then
      . $i	
    elif  test -f "${CURENVBASE}/tools/$i" ; then
      . ${CURENVBASE}/tools/$i
    elif  test -f "${CURENVBASE}/tools/$i.sh" ;  then
      . ${CURENVBASE}/tools/$i.sh
    else
      echo "No such flavor or file"
      return 1
    fi
  done

elif [ -f $MKPFILE ] ; then
  # This is an old convention of also using the "env" file
  . $MKPFILE
else
  . ${CURENVBASE}/tools/targetProc.libSh
  
  if [[ -f ${mkpDefaultFile} ]] ; then
    . ${mkpDefaultFile}
  else
    EH_problem "${mkpDefaultFile} not found"
    return 1
  fi
fi

# MKP Special targets

case $1 in
make)
    #
    TARGETS="$*"
    ;;
install)
    # Install consists of transferring all the man pages and
    # supporting data
    MANS=`$MAKE e_man`
    for i in ${MANS} ; do
        #set EXT=$i:e
        EXT=$i:e
        echo "copying $i to  $MAN_PATH/man$EXT/$i"
        cp $i $MAN_PATH/man$EXT/$i
    done
    TARGETS=install
    ;;
clean)
    #shift argv
    #
    OS=`make e_objs`
    echo "removing $OS"
    ''rm $OS
    TARGETS=clean
    # NOTYET
    exit
    ;;
*)
    TARGETS="$*"
    ;;
esac
#


function runOpusMake {
  # Then it means we are using NT/Win95/...
  echo \"CC=$CC\" \"AS=$AS\" \"LB=$LB\" \"RANLIB=$RANLIB\" \"LK=$LK\"\
    \"LOCATE=$LOCATE\"\
    \"ETAGS=$ETAGS\"\
    \"CPP_C=$CPP_C\" \"CPP_FLAGS=$CPP_FLAGS\" \"CPP_LK=$CPP_LK\"\
    \"CPP_LKFLAGS=$CPP_LKFLAGS\" \"CPP_LIBS_PATH=$CPP_LIBS_PATH\"\
    \"SHELL=$SHELL\"\
    \"MAKE=$MAKE\"\
    \"BASE=$BASE\"\
    \"CFLAGS=$CFLAGS\"\
    \"AFLAGS=$AFLAGS\"\
    \"LIBS_PATH=$LIBS_PATH\"\
    \"SYSLIBS=$SYSLIBS\"\
    \"PINEDIR=$PINEDIR\"\
    \"BIN_PATH=$BIN_PATH\"\
    \"MAN_PATH=$MAN_PATH\"\
    \"DATA_PATH=$DATA_PATH\"\
    \"RC=$RC\"\
    \"RCFLAGS=$RCFLAGS\"\
    $TARGETS > make.rsp

  case ${opRunOsType} in
    'UWIN-NT')
      typeset -H tmpMakeFile=/C/tmp/mkfile.$$
      ;;
    *)
      typeset tmpMakeFile="C:\tmp\mkfile.$$"
      ;;
  esac

  sed -e 's/\.a/\.lib/' makefile > ${tmpMakeFile}

  opDoRet ${mkpMakeProg} -a -f ${tmpMakeFile} @make.rsp 
  
  retVal=$?
  rm make.rsp
  rm ${tmpMakeFile}

  return ${retVal}
}

function runGmake {

  ANT_raw "Running $@"
  
  typeset tmpMakeFile="/tmp/mkfile.$$"

  if [[ "${CC}X" != "gccX" ]] ; then

 cat << _EOF_ > ${tmpMakeFile} 

# Compiling Rules (.c.obj is builtin)

.SUFFIXES:
.SUFFIXES: .out .ln .o .obj .c .cc .C .cpp .p .f .F .r .y .l .s .S .mod .sym .def .h .sh

.c.asm:
	\$(CC) \$(CFLAGS) -S \$(.SOURCE)


%.obj : %.c
	\$(CC)  \$(CFLAGS) -c \$<

%.obj : %.cpp
	\$(CC)  \$(CFLAGS) -c \$<

%.o : %.c
	\$(CC)  \$(CFLAGS) -c \$< 
	mv \$*.obj \$*.o

%.o : %.cpp
	\$(CC)  \$(CFLAGS) -c \$< 
	mv \$*.obj \$*.o

_EOF_

  sed -e 's/\.a/\.lib/' makefile >> ${tmpMakeFile}

  runMake ${tmpMakeFile}
  retVal=$?
  rm ${tmpMakeFile}
  return ${retVal}
else 
  runMake || return
fi
}

function runMake {
  # Suffixes should also be checked before calling
  # 

  typeset filenameArg=""
  if [[ $# -eq 1 ]] ; then
    filenameArg="-f $1"
    shift
  fi

CPP_C=cl.exe

  opDoComplain ${mkpMakeProg}\
    "SUFFIXES=$SUFFIXES"\
    "CC=$CC"\
    "AS=$AS"\
    "LB=$LB"\
    "RANLIB=$RANLIB"\
    "LK=$LK"\
    "LOCATE=$LOCATE"\
    "ETAGS=$ETAGS"\
    "CPP_C=$CPP_C"\
    "CPP_FLAGS=$CPP_FLAGS"\
    "CPP_LK=$CPP_LK"\
    "CPP_LKFLAGS=$CPP_LKFLAGS"\
    "CPP_LIBS_PATH=$CPP_LIBS_PATH"\
    "LIBS_PATH=$LIBS_PATH"\
    "SHELL=$SHELL"\
    "MAKE=$MAKE"\
    "BASE=$BASE"\
    "CFLAGS=$CFLAGS"\
    "AFLAGS=$AFLAGS"\
    "RC=$RC"\
    "RCFLAGS=$RCFLAGS"\
    "SYSLIBS=$SYSLIBS"\
    "PINEDIR=$PINEDIR"\
    "BIN_PATH=$BIN_PATH"\
    "MAN_PATH=$MAN_PATH"\
    "DATA_PATH=$DATA_PATH"\
    ${filenameArg} $TARGETS $@

  return
}

if [ "${osType}" = "SunOS" -o "${osType}" = "Linux" ] ; then

  SUFFIXES=".out .ln .o .c .cc .C .cpp .p .f .F .r .y .l .s .S .mod .sym .def .h .sh"
  
  opDoComplain runGmake
  retVal=$?
else
  if [[ "${OpusMake}X" == "opusX" ]] ; then
    runOpusMake
  else
    opDoComplain runGmake
    retVal=$?
  fi
fi

if [ "${retVal}" == "0" ] ; then
  print -u2 "SUMMARY: MKP Success in `pwd`"
else 
  print -u2 "SUMMARY: MKP Fail in `pwd`" 
  exit ${retVal}
fi

#echo "MKP: make done, status ${retVal}"

