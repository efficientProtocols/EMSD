#!/bin/osmtKsh
#!/bin/osmtKsh

typeset RcsId="$Id: swProc.sh,v 1.3 2002/10/10 20:16:00 mohsen Exp $"

if [ "${loadFiles}X" == "X" ] ; then
  seedSwProc.sh -l $0 "$@"
  exit $?
fi
#set -x
typeset HERE=`pwd`

#. ${HERE}/tools/ocp-lib.sh
#. ${HERE}/tools/buildallhead.sh

OrderedArray itemOrderedList=("genrelid" "cpright" "relidusr")

function item_genrelid {
  itemPre_swProc
  iv_swProc_dirName=genrelid
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_cpright {
  itemPre_swProc
  iv_swProc_dirName=cpright
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_relidusr {
  itemPre_swProc
  iv_swProc_dirName=relidusr
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

