#!/bin/osmtKsh
#!/bin/osmtKsh

typeset RcsId="$Id: swProc.sh,v 1.3 2002/10/10 20:15:58 mohsen Exp $"

if [ "${loadFiles}X" == "X" ] ; then
  seedSwProc.sh -l $0 "$@"
  exit $?
fi
#set -x
typeset HERE=`pwd`

#. ${HERE}/tools/ocp-lib.sh
#. ${HERE}/tools/buildallhead.sh

OrderedArray itemOrderedList=("bo_" "du_" "inet" "sap_" "udp_pco" "asn" "upq_" "upq_portable")

function item_bo_ {
  itemPre_swProc
  iv_swProc_dirName=bo_/portable
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_du_ {
  iv_swProc_dirName=du_/unhosted
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_inet {
  itemPre_swProc
  iv_swProc_dirName=inet/unix
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_sap_ {
  itemPre_swProc
  iv_swProc_dirName=sap_/portable
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_udp_pco {
  itemPre_swProc
  iv_swProc_dirName=udp_pco
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_asn {
  itemPre_swProc
  iv_swProc_dirName=asn
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}

function item_upq_ {
  itemPre_swProc
  iv_swProc_dirName=upq/sol2/upq_bsd.async
  iv_swProc_cmdType="mkp"
  iv_swProc_supportedTargetOs=("SunOS" "Linux")
  itemPost_swProc
}

function item_upq_portable {
  itemPre_swProc
  iv_swProc_dirName=upq/portable/upq_simu
  iv_swProc_cmdType="mkp"
  itemPost_swProc
}
