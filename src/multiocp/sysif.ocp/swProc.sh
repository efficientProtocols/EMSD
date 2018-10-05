#!/bin/osmtKsh
#!/bin/osmtKsh

typeset RcsId="$Id: swProc.sh,v 1.4 2003/04/03 18:27:22 pinneke Exp $"

if [ "${loadFiles}X" == "X" ] ; then
  seedSwProc.sh -l $0 "$@"
  exit $?
fi
#set -x
typeset HERE=`pwd`

#. ${HERE}/tools/ocp-lib.sh
#. ${HERE}/tools/buildallhead.sh

OrderedArray itemOrderedList=("sf_" "os" "os_diraccess" "sch_" "tmr_" "outstrm_" "nvm_")

function item_sf_ {
  itemPre_swProc
  iv_swProc_dirName=sf_/unix
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_os {
  itemPre_swProc
  if [[ "${targetOS}_" == "SunOS_" ]] ; then
    iv_swProc_dirName=os/sol2
  elif [[ "${targetOS}_" == "Linux_" ]] ; then
    iv_swProc_dirName=os/linux
  elif [[ "${targetOS}_" == "WINNT_" ]] ; then
    iv_swProc_dirName=os/nt-4.0
  elif [ "${targetOS}_" == "wince_" -o "${targetOS}_" == "win95_" ] ; then
    iv_swProc_dirName=os/wce-1.0
  fi
  iv_swProc_cmdType="mkp"
  iv_swProc_supportedTargetOs=("SunOS" "Linux" "WINNT" "wince" "win95")
  itemPost_swProc
}

function item_os_diraccess {
  itemPre_swProc
  iv_swProc_dirName=os/win32/diraccess
  iv_swProc_cmdType="mkp"
  iv_swProc_supportedTargetOs=("WINNT")
  itemPost_swProc
}

function item_sch_ {
  itemPre_swProc
  iv_swProc_dirName=sch_/unix
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_tmr_ {
  itemPre_swProc
  if [ "${targetOS}_" == "SunOS_" -o "${targetOS}_" == "Linux_" ] ; then
    iv_swProc_dirName=tmr_/unix.djl
  elif [[ "${targetOS}_" == "WINNT_" ]] ; then
    iv_swProc_dirName=tmr_/winnt4
  elif [ "${targetOS}_" == "wince_" -o "${targetOS}_" == "win95_" ] ; then
    iv_swProc_dirName=tmr_/wce-1.0
  fi
  iv_swProc_cmdType="mkp"
  iv_swProc_supportedTargetOs=("SunOS" "Linux" "WINNT" "wince" "win95")
  itemPost_swProc
}

function item_outstrm_ {
  itemPre_swProc
  iv_swProc_dirName=outstrm_/outfile
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_nvm_ {
  itemPre_swProc
  iv_swProc_dirName=nvm_/unix
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

