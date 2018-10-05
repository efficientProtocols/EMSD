#!/bin/osmtKsh
#!/bin/osmtKsh

typeset RcsId="$Id: leapEmsdUaAdmin.sh,v 1.1 2002/11/06 04:41:32 mohsen Exp $"

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
${G_myName} -i runPerformer
${G_myName} -i invokeTo ${tehranIntra}
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


function leapEsroPrep {
    leapSwBasePrep || return
    leapRunBasePrep || return

    esroPrvdrPduLog=${leapLogBaseDir}/esrosLog.pdu 
    esroPrvdrPduErr=${leapLogBaseDir}/esrosErr.pdu

    esroPrvdrConfigFile=${leapConfigBaseDir}/erop.ini
    esroPrvdrTraceFile=${leapLogBaseDir}/esros.trace

    esroPrvdrTraceOptions="-T G_,ffff  -T ESRO_,ffff  -T LOPS_,ffff -T EROP_,ffff -T FSM_,01 -T UDP_,ffff  -T SCH_,01 -T DU_,0400"

    esroUsrTraceOptions="-T G_,ffff  -T ESRO_,ffff"
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
  pkill 'esros'
  vis_showProcs 
}

function vis_serviceStop {
    vis_killProcs
}

function vis_serviceStart {
    leapEsroPrep || return

    vis_esroIniOutput > ${esroPrvdrConfigFile}

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


function vis_runPerformer {
  EH_assert [[ $# -eq 0 ]]
  leapEsroPrep || return

 opDoComplain ${leapBinBaseDir}/performer ${esroUsrTraceOptions} -l 13 -s /tmp/SP 
}


function vis_invokeTo {
  EH_assert [[ $# -eq 1 ]]
  leapEsroPrep || return

  opDoComplain ${leapBinBaseDir}/invoker ${esroUsrTraceOptions} -l 12 -r 13 -p 2002 -s /tmp/SP -n $1 
}


function vis_esroIniOutput {
 cat  << _EOF_
# ESROS Engine configurable parameters
#

[Params]
	Port Number 	      = 2002 
	Max SAPs    	      = 200
	Max Invokes    	      = 200
	Max Reference Numbers = 256
	Pdu Size    	      = 1500

[Timer Values]
	Acknowledgement    = 100
	Roundtrip          = 1000
	Retransmit         = 2000
	Max NSDU Life Time = 3000
	Retransmit Count   = 8
_EOF_
}
