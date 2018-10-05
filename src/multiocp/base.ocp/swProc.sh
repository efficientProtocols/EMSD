#!/bin/osmtKsh
#!/bin/osmtKsh

typeset RcsId="$Id: swProc.sh,v 1.6 2003/04/03 18:27:21 pinneke Exp $"

if [ "${loadFiles}X" == "X" ] ; then
  seedSwProc.sh -l $0 "$@"
  exit $?
fi
#set -x
typeset HERE=`pwd`

#. ${HERE}/tools/ocp-lib.sh
#. ${HERE}/tools/buildallhead.sh

# NOTYET, needs to be moved

OrderedArray itemOrderedList=("pf_" "getopt" "alloc" "buf" "cfg" "eh_" "pn_" "profile" "qu_" "seq_" "str" "log_" "bs_" "fsm_" "lic_" "sbm_" "mm_" "toEng" "tm_" "vsprintf")

function item_pf_ {
  itemPre_swProc
  iv_swProc_dirName=pf_/portable
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_getopt {
  itemPre_swProc
  iv_swProc_dirName=getopt/portable
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_alloc {
  itemPre_swProc
  iv_swProc_dirName=alloc/hosted
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_buf {
  itemPre_swProc
  iv_swProc_dirName=buf/hosted
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_cfg {
  itemPre_swProc
  iv_swProc_dirName=cfg/hosted
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_eh_ {
  itemPre_swProc
  iv_swProc_dirName=eh_/unix
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_pn_ {
  itemPre_swProc
  iv_swProc_dirName=pn_/unix
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_profile {
  itemPre_swProc
  iv_swProc_dirName=profile/hosted
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_qu_ {
  itemPre_swProc
  iv_swProc_dirName=qu_/portable
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_seq_ {
  itemPre_swProc
  iv_swProc_dirName=seq_/hosted
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_str {
  itemPre_swProc
  iv_swProc_dirName=str/hosted
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_log_ {
  itemPre_swProc
  iv_swProc_dirName=log_/ontopof
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_bs_ {
  itemPre_swProc
  iv_swProc_dirName=bs_/portable
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_fsm_ {
  itemPre_swProc
  iv_swProc_dirName=fsm_/portable
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_lic_ {
  itemPre_swProc
  iv_swProc_dirName=lic_/hosted
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_sbm_ {
  itemPre_swProc
  iv_swProc_dirName=sbm_/hosted
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_mm_ {
  itemPre_swProc
  iv_swProc_dirName=mm_/hosted
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_toEng {
  itemPre_swProc
  iv_swProc_dirName=toEng/portable
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_tm_ {
  itemPre_swProc
  if [ "${targetOS}_" == "SunOS_" -o "${targetOS}_" == "Linux_" -o "${targetOS}_" == "winnt_" -o "${targetOS}_" == "WINNT_" ] ; then
    iv_swProc_dirName=tm_/hosted
  elif [[ "${targetOS}_" == "wince_" ]] ; then
    iv_swProc_dirName=tm_/wce
  fi
  iv_swProc_cmdType="mkp"
  iv_swProc_supportedTargetOs=("SunOS" "Linux" "winnt" "WINNT" "wince" "win95")
  itemPost_swProc
}

function item_vsprintf {
  itemPre_swProc
  iv_swProc_dirName=vsprintf/portable
  iv_swProc_cmdType="mkp"
  iv_swProc_supportedTargetOs=("wince")
  itemPost_swProc
}