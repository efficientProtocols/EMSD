# RCS: $Id: win32.sh,v 1.10 2002/10/25 19:38:08 mohsen Exp $
#
# This Script is patterned very much after MS Visual C win32.mak
#
# This will also support WCE
# 

MAKE=make	
#MAKE=echo
SHELL=sh.exe

. $CURENVBASE/tools/pdt-current.sh


case ${targetOS} in
'WINNT' | 'WIN95' | 'WINNT+WIN95' |  'WINCE-EMULATION')
  expectedTargetCPU=i386
  ;;
'WINCE')
  expectedTargetCPU=""
  ;;
*)
  echo "EH_problem XXX - Unknown prodFlavor: ${mkpFlavor}"
  exit 1
  ;;
esac

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

#LB="${MKSPATH}\sh.exe wcelib.ksh -c ${targetCPU}"
LB="${MKSPATH}\sh.exe win32lib.sh -c ${targetCPU} -s ${targetOS}"
RANLIB="echo Done Building"
#LK="${MKSPATH}\sh.exe wcelink.ksh -c ${targetCPU}"
LK="${MKSPATH}\sh.exe win32link.sh -c ${targetCPU} -s ${targetOS}"
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



case ${targetOS} in
'WINNT' )
  resultsOS=winnt

  DEFS="${DEFS} -DCRTAPI1=_cdecl -DCRTAPI2=_cdecl"
  DEFS="${DEFS} -D_WINNT -D_WIN32_WINNT=0x0400"
  # Multi Threaded NT
  DEFS="${DEFS} -D_MT"

  case ${targetPDT} in
    'BASEUA.ALL' )
      INCL="-I."
      INCL="${INCL} -I${KSHCURENVBASE}\\\\this\\\\include"
      INCL="${INCL} -I${KSHCURENVBASE}\\\\include\\\\dos"
      INCL="${INCL} -I${KSHCURENVBASE}\\\\mts_ua\\\\include"
      INCL="${INCL} -I${KSHCURENVBASE}\\\\esros\\\\include"
      INCL="${INCL} -I${KSHCURENVBASE}\\\\ocp\\\\include"
      INCL="${INCL} -Ic:\\\\devstudio\\\\vc\\\\include"
      ;;
    'OCPDEVL.MUL' )
      INCL="-I."
      INCL="${INCL} -I${KSHCURENVBASE}\\\\src\\\\this\\\\include.nt4"
      INCL="${INCL} -I${KSHCURENVBASE}\\\\src\\\\multiocp\\\\sysif.ocp\\\\include.nt4"
      INCL="${INCL} -I${KSHCURENVBASE}\\\\src\\\\multiocp\\\\include.ocp"
      INCL="${INCL} -I${KSHCURENVBASE}\\\\src\\\\esros\\\\include"
      INCL="${INCL} -ID:\\\\devstudio60\\\\vc\\\\include"
      ;;
    'esrosSDK')
      INCL="-I."
      INCL="${INCL} -I${CURENVBASE}\\\\include -I${CURENVBASE}\\\\include\\\\esros"
      INCL="${INCL} -I${CURENVBASE}\\\\include\\\\sysif"
      ;;
    'VORDE' )

      INCL="-I."
      INCL="${INCL} -I${KSHCURENVBASE}\\\\src\\\\this\\\\include.nt4"
      INCL="${INCL} -I${KSHCURENVBASE}\\\\src\\\\multiocp\\\\sysif.ocp\\\\include.nt4"
      # NOTEYT -- Figure a way of amking this independent
      #INCL="${INCL} -Id:\\\\Dialogic3.1\\\\INC"
      INCL="${INCL} -I${DialogicBase}\\\\INC"

      INCL="${INCL} -I${KSHCURENVBASE}\\\\src\\\\mts_ua\\\\include"

      INCL="${INCL} -I${KSHCURENVBASE}\\\\src\\\\esros\\\\include"

      INCL="${INCL} -I${KSHCURENVBASE}\\\\src\\\\multiocp\\\\include.ocp"
      INCL="${INCL} -Ic:\\\\devstudio\\\\vc\\\\include"
      ;;
    'UADEVL.MUL' )
      INCL="-I."
      INCL="${INCL} -I${KSHCURENVBASE}\\\\src\\\\multiocp\\\\this\\\\include.nt4"
      INCL="${INCL} -I${KSHCURENVBASE}\\\\src\\\\multiocp\\\\sysif.ocp\\\\include.nt4"
      INCL="${INCL} -I${KSHCURENVBASE}\\\\src\\\\mts_ua\\\\include"
      INCL="${INCL} -I${KSHCURENVBASE}\\\\src\\\\esros\\\\include"
      INCL="${INCL} -I${KSHCURENVBASE}\\\\src\\\\multiocp\\\\include.ocp"
      INCL="${INCL} -Ic:\\\\devstudio\\\\vc\\\\include"
      ;;
     * )
      echo "$0: invalid Target CPU: ${targetCPU}"
      exit 1
      ;;
  esac


  RCFLAGS="${RCFLAGS} -DWIN32 -D_WIN32 -DWINVER=0x0400"

  # NOTYET HACK
  DEFS="${DEFS} -DWINDOWS"
  ;;
