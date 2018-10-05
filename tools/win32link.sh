#! /bin/ksh
#
# RCS: $Id: win32link.sh,v 1.22 2003/04/15 23:32:28 mohsen Exp $
#
# Syntax:
# Description:
#
#set -x 
#echo ${CURENVBASE}
#echo ${OSMTROOT}

if [[ "${BASH_VERSION}X" != "X" ]] ; then
 . ${OSMTROOT}/lib/printBash.sh
fi

#. $CURENVBASE/tools/pdt-current.sh
. $CURENVBASE/tools/win32sdk.libSh 
. $CURENVBASE/tools/mkpDefault.parSh 

if [ -f ${OSMTROOT}/bin/opConfig.sh ] ; then
  .  ${OSMTROOT}/bin/opConfig.sh 
else
  echo "${OSMTROOT}/bin/opConfig.sh not found.\n"
  exit 1
fi

. ${opBase}/lib/ocp-lib.sh
. ${opBase}/lib/doLib.sh

badUsage=
traceLevel=

targetCPU=""    # MANDATORY
targetOS=""     # MANDATORY
programType="CONSOLE"  # GUI, DLL, OLE

while getopts t:s:o:vc:n:T:f:u? c
do
    case $c in
    o)
	targetFile=$OPTARG
	;;
    c)
	targetCPU=$OPTARG
	;;
    s)
	targetOS=$OPTARG
	;;
    f)
	targetPDT=$OPTARG
	;;
    v)
	G_verbose="verbose"
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
    t)
	programType=$OPTARG
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

if [ "${targetCPU}" = "" ] ; then
  echo "EH_OOPS"
  usage
  exit 1
fi

if [ "${targetOS}" = "" ] ; then
  echo "EH_OOPS"
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

#baseFileName=`basename $1`

#shift


print -r -- targetFile = ${targetFile}

targetCPU_analyze

echo "Linking for OS=${targetOS} CPU=${targetCPU} programType=${programType}"

targetPDT=${targetPDT}

#CURENVBASE_winVersionPATH
#WINTOOLSBASE_winVersionPATH


####### libpath begin #########
libPath=""

for i in "${LINK_PATH_LIST[@]}"; do
  libPath="${libPath} -libpath:${i}"
done

case ${targetOS} in
'WINNT' )
  resultsOS=winnt

  case ${programType} in
    'CONSOLE')   
      libList="libc.lib oldnames.lib kernel32.lib  ws2_32.lib mswsock.lib"
      libList="${libList}  advapi32.lib"
      
      LINK32_FLAGS="-debug:full -debugtype:cv -NODEFAULTLIB -incremental:yes\
       -subsystem:console,4.0"
      ;;

    'GUI')   
      libList="libc.lib oldnames.lib kernel32.lib  ws2_32.lib mswsock.lib"
      libList="${libList}  advapi32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib"
      
      LINK32_FLAGS="-debug:full -debugtype:cv -NODEFAULTLIB -incremental:yes\
       -subsystem:windows,4.0"
      ;;

    'DLL')   
      libList="libc.lib oldnames.lib kernel32.lib  ws2_32.lib mswsock.lib"
      libList="${libList}  advapi32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib"
      
      LINK32_FLAGS="-debug:full -debugtype:cv -NODEFAULTLIB -incremental:yes\
       -subsystem:windows,4.0"
      ;;

    * )
	echo "EH_PROBLEM"
	exit 1
	;;
  esac
  ;;
'WIN95' )
  resultsOS=winnt

  echo EH_problem not yet
  ;;
