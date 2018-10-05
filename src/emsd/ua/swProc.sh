#!/bin/osmtKsh
#!/bin/osmtKsh

typeset RcsId="$Id: swProc.sh,v 1.2 2002/10/25 07:46:34 mohsen Exp $"

if [ "${loadFiles}X" == "X" ] ; then
  seedSwProc.sh -l $0 "$@"
  exit $?
fi
#set -x
typeset HERE=`pwd`

#. ${HERE}/tools/ocp-lib.sh
#. ${HERE}/tools/buildallhead.sh

OrderedArray itemOrderedList=("uacore" "uashdvem_one" "uashpine_one")

function item_uacore {
  itemPre_swProc
  iv_swProc_dirName=uacore
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_uashdvem_one {
  itemPre_swProc
  iv_swProc_dirName=uashdvem.one
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_uashpine_one {
  itemPre_swProc
  iv_swProc_dirName=uashpine.one
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