'WIN95' )
  resultsOS=winnt

  DEFS="${DEFS} -D_WIN95 -D_WIN32_WINDOWS=0x0400"
  ;;
'WINCE' )
  resultsOS=wce

  DEFS="${DEFS} -D_WIN32_WCE"
  DEFS="${DEFS} -DWINCE"
  DEFS="${DEFS} -DUNICODE"

  case ${targetPDT} in
    'BASEUA.ALL' )
      INCL="-I."
      INCL="${INCL} -I${KSHCURENVBASE}\\\\this\\\\include"
      INCL="${INCL} -I${KSHCURENVBASE}\\\\include\\\\dos"
      INCL="${INCL} -I${KSHCURENVBASE}\\\\mts_ua\\\\include"
      INCL="${INCL} -I${KSHCURENVBASE}\\\\esros\\\\include"
      INCL="${INCL} -I${KSHCURENVBASE}\\\\ocp\\\\include"
      INCL="${INCL} -Ic:\\\\devstudio\\\\wce\\\\include"
      INCL="${INCL} -Ic:\\\\devstudio\\\\vc\\\\include"
      ;;
    'OCPDEVL.MUL' )
      INCL="-I."
      INCL="${INCL} -I${KSHCURENVBASE}\\\\src\\\\multiocp\\\\this\\\\include.dos"
      INCL="${INCL} -I${KSHCURENVBASE}\\\\src\\\\multiocp\\\\sysif.ocp\\\\include.dos"
      INCL="${INCL} -I${KSHCURENVBASE}\\\\src\\\\multiocp\\\\include.ocp"
      INCL="${INCL} -Ic:\\\\devstudio\\\\wce\\\\include"
      INCL="${INCL} -Ic:\\\\devstudio\\\\vc\\\\include"
      ;;
    'UADEVL.MUL' )
      INCL="-I."
      INCL="${INCL} -I${KSHCURENVBASE}\\\\src\\\\multiocp\\\\this\\\\include.dos"
      INCL="${INCL} -I${KSHCURENVBASE}\\\\src\\\\multiocp\\\\sysif.ocp\\\\include.dos"
      INCL="${INCL} -I${KSHCURENVBASE}\\\\src\\\\mts_ua\\\\include"
      INCL="${INCL} -I${KSHCURENVBASE}\\\\src\\\\esros\\\\include"
      INCL="${INCL} -I${KSHCURENVBASE}\\\\src\\\\multiocp\\\\include.ocp"
      INCL="${INCL} -Ic:\\\\devstudio\\\\wce\\\\include"
      INCL="${INCL} -Ic:\\\\devstudio\\\\vc\\\\include"
      ;;
    * )
      echo "$0: invalid Target CPU: ${targetCPU}"
      exit 1
      ;;
  esac

  RCFLAGS="${RCFLAGS} -l 0x409 -d \"MIPS\" -d \"_MIPS_\" -d \"_WIN32_WCE\" -d \"DEBUG\" -I \"c:\devstudio\wce\include\""
  ;;
'WINCE-EMULATION' )
  resultsOS=wce

  DEFS="${DEFS} -D_WIN32_WCE"
  DEFS="${DEFS} -DWINCE"
  DEFS="${DEFS} -D_WIN32_WCE_EMULATION"
  DEFS="${DEFS} -DUNICODE"
  DEFS="${DEFS} -DSTRICT"

  INCL="${INCL} -Ic:\\\\devstudio\\\\wce\\\\include"
  ;;
* )
  echo "$0: invalid Target CPU: ${targetCPU}"
  exit 1
  ;;
esac




DEFS="${DEFS} -DWINVER=0x0400"

DEFS="${DEFS} -DNO_UPSHELL"
DEFS="${DEFS} -DTM_ENABLED"
DEFS="${DEFS} -DDELIVERY_CONTROL"

DEFS="${DEFS} -Fo.\\\\"
DEFS="${DEFS} -Fd.\\\\"

CFLAGS="${DEFS} ${INCL}"


PINEDIR=""


LIBS_PATH="${KSHCURENVBASE}\\\\results\\\\${resultsOS}\\\\${resultsCPU}\\\\libc"
BIN_PATH="${KSHCURENVBASE}\\\\results\\\\${resultsOS}\\\\${resultsCPU}\\\\bin"
MAN_PATH=""
DATA_PATH=""