'WINCE' )
  resultsOS=wce

  case ${programType} in
    'CONSOLE' | 'GUI' )
      libPath="${libPath} -libpath:${DEVSTUDIOPATH_winVersion}/wce/lib/wce101/${wceLibCPU}"
      libPath="${libPath} -libpath:${DEVSTUDIOPATH_winVersion}/vc98/lib"
      libPath="${libPath} -libpath:${DEVSTUDIOPATH_winVersion}/vc98/mfc/lib"

      libList="commctrl.lib coredll.lib libcd.lib"
      sysLibList="winsock.lib"

      LINK32_FLAGS="-entry:\"WinMainCRTStartup\"  \
                    -base:\"0x00010000\"          \
                    -align:\"4096\"               \
                    -incremental:yes              \
                    -debug                        \
                    -machine:${machineCPU}        \
                    -subsystem:windowsce"
      ;;
    'DLL' )
      libPath="${libPath} -libpath:${DEVSTUDIOPATH_winVersion}/wce/lib/wce101/${wceLibCPU}"
      libPath="${libPath} -libpath:${DEVSTUDIOPATH_winVersion}/vc/lib"
      libPath="${libPath} -libpath:${DEVSTUDIOPATH_winVersion}/vc/mfc/lib"

      libList="commctrl.lib coredll.lib libcd.lib msgstore.lib"
      sysLibList="winsock.lib"

      LINK32_FLAGS="-entry:\"_DllMainCRTStartup\"\
       -base:\"0x00010000\" -align:\"4096\" -dll -incremental:yes\
       -debug -machine:${machineCPU}\
       -subsystem:windowsce"
      ;;
    * )
      echo "$0: (2) invalid Target CPU: ${targetCPU}"
      exit 1
      ;;
  esac
  ;;
'wce3' )

  resultsOS=ppc3
  wce3Tools_setPATH
  WINCEROOT_winVersion=`cygpath -w  $WINCEROOT`
  SDKROOT_winVersion=`cygpath -w  $SDKROOT`

  case ${programType} in
    'CONSOLE' | 'GUI' )
       libPath="${libPath} -libpath:${SDKROOT_winVersion}/${OSVERSION}/${PLATFORM}/lib/${sdkCPU}"
       libPath="${libPath} -libpath:${SDKROOT_winVersion}/${OSVERSION}/${PLATFORM}/MFC/lib/${sdkCPU}"
       libPath="${libPath} -libpath:${SDKROOT_winVersion}/${OSVERSION}/${PLATFORM}/ATL/lib/${sdkCPU}"

       case ${sdkCPU} in 
	 'arm')
	     libList="aygshell.lib commctrl.lib coredll.lib "
	     sysLibList="winsock.lib"

	     LINK32_FLAGS="-nologo         \
	      -entry:\"WinMainCRTStartup\" \
	      -base:\"0x00010000\"         \
	      -align:\"4096\"              \
	      -incremental:no              \
	      -nodefaultlib                \
	      -debug                       \
	      -machine:${machineCPU}       \
	      -subsystem:windowsce"
	     ;;
	 'x86em')
              libList="corelibc.lib coredll.lib aygshell.lib"
	      
	      LINK32_FLAGS="-nologo         \
	       -entry:\"WinMainCRTStartup\" \
	       -base:\"0x00010000\"         \
	       -align:\"4096\"              \
	       -incremental:no              \
	       -nodefaultlib                \
	       -debug                       \
	       -machine:${machineCPU}       \
	       -subsystem:windows"
	      ;;
       esac
      ;;
    'DLL' )
      libPath="${libPath} -libpath:${DEVSTUDIOPATH_winVersion}/wce/lib/wce101/${wceLibCPU}"
      libPath="${libPath} -libpath:${DEVSTUDIOPATH_winVersion}/vc/lib"
      libPath="${libPath} -libpath:${DEVSTUDIOPATH_winVersion}/vc/mfc/lib"

      libList="commctrl.lib coredll.lib libcd.lib msgstore.lib"
      sysLibList="winsock.lib"

      LINK32_FLAGS="-entry:\"_DllMainCRTStartup\" \
                    -base:\"0x00010000\"          \
                    -align:\"4096\"               \
                    -dll                          \
                    -incremental:yes              \
                    -debug                        \
                    -machine:${machineCPU}        \
                    -subsystem:windowsce"
      ;;
    * )
      echo "$0: (2) invalid Target CPU: ${targetCPU}"
      exit 1
      ;;
  esac
  ;;
