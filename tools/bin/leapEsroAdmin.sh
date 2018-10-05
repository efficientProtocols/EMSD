#!/bin/osmtKsh
#!/bin/osmtKsh

typeset RcsId="$Id: leapEsroAdmin.sh,v 1.2 2002/11/06 06:09:14 mohsen Exp $"

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
--- FULL MANIPULATORS ---
${G_myName} ${extraInfo}  -i fullReport
${G_myName} ${extraInfo}  -i fullUpdate  
--- ENABLE/DISABLE THE ESROS SERVICE (next reboot) ---
${G_myName} ${extraInfo}  -i compEnable  
${G_myName} ${extraInfo}  -i compDisable
--- ENABLE/DISABLE THE ESROS SERVICE (one time) ---
${G_myName} ${extraInfo}  -i serviceStart
${G_myName} ${extraInfo}  -i serviceStop
${G_myName} ${extraInfo}  -i serviceRestart
--- Server Processes ---
${G_myName} -i showProcs
${G_myName} -i killProcs
--- ESRO SERVER INFO and LOGS ---
${G_myName} -n showOnly -i showPrvdrTrace
${G_myName} -i showPduLogs
--- Sample Usage ----
${G_myName} -i performer
${G_myName} -i invokeTo ${tehranIntra}
${G_myName} -i performerCB
${G_myName} -i invokeCBTo ${tehranIntra}
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


vis_fullReport() {
  vis_showProcs
}

vis_showProcs() {
  typeset -i procsNum=`pgrep  'esros' | wc -l`
  if [ "${procsNum}_" != "1_" ] ; then
     echo "Some ESRO Processes Are Missing, ${procsNum}"
     return 1
   else
     echo "All ESRO Processes Running"
  fi
  pgrep -l 'esros'
}

vis_killProcs() {
  vis_showProcs || return
  ANT_raw "About to Kill esros process"
  pkill 'esros' || return
  opDo rm /tmp/SP* /tmp/xxXxx
  vis_showProcs 
}

function vis_serviceStop {
    vis_killProcs
}

function vis_serviceStart {
    leapEsroPrep || return

    CMD="${leapBinBaseDir}/esros -o ${esroPrvdrPduLog} -e ${esroPrvdrPduErr} -c${esroPrvdrConfigFile}"

    echo $CMD
    date       > ${esroPrvdrTraceFile}
    echo $CMD >> ${esroPrvdrTraceFile}
    $CMD      >> ${esroPrvdrTraceFile} 2>&1 &

    echo "To see the logfile, run:"
    echo tail -80 ${esroPrvdrTraceFile}
    echo ""
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


function vis_performer {
  EH_assert [[ $# -eq 0 ]]
  leapEsroPrep || return

 opDoComplain ${leapBinBaseDir}/performer ${esroUsrTraceOptions} -l ${esroSapSelOpsXmplPerformer} -s ${esroPrvdrSocket}
}


function vis_invokeTo {
  EH_assert [[ $# -eq 1 ]]
  leapEsroPrep || return

  opDoComplain ${leapBinBaseDir}/invoker ${esroUsrTraceOptions} -l ${esroSapSelOpsXmplInvoker} -r ${esroSapSelOpsXmplPerformer} -p ${esroSapUdpPortPerformer} -s ${esroPrvdrSocket} -n $1 
}

function vis_performerCB {
  EH_assert [[ $# -eq 0 ]]
  leapEsroPrep || return

 opDoComplain ${leapBinBaseDir}/performer.cb ${esroUsrTraceOptions} -l 13 -s /tmp/SP 
}


function vis_invokeCBTo {
  EH_assert [[ $# -eq 1 ]]
  leapEsroPrep || return

  opDoComplain ${leapBinBaseDir}/invoker.cb ${esroUsrTraceOptions} -l 12 -r 13 -p 2002 -s /tmp/SP -n $1 
}

