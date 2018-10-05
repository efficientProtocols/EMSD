# RCS: $Id: cygwin-opus.sh,v 1.1 2002/10/25 23:47:41 mohsen Exp $
#
# This Script is patterned very much after MS Visual C win32.mak
#
# This will also support WCE
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
fi

DEFS=""
RCFLAGS="-r"

case ${targetCPU} in
'SH3' )
  CC="shcl.exe"

  DEFS="${DEFS} -D_SH3_"
  DEFS="${DEFS} -DSH3"

  resultsCPU="sh3"
  ;;
'MIPS' )
  CC="clmips.exe"

  DEFS="${DEFS} -D_MIPS_"
  DEFS="${DEFS} -DMIPS"

  resultsCPU="mips"
  ;;
'i386' )
  CC="cl.exe"

  DEFS="${DEFS} -D_X86_"
  DEFS="${DEFS} -DX86"

  resultsCPU="x86"
  ;;
'EMULATION' )
  CC="cl.exe"

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

LB="${CYGWIN_PATH_winVersion}\\\\bash.exe win32lib.sh -c ${targetCPU} -s ${targetOS}"
RANLIB="echo Done Building"
#LK="${MKSPATH}\sh.exe wcelink.ksh -c ${targetCPU}"
LK="${CYGWIN_PATH_winVersion}\\\\bash.exe win32link.sh -c ${targetCPU} -s ${targetOS}"
RC=rc
HC="hcrtf -xn"
ETAGS=etags
AS="tasm"

# -------------------------------------------------------------------------
# Platform Dependent Compile Flags 
#
# These switches allow for source level debugging with WinDebug for local
# and global variables.
#
# Both compilers now use the same front end - you must still define either
# _X86_, _MIPS_, _PPC_ or _ALPHA_.
#
# Common compiler flags:
#   -c   - compile without linking
#   -W3  - Set warning level to level 3
#   -Zi  - generate debugging information
#   -Od  - disable all optimizations
#   -Ox  - use maximum optimizations
#   -Zd  - generate only public symbols and line numbers for debugging
#
# i386 specific compiler flags:
#   -Gz  - stdcall
#
# -------------------------------------------------------------------------

#DEFS="${DEFS} -nologo" 
DEFS="${DEFS} -W3"
DEFS="${DEFS} -Zi"
DEFS="${DEFS} -Od"

DEFS="${DEFS} -DDEBUG"

resultsOS=winnt

DEFS="${DEFS} -DCRTAPI1=_cdecl -DCRTAPI2=_cdecl"
DEFS="${DEFS} -D_WINNT -D_WIN32_WINNT=0x0400"
# Multi Threaded NT
DEFS="${DEFS} -D_MT"

typeset CURENVBASE_winVersion=`cygpath -w  $CURENVBASE | sed -e 's:\\\\:\\\\\\\\:g'`
typeset DEVSTUDIOPATH_winVersion=`cygpath -w  $DEVSTUDIOPATH | sed -e 's:\\\\:\\\\\\\\:g'`
typeset DialogicBase_winVersion=`cygpath -w  $DialogicBase | sed -e 's:\\\\:\\\\\\\\:g'`

  case ${targetPDT} in
    'BASEUA.ALL' )
      INCL="-I."
      INCL="${INCL} -I${CURENVBASE_winVersion}\\this\\include"
      INCL="${INCL} -I${CURENVBASE_winVersion}\\include\\dos"
      INCL="${INCL} -I${CURENVBASE_winVersion}\\mts_ua\\include"
      INCL="${INCL} -I${CURENVBASE_winVersion}\\esros\\include"
      INCL="${INCL} -I${CURENVBASE_winVersion}\\ocp\\include"
      INCL="${INCL} -I${DEVSTUDIOPATH_winVersion}\\VC98\\Include"
      ;;
    'OCPDEVL.MUL' )
      INCL="-I."
      INCL="${INCL} -I${CURENVBASE_winVersion}\\src\\this\\include.nt4"
      INCL="${INCL} -I${CURENVBASE_winVersion}\\src\\multiocp\\sysif.ocp\\include.nt4"
      INCL="${INCL} -I${CURENVBASE_winVersion}\\src\\multiocp\\include.ocp"
      INCL="${INCL} -I${CURENVBASE_winVersion}\\src\\esros\\include"
      INCL="${INCL} -I${DEVSTUDIOPATH_winVersion}\\VC98\\Include"
      ;;
    'esrosSDK')
      INCL="-I."
      INCL="${INCL} -I${CURENVBASE}\\include -I${CURENVBASE}\\include\\esros"
      INCL="${INCL} -I${CURENVBASE}\\include\\sysif"
      ;;
    'VORDE' )

      INCL="-I."
      INCL="${INCL} -I${CURENVBASE_winVersion}\\src\\this\\include.nt4"
      INCL="${INCL} -I${CURENVBASE_winVersion}\\src\\multiocp\\sysif.ocp\\include.nt4"
      # NOTEYT -- Figure a way of amking this independent
      #INCL="${INCL} -Id:\\Dialogic3.1\\INC"
      INCL="${INCL} -I${DialogicBase_winVersion}\\INC"

      INCL="${INCL} -I${CURENVBASE_winVersion}\\src\\mts_ua\\include"

      INCL="${INCL} -I${CURENVBASE_winVersion}\\src\\esros\\include"

      INCL="${INCL} -I${CURENVBASE_winVersion}\\src\\multiocp\\include.ocp"
      INCL="${INCL} -I${DEVSTUDIOPATH_winVersion}\\VC98\\Include"
      ;;
    'UADEVL.MUL' )
      INCL="-I."
      INCL="${INCL} -I${CURENVBASE_winVersion}\\src\\multiocp\\this\\include.nt4"
      INCL="${INCL} -I${CURENVBASE_winVersion}\\src\\multiocp\\sysif.ocp\\include.nt4"
      INCL="${INCL} -I${CURENVBASE_winVersion}\\src\\mts_ua\\include"
      INCL="${INCL} -I${CURENVBASE_winVersion}\\src\\esros\\include"
      INCL="${INCL} -I${CURENVBASE_winVersion}\\src\\multiocp\\include.ocp"
      INCL="${INCL} -I${DEVSTUDIOPATH_winVersion}\\VC98\\Include"
      ;;
     * )
      echo "$0: invalid Target CPU: ${targetCPU}"
      exit 1
      ;;
  esac


  RCFLAGS="${RCFLAGS} -DWIN32 -D_WIN32 -DWINVER=0x0400"
  
  # NOTYET HACK
  DEFS="${DEFS} -DWINDOWS"

DEFS="${DEFS} -DWINVER=0x0400"

DEFS="${DEFS} -DNO_UPSHELL"
DEFS="${DEFS} -DTM_ENABLED"
DEFS="${DEFS} -DDELIVERY_CONTROL"

DEFS="${DEFS} -Fo.\\"
DEFS="${DEFS} -Fd.\\"

CFLAGS="${DEFS} ${INCL}"


PINEDIR=""


LIBS_PATH="${CURENVBASE_winVersion}\\\\results\\\\${resultsOS}\\\\${resultsCPU}\\\\libc"
BIN_PATH="${CURENVBASE_winVersion}\\\\results\\\\${resultsOS}\\\\${resultsCPU}\\\\bin"
MAN_PATH=""
DATA_PATH=""





