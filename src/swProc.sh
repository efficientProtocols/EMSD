#!/bin/osmtKsh
#!/bin/osmtKsh

typeset RcsId="$Id: swProc.sh,v 1.6 2003/04/14 19:14:41 pinneke Exp $"

if [ "${loadFiles}X" == "X" ] ; then
  seedSwProc.sh -l $0 "$@"
  exit $?
fi
#set -x
typeset HERE=`pwd`

#. ${HERE}/tools/ocp-lib.sh
#. ${HERE}/tools/buildallhead.sh

#typeset itemOrderedList=("src" "openSrc")
OrderedArray itemOrderedList=("multiocp" "kpathsea" "cpp" "this" "vorde" "esros" "emsd")

function item_multiocp {
  itemPre_swProc
  iv_swProc_dirName=multiocp
  itemPost_swProc
}

function item_kpathsea {
  itemPre_swProc
  if [ "${targetOS}_" == "SunOS_" -o "${targetOS}_" == "Linux_" ] ; then
    iv_swProc_dirName=kpathsea/sol2
  elif [[ "${targetOS}_" == "WINNT_" ]] ; then
    iv_swProc_dirName=kpathsea/winnt4
  fi
  iv_swProc_cmdType="mkp"
  iv_swProc_supportedTargetOs=("SunOS" "Linux" "WINNT")
  itemPost_swProc
}

function item_cpp {
  itemPre_swProc
  iv_swProc_dirName=cpp/hosted
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_this {
  itemPre_swProc
  iv_swProc_dirName=this
  itemPost_swProc
}

function item_vorde {
  itemPre_swProc
  iv_swProc_dirName=vorde
  itemPost_swProc
}

function item_esros {
  itemPre_swProc
  iv_swProc_dirName=esros
  itemPost_swProc
}

function item_emsd {
  itemPre_swProc
  iv_swProc_dirName=emsd
  itemPost_swProc
}

