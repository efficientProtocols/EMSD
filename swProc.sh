#!/bin/osmtKsh
#!/bin/osmtKsh

typeset RcsId="$Id: swProc.sh,v 1.4 2002/10/10 20:15:53 mohsen Exp $"
if [ "${loadFiles}X" == "X" ] ; then
  seedSwProc.sh -l $0 "$@"
  exit $?
fi

typeset HERE=`pwd`

#. ${HERE}/tools/ocp-lib.sh
#. ${HERE}/tools/buildallhead.sh

#typeset itemOrderedList=("src" "openSrc")
OrderedArray itemOrderedList=("src")
#echo $PATH

function item_src {
  itemPre_swProc

  iv_swProc_dirName=src

  itemPost_swProc
}

function item_openSrc {
  itemPre_swProc

  iv_swProc_dirName=openSrc

  itemPost_swProc
}
