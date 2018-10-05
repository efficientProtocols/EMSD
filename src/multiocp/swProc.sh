#!/bin/osmtKsh
#!/bin/osmtKsh

typeset RcsId="$Id: swProc.sh,v 1.5 2002/10/10 20:15:56 mohsen Exp $"

if [ "${loadFiles}X" == "X" ] ; then
  seedSwProc.sh -l $0 "$@"
  exit $?
fi

typeset HERE=`pwd`

#. ${HERE}/tools/ocp-lib.sh
#. ${HERE}/tools/buildallhead.sh

OrderedArray itemOrderedList=("base_ocp" "net_ocp" "sysif_ocp")

function item_base_ocp {
  itemPre_swProc
  iv_swProc_dirName=base.ocp
  itemPost_swProc
}

function item_net_ocp {
  itemPre_swProc
  iv_swProc_dirName=net.ocp
  itemPost_swProc
}

function item_sysif_ocp {
  itemPre_swProc
  iv_swProc_dirName=sysif.ocp
  itemPost_swProc
}

