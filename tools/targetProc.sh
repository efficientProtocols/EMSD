#!/bin/osmtKsh
#!/bin/osmtKsh

typeset RcsId="$Id: targetProc.sh,v 1.3 2002/10/08 19:04:44 mohsen Exp $"

if [ "${loadFiles}X" == "X" ] ; then
    seedActions.sh -l $0 "$@"
    exit $?
fi

. ${CURENVBASE}/tools/targetProc.libSh

function vis_examples {
  typeset visLibExamples=`visLibExamplesOutput ${G_myName}`
 cat  << _EOF_
EXAMPLES:
${visLibExamples}
--- Information
${G_myName} -i mkpParamsFilesReGen
${G_myName} -i mkpParamsFilesGen
${G_myName} -v -i mkpParamsFilesReGen
_EOF_
}


function noArgsHook {
  vis_examples
}

function vis_mkpParamsFilesReGen {
  mkpParamsFilesReGen || return
}

function vis_mkpParamsFilesGen {
  mkpParamsFilesGen || return
}
