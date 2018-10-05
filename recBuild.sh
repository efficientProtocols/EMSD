#!/bin/osmtKsh
#!/bin/osmtKsh

typeset RcsId="$Id: recBuild.sh,v 1.12 2003/04/15 23:22:49 mohsen Exp $"

if [ "${loadFiles}X" == "X" ] ; then
    seedActions.sh -l $0 "$@"
    exit $?
fi


function vis_examples {
  typeset visLibExamples=`visLibExamplesOutput ${G_myName}`
 cat  << _EOF_
EXAMPLES:
${visLibExamples}
--- NOTYET FEATURES
${G_myName} -i cleanKeepResults
${G_myName} -i showSummary
${G_myName} -i showLog
${G_myName} -i showFailures
--- FULL ACTIONS
${G_myName} -i fullBuild &
${G_myName} -i fullClean &
_EOF_
}

function noArgsHook {
  vis_examples
}

function vis_fullClean {
  vis_fullAction fullClean
    #FN_rmIfThere ${CURENVBASE}/tools/buildEnv.sh ${CURENVBASE}/tools/mkpDefault.parSh  ${CURENVBASE}/tools/target.sh ${CURENVBASE}/tools/targetFamily.sh

}

function vis_fullBuild {
  vis_fullAction fullBuild
}

function vis_fullAction {
  # $1 = action to be taken
  EH_assert [[ $# -eq 1 ]]

  if [[ "${CURENVBASE}_" == "_" ]] ; then
    EH_problem "You should dot/source ./setenv.sh first"
    return 1
  fi

  typeset here=`pwd`
  if [[ "${CURENVBASE}_" != "${here}_" ]] ; then
    EH_problem "CURENVBASE mismatch -- ${CURENVBASE} != ${here}"
  fi

  # ./tools/target.sh 
  if [[ -e ${CURENVBASE}/tools/target.sh ]] ; then
    ANT_raw "Using ${CURENVBASE}/tools/target.sh"
  else
    ANT_raw "Generating target.sh"
    ${CURENVBASE}/tools/targetSelect.sh -i defaultGen    
  fi


  typeset dateTag=`date +%y%m%d%H%M%S`
  typeset OUTFILE=Bout.`uname -n`.$dateTag
  typeset ERRFILE=${OUTFILE}

  echo "To see the results"
  echo "tail -f ${OUTFILE}"
  echo "tail -f ${ERRFILE}"

  echo "Begin Date: `date`" 1> ${OUTFILE}
  typeset orderedListStr=`./swProc.sh -i orderedList`
  ./swProc.sh  ${orderedListStr} -a $1 1>> ${OUTFILE} 2>> ${ERRFILE} 
  echo "End Date: `date`"  1>> ${OUTFILE}
}

