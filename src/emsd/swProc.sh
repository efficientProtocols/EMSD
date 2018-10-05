#!/bin/osmtKsh
#!/bin/osmtKsh

typeset RcsId="$Id: swProc.sh,v 1.4 2002/10/25 17:04:37 mohsen Exp $"

if [ "${loadFiles}X" == "X" ] ; then
  seedSwProc.sh -l $0 "$@"
  exit $?
fi
#set -x
typeset HERE=`pwd`

#. ${HERE}/tools/ocp-lib.sh
#. ${HERE}/tools/buildallhead.sh

OrderedArray itemOrderedList=("dup"      \
                              "emsd"     \
                              "mtsua"    \
                              "emsdmgr"  \
                              "ua"       \
			      "mailer"   \
                              "mts"      \
                              "mts_one"  \
                              "emsdscope")

function item_dup {
  itemPre_swProc
  iv_swProc_dirName=dup
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_emsd {
  itemPre_swProc
  iv_swProc_dirName=emsd
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_emsdscope {
  itemPre_swProc
  iv_swProc_dirName=emsdscope
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_mtsua {
  itemPre_swProc
  iv_swProc_dirName=mtsua
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_ua {
  itemPre_swProc
  iv_swProc_dirName=ua
  itemPost_swProc
}

function item_emsdmgr {
  itemPre_swProc
  iv_swProc_dirName=emsdmgr
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_mailer {
  itemPre_swProc
  iv_swProc_dirName=mailer
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_mts {
  itemPre_swProc
  iv_swProc_dirName=mts
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_mts_one {
  itemPre_swProc
  iv_swProc_dirName=mts.one
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

