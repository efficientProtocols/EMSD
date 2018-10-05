#!/bin/osmtKsh
#!/bin/osmtKsh

typeset RcsId="$Id: swProc.sh,v 1.6 2002/10/25 19:52:24 mohsen Exp $"

if [ "${loadFiles}X" == "X" ] ; then
  seedSwProc.sh -l $0 "$@"
  exit $?
fi
#set -x
typeset HERE=`pwd`

. ${CURENVBASE}/tools/targetProc.libSh

buildEnvAnalyze
targetParamsAnalyze


case ${targetOS} in
  'Linux'|'SunOS')
        OrderedArray itemOrderedList=("erop_eng"        \
                                      "erop_sh"         \
                                      "prvdr_sh"        \
                                      "user_sh"         \
                                      "user_sh_cb"      \
                                      "prvdr"           \
                                      "ops_xmpl_cb"     \
                                      "pdu_log"         \
                                      "ops_xmpl"        \
                                      "ops_xmpl_cb_one" \
                                      "ops_xmpl_one" )
	# 
	# NOTYET, "stress_cb" "stress" "stress_cb_one" "stress_one" "tester"
	#
	 ;;
  WINNT)
        OrderedArray itemOrderedList=("erop_eng"        \
                                      "erop_sh"         \
                                      "prvdr_sh"        \
                                      "user_sh"         \
                                      "user_sh_cb"      \
                                      "prvdr"           \
                                      "ops_xmpl_cb"     \
                                      "pdu_log"         \
                                      "ops_xmpl"        \
                                      "ops_xmpl_cb_one" \
                                      "ops_xmpl_one" )
	 ;;
  WCE3)
        OrderedArray itemOrderedList=("erop_eng"        \
                                      "erop_sh"         \
                                      "prvdr_sh"        \
                                      "user_sh"         \
                                      "user_sh_cb"      \
                                      "prvdr"           \
                                      "ops_xmpl_cb"     \
                                      "pdu_log"         \
                                      "ops_xmpl"        \
                                      "ops_xmpl_cb_one" \
                                      "ops_xmpl_one" )
	 ;;

  *)
     EH_problem "${targetOS}"
     return 1
     ;;
esac

function item_erop_eng {
  itemPre_swProc
  iv_swProc_dirName=erop_eng
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_erop_sh {
  itemPre_swProc
  iv_swProc_dirName=erop_sh
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_prvdr {
  itemPre_swProc
  iv_swProc_dirName=prvdr
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_prvdr_sh {
  itemPre_swProc
  iv_swProc_dirName=prvdr_sh
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_user_sh {
  itemPre_swProc
  iv_swProc_dirName=user_sh
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_user_sh_cb {
  itemPre_swProc
  iv_swProc_dirName=user_sh.cb
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_ops_xmpl_cb {
  itemPre_swProc
  iv_swProc_dirName=ops_xmpl.cb
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_pdu_log {
  itemPre_swProc
  iv_swProc_dirName=pdu_log
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_stress_cb {
  itemPre_swProc
  iv_swProc_dirName=stress.cb
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_ops_xmpl {
  itemPre_swProc
  iv_swProc_dirName=ops_xmpl
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_ops_xmpl_cb_one {
  itemPre_swProc
  iv_swProc_dirName=ops_xmpl.cb.one
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_ops_xmpl_one {
  itemPre_swProc
  iv_swProc_dirName=ops_xmpl.one
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_stress {
  itemPre_swProc
  iv_swProc_dirName=stress
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_stress_cb_one {
  itemPre_swProc
  iv_swProc_dirName=stress.cb.one
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_stress_one {
  itemPre_swProc
  iv_swProc_dirName=stress.one
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_tester {
  itemPre_swProc
  iv_swProc_dirName=tester
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}
