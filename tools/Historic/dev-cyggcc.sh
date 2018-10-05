# RCS: $Id: dev-cyggcc.sh,v 1.1 2002/10/25 23:47:41 mohsen Exp $
#
# This Script is patterned very much after MS Visual C win32.mak
#

MAKE=make	
#MAKE=echo
SHELL=/bin/bash.exe

. $CURENVBASE/tools/pdt-current.sh

expectedTargetCPU=i386

if [ "${expectedTargetCPU}" = "i386" ] ; then
  if [ "${targetCPU}" = "" ] ; then
    targetCPU=i386
  elif [ "${expectedTargetCPU}" != "${targetCPU}" ] ; then
    echo "EH_problem ${targetOS} and ${targetCPU} are not compatible"
    exit 1
  fi
fi

case ${targetCPU} in
'SH3' | 'MIPS' | 'i386' |  'PPC')
  ;;
*)
  echo "EH_problem Unknown cpu type"
  exit 1
  ;;
esac

if [ "${targetPDT}" = "" ] ; then
  targetPDT=OCPDEVL.MUL
  #targetPDT=BASEUA.ALL
  #targetPDT=UADEVL.MUL
fi


#SHELL=/bin/sh
SHELL=/bin/ksh
MAKE=gmake	
CC=gcc
AS="as -"
LB="ar r"
RANLIB=ranlib
LK="gcc -g"
ETAGS=etags
#SYSLIBS="-lsocket -lnsl -lucb -lcurses -ltermcap"
#SYSLIBS="-lsocket -lnsl -lcurses -ltermcap /usr/ucblib/libucb.a
SYSLIBS="-lnsl -lcurses"
#
# ../results/winnt/x86/
LIBS_PATH=${CURENVBASE}/results/winnt/x86/libc
BIN_PATH=${CURENVBASE}/results/winnt/x86/bin
MAN_PATH=${CURENVBASE}/results/share/man
DATA_PATH=${CURENVBASE}/results/share/data

DEFS=""
DEFS="${DEFS} -D TM_ENABLED"
#DEFS="${DEFS} -DUCBSIGNALS -DUCBCURSES"
DEFS="${DEFS} -g  -Wall"

DEFS=""

case ${targetCPU} in
'i386' )
  DEFS="${DEFS} -D_X86_"
  DEFS="${DEFS} -DX86"

  resultsCPU="x86"
  ;;
* )
  echo "$0: invalid Target CPU: ${targetCPU}"
  exit 1
  ;;
esac

# Tools Common to all platforms

typeset CYGWIN_PATH_winVersion=`cygpath -w  ${CYGWIN_PATH} | sed -e 's:\\\\:\\\\\\\\:g'`


DEFS="${DEFS} -D_GCC_"
DEFS="${DEFS} -DDEBUG"

resultsOS=winnt

DEFS="${DEFS} -DCRTAPI1=_cdecl -DCRTAPI2=_cdecl"

DEFS="${DEFS} -D_WINNT -D_WIN32_WINNT=0x0400"

# Multi Threaded NT
DEFS="${DEFS} -D_MT"


DEFS="${DEFS} -DWINVER=0x0400"

DEFS="${DEFS} -DNO_UPSHELL"
DEFS="${DEFS} -DTM_ENABLED"
DEFS="${DEFS} -DDELIVERY_CONTROL"

CFLAGS="${DEFS} ${INCL}"