'WINCE-EMULATION' )
  resultsOS=wce

  ;;
* )
  echo "$0: (3) invalid Target CPU: ${targetCPU}"
  exit 1
  ;;
esac

echo TARGETPDT=${targetPDT}


  case ${targetPDT} in
     'VORDE' | 'VoRDE-MulPub')
       # d:/Dialogic3.1/lib  -- NOTYET
       libList="${libList} kernel32.lib user32.lib gdi32.lib winspool.lib uuid.lib libdtimt.lib libdxxmt.lib libsrlmt.lib sctools.lib libfaxmt.lib"
       #libList="${libList} kernel32.lib user32.lib gdi32.lib winspool.lib uuid.lib"
      ;;
     'OCPDEVL.MUL' | 'esrosSDK' | 'ocpBase' )

        case ${machineCPU} in
	  'arm')
             libList="${libList}"
	     ;;
	  *)
	     libList="${libList} kernel32.lib user32.lib gdi32.lib winspool.lib uuid.lib \
	      kernel32.lib user32.lib gdi32.lib winspool.lib uuid.lib"
	     
	     #libList="${libList} kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ${LibList} kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib"
	     ;;
	esac
       ;;
     * )
      echo "$0: Target PDT: ${targetPDT} Ignored"
      ;;
  esac



for d in $* 
do
	#echo "$d";
	baseFileName=`basename $d`
	prefixFileName=${baseFileName%%\.*}
	suffixFileName=${baseFileName##*\.}

	case ${suffixFileName} in
	'o')
	    objList="${objList} ${baseFileName}"
	    ;;
	'obj')
	    objList="${objList} ${baseFileName}"
	    ;;
	'rsc')
	    objList="${objList} ${baseFileName}"
	    ;;
	'lib')
    	    libList="${libList} ${baseFileName}"
	    ;;
	'a')
	    # Convert Connonical .a libraries to .lib
    	    libList="${libList} ${prefixFileName}.lib"
	    ;;
	*)
	    echo ignoring ${baseFileName}
	    ;;
	esac
done

LINK32="link.exe"

#NOLOGO="-nologo"
NOLOGO=""


#-out:`basename ${targetFile} .exe`.exe

case ${programType} in
  'CONSOLE' | 'GUI' )
#### OLD MODEL: generate a .rsp file ####
#     cat > $$.rsp << __EOF__
# ${NOLOGO}
# ${LINK32_FLAGS}
# -pdb:`basename ${targetFile} .exe`.pdb
# -out:${targetFile}.exe
# ${libPath}
# ${objList}
# ${libList}
# ${sysLibList}
# __EOF__
opDoComplain eval "${LINK32} ${NOLOGO} ${LINK32_FLAGS} -pdb:`basename ${targetFile} .exe`.pdb -out:${targetFile}.exe ${libPath} ${objList} ${libList} ${sysLibList}"
    ;;

  'DLL' )
#### OLD MODEL: generate a .rsp file #####
#     cat > $$.rsp << __EOF__
# ${NOLOGO}
# ${LINK32_FLAGS}
# -pdb:`basename ${targetFile} .dll`.pdb
# -def:`basename ${targetFile} .dll`.def
# -implib:`basename ${targetFile} .dll`.lib
# -out:${targetFile}
# ${libPath}
# ${libList}
# ${objList}
# ${sysLibList}
# __EOF__
opDoComplain eval "${LINK32} ${NOLOGO} ${LINK32_FLAGS} -pdb:`basename ${targetFile} .dll`.pdb -def:`basename ${targetFile} .dll`.def -implib:`basename ${targetFile} .dll`.lib -out:${targetFile} ${libPath} ${libList} ${objList} ${sysLibList}"
    ;;
  * )
    echo "$0: (4) invalid Target CPU: ${targetCPU}"
    exit 1
    ;;
esac

#opDoComplain ${LINK32} @$$.rsp

# NOTYET -- We need to test for exit status here

#rm -- $$.rsp
