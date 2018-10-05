#!/bin/osmtKsh
#!/bin/osmtKsh

typeset RcsId="$Id: runScript.sh,v 1.1 2002/11/05 06:34:55 mohsen Exp $"

if [ "${loadFiles}X" == "X" ] ; then
    seedActions.sh -l $0 "$@"
    exit $?
fi

. ${CURENVBASE}/tools/targetProc.libSh


function vis_examples {
  typeset visLibExamples=`visLibExamplesOutput ${G_myName}`
  typeset tehranIntra=192.168.0.38
    AfrasiabInra=192.168.0.144
    FarhadInra=192.168.0.110
    roknabadInra=192.168.0.238

 cat  << _EOF_
EXAMPLES:
${visLibExamples}
--- FULL ACTIONS
${G_myName} -i runPerformer
${G_myName} -i invokeTo ${tehranIntra}
${G_myName} -i showLogs
_EOF_
}

function noArgsHook {
  vis_examples
}

  targetParamsAnalyze || return 
  targetParamsDerivedAnalyze || return 


function vis_runPerformer {
  EH_assert [[ $# -eq 1 ]]

  typeset TRACEOPTIONS="-T G_,ffff  -T ESRO_,ffff  -T LOPS_,ffff -T EROP_,ffff -T FSM_,01 -T UDP_,ffff  -T SCH_,01 -T DU_,0400"

  opDoComplain ${target_binPath}/performer.cb.one -c erop.ini -l 15 ${TRACEOPTIONS} || return
  #${BINDIR}/performer.cb.one -c erop.ini -l 15 -o testPDUlog.010726.log ${TRACEOPTIONS}
}


function vis_invokeTo {
  EH_assert [[ $# -eq 1 ]]

  typeset TRACEOPTIONS="-T G_,ffff  -T ESRO_,ffff  -T LOPS_,ffff -T EROP_,ffff -T FSM_,01 -T UDP_,ffff  -T SCH_,01 -T DU_,0400"

  opDoComplain ${target_binPath}/invoker.cb.one -c erop.ini -l 14 -r 15 -p 2002 -n $1 ${TRACEOPTIONS} || return
  #${BINDIR}/invoker.cb.one -c erop.ini -l 14 -r 15 -p 2002 -n 192.168.0.105 -o testPDUlog.010726.log ${TRACEOPTIONS}
}

# NOTYET, not tested
function vis_showLogs {
  EH_assert [[ $# -eq 1 ]]
  SYSENV=${CURENVBASE}/results/systems/`uname -n`
    
  echo SYSENV=${SYSENV}

  ${target_binPath}/bin/eropscop -f ${SYSENV}/log/stress$1o.log
}