typeset CURENVBASE_winVersion=`cygpath -w  $CURENVBASE | sed -e 's:\\\\:/:g'`
typeset DEVSTUDIOPATH_winVersion=`cygpath -w  $DEVSTUDIOPATH | sed -e 's:\\\\:/:g'`
typeset DialogicBase_winVersion=`cygpath -w  $DialogicBase | sed -e 's:\\\\:/:g'`

  case ${targetPDT} in
    'BASEUA.ALL' )
      INCL="-I."
      INCL="${INCL} -I${CURENVBASE_winVersion}/this/include"
      INCL="${INCL} -I${CURENVBASE_winVersion}/include/dos"
      INCL="${INCL} -I${CURENVBASE_winVersion}/mts_ua\\include"
      INCL="${INCL} -I${CURENVBASE_winVersion}\\esros\\include"
      INCL="${INCL} -I${CURENVBASE_winVersion}\\ocp\\include"
      ;;
    'OCPDEVL.MUL' )
      INCL="-I."
      INCL="${INCL} -I${CURENVBASE_winVersion}\\src\\this\\include.nt4"
      INCL="${INCL} -I${CURENVBASE_winVersion}\\src\\multiocp\\sysif.ocp\\include.nt4"
      INCL="${INCL} -I${CURENVBASE_winVersion}\\src\\multiocp\\include.ocp"
      INCL="${INCL} -I${CURENVBASE_winVersion}\\src\\esros\\include"
      ;;
    'esrosSDK')
      INCL="-I."
      INCL="${INCL} -I${CURENVBASE}\\include -I${CURENVBASE}\\include\\esros"
      INCL="${INCL} -I${CURENVBASE}\\include\\sysif"
      ;;
    'VORDE' )
      INCL="-I."
      INCL="${INCL} -I${CURENVBASE_winVersion}/src/this/include.nt4"
      INCL="${INCL} -I${CURENVBASE_winVersion}/src/multiocp/sysif.ocp/include.nt4"
      # NOTEYT -- Figure a way of amking this independent
      #INCL="${INCL} -Id:/Dialogic3.1/INC"
      INCL="${INCL} -I${DialogicBase_winVersion}/INC"

      INCL="${INCL} -I${CURENVBASE_winVersion}/src/mts_ua/include"

      INCL="${INCL} -I${CURENVBASE_winVersion}/src/esros/include"

      INCL="${INCL} -I${CURENVBASE_winVersion}/src/multiocp/include.ocp"
      ;;
    'UADEVL.MUL' )
      INCL="-I."
      INCL="${INCL} -I${CURENVBASE_winVersion}\\src\\multiocp\\this\\include.nt4"
      INCL="${INCL} -I${CURENVBASE_winVersion}\\src\\multiocp\\sysif.ocp\\include.nt4"
      INCL="${INCL} -I${CURENVBASE_winVersion}\\src\\mts_ua\\include"
      INCL="${INCL} -I${CURENVBASE_winVersion}\\src\\esros\\include"
      INCL="${INCL} -I${CURENVBASE_winVersion}\\src\\multiocp\\include.ocp"
      ;;
     * )
      echo "$0: invalid Target CPU: ${targetCPU}"
      exit 1
      ;;
  esac



# case ${targetPDT} in
#   'esrosSDK' )
#       INCL="-I."
#       INCL="${INCL} -I${CURENVBASE}/include -I${CURENVBASE}/include/esros"
#       INCL="${INCL} -I${CURENVBASE}/include/sysif"
#       ;;
#   'OCPDEVL.MUL' )
#     INCL="-I."
#     INCL="${INCL} -I${CURENVBASE}/src/esros/include"
#     INCL="${INCL} -I${CURENVBASE}/src/this/include.linux"
#     INCL="${INCL} -I${CURENVBASE}/src/multiocp/sysif.ocp/include.linux"
#     INCL="${INCL} -I${CURENVBASE}/src/multiocp/sysif.ocp/os/linux"
#     INCL="${INCL} -I${CURENVBASE}/src/multiocp/sysif.ocp/oe/unix/sysv"
#     INCL="${INCL} -I${CURENVBASE}/src/multiocp/include.ocp"
#     ;;
#   'VORDE' )
#     INCL="-I."
#     INCL="${INCL} -I${CURENVBASE}/src/mts_ua/include"
#     INCL="${INCL} -I${CURENVBASE}/src/esros/include"
#     INCL="${INCL} -I${CURENVBASE}/src/this/include.linux"
#     INCL="${INCL} -I${CURENVBASE}/src/multiocp/sysif.ocp/include.linux"
#     INCL="${INCL} -I${CURENVBASE}/src/multiocp/sysif.ocp/os/linux"
#     INCL="${INCL} -I${CURENVBASE}/src/multiocp/sysif.ocp/oe/unix/sysv"
#     INCL="${INCL} -I${CURENVBASE}/src/multiocp/include.ocp"
#     ;;
#   'UADEVL.MUL' )
#     INCL="-I."
#     INCL="${INCL} -I${CURENVBASE}/src/this/include.linux"
#     INCL="${INCL} -I${CURENVBASE}/src/multiocp/sysif.ocp/include.linux"
#     INCL="${INCL} -I${CURENVBASE}/src/mts_ua/include"
#     INCL="${INCL} -I${CURENVBASE}/src/esros/include"
#     INCL="${INCL} -I${CURENVBASE}/src/multiocp/include.ocp"
#     ;;
#   * )
#     echo "$0: invalid Target CPU: ${targetCPU}"
#     exit 1
#     ;;
# esac


CFLAGS="${DEFS} ${INCL}"


# LIBS_PATH="${CURENVBASE_winVersion}/results/${resultsOS}/${resultsCPU}/libc"
# BIN_PATH="${CURENVBASE_winVersion}/results/${resultsOS}/${resultsCPU}/bin"
# MAN_PATH=""
# DATA_PATH=""






