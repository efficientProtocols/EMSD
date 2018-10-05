#!/bin/osmtKsh
#!/bin/osmtKsh

typeset RcsId="$Id: leapEsroOneAdmin.sh,v 1.2 2002/11/06 09:05:22 mohsen Exp $"

if [ "${loadFiles}X" == "X" ] ; then
    seedActions.sh -l $0 $@
    exit $?
fi


. ${opBinBase}/leapCommon.libSh

function vis_examples {
  typeset extraInfo="-v -n showRun"
  typeset tehranIntra=192.168.0.38
  typeset jamshidInra=192.168.0.144
  typeset oneRemote=${tehranIntra}
  typeset visLibExamples=`visLibExamplesOutput ${G_myName}`
 cat  << _EOF_
EXAMPLES:
${visLibExamples}
--- ESRO SERVER INFO and LOGS ---
${G_myName} -n showOnly -i showPrvdrTrace
${G_myName} -i showPduLogs
--- Sample Usage ----
${G_myName} -i performerOne
${G_myName} -i invokeOneTo ${tehranIntra}
${G_myName} -i performerOneCB
${G_myName} -i invokeOneCBTo ${tehranIntra}
_EOF_
}

vis_help () {
  return 0
}

vis_hints () {
  return 0
}

noArgsHook() {
  vis_examples
}



function vis_showPduLogs {
  EH_assert [[ $# -eq 0 ]]
  leapEsroPrep || return
  ${leapBinBaseDir}/eropscop -f ${esroPrvdrPduLog}
}


function vis_showPrvdrTrace {
  EH_assert [[ $# -eq 0 ]]
  leapEsroPrep || return
  opDo tail -80 ${esroPrvdrTraceFile}
}


function vis_performerOne {
  EH_assert [[ $# -eq 0 ]]
  leapEsroPrep || return

 opDoComplain ${leapBinBaseDir}/performer.one -c ${esroPrvdrConfigFile} -l ${esroSapSelOpsXmplPerformer} ${esroOneTraceOptions} 
}


function vis_invokeOneTo {
  EH_assert [[ $# -eq 1 ]]
  leapEsroPrep || return

  opDoComplain ${leapBinBaseDir}/invoker.one  -c ${esroPrvdrConfigFile} -l ${esroSapSelOpsXmplInvoker} -r ${esroSapSelOpsXmplPerformer} -p ${esroSapUdpPortPerformer}  -n $1 ${esroOneTraceOptions}
}

function vis_performerOneCB {
  EH_assert [[ $# -eq 0 ]]
  leapEsroPrep || return

  opDoComplain ${leapBinBaseDir}/performer.cb.one  -c ${esroPrvdrConfigFile} -l ${esroSapSelOpsXmplPerformer} ${esroOneCBTraceOptions}
}


function vis_invokeOneCBTo {
  EH_assert [[ $# -eq 1 ]]
  leapEsroPrep || return

  opDoComplain ${leapBinBaseDir}/invoker.cb.one  -c ${esroPrvdrConfigFile} -l ${esroSapSelOpsXmplInvoker} -r ${esroSapSelOpsXmplPerformer} -p ${esroSapUdpPortPerformer}  -n $1 ${esroOneCBTraceOptions}
}


