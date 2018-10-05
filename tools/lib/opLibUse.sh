#!/bin/osmtKsh
#!/bin/osmtKsh


if [ "${loadFiles}X" == "X" ] ; then
  seedActions.sh -l $0 $@
  exit $?
fi

vis_examples () {
  cat  << _EOF_
==== USE OF doStderrToStdout
--- Assuming BadBad does not exist
${G_myName} -i stderrToStdout ls BadBad | grep -i BadBad
${G_myName} -i stderrToStdout ls BadBad | grep -i DontMatch
--- Empty stderr as empty stdout
${G_myName} -i stderrToStdout ls .
===== USE OF opDo
${G_myName} -i opDo true
${G_myName} -i opDoRet false
${G_myName} -n showOnly -i opDoExit ls -l /etc/motd
${G_myName} -n runShow -i opDoGoodExamples
${G_myName} -i opDoBadExamples
===== USE OF FN_symlink
${G_myName} -i FN_fileSymlinkEndGet /opt/public/mma/bin
===== LEFT and Right Parsing
${G_myName} -i ATTR_rightSide "progs=someText afterSpace xx=zzz"
${G_myName} -i ATTR_leftSide "progs=someText afterSpace xx=zzz"
===== OUTPUT 
${G_myName} -v -i outputExamples
====== Check Mode
${G_myName} -c fast -i outputExamples
_EOF_
}


function noArgsHook {
  vis_examples
}

function vis_stderrToStdout {
  doStderrToStdout $@
}

function vis_opDo {
  opDo $@
}

function vis_opDoRet {
  opDoRet $@
}

function vis_opDoExit {
  opDoRet $@
}

function vis_opDoGoodExamples {

  typeset someStr="some"
  typeset emptyStr=""

  opDoRet  eval "( cat /etc/passwd | grep mohsen )"
  #opDoRet  eval "( cat /etc/passwd > ./TTT )"
  #opDoRet eval "(  echo \"| ./mbox\" > ./ZZZ )"
  #ls -l ./ZZZ
  echo ${dryRunLevel}
  return

  opDoRet showParam "arg1" "arg2And space" "arg3"
  opDoRet eval [[ "${emptyStr}_" == "some_" ]]
  opDoRet eval [[ "${someStr}_" == "some_" ]]
  #opDoRet  cat /etc/passwd | opDoRet grep notThere; echo $?
  opDoRet  eval '(' cat /etc/passwd '|' grep notThere ')'
  opDoRet  eval "( cat /etc/passwd | grep mohsen )"


  opDoRet [[ "${emptyStr}_" == "some_" ]]
  opDoRet [ "${emptyStr}_" == "some_" ]
  opDoRet test "${emptyStr}_" == "some_"

  opDoRet [[ "${someStr}_" == "_" ]]

  opDoRet true
  opDoRet false

  opDoExit test "${emptyStr}_" == "some_"

  print -- "Not reached"
}

function vis_opDoBadExamples {
  dryRunLevel="showAndRun"
  
  typeset someStr="some"
  typeset emptyStr=""
  
  # Bad Syntax
  opDoRet [[ "${emptyStr}" == "some" ]]

  # accepted Syntax
  opDoRet [[ "${emptyStr}" == "" ]]

  # bad Syntax
  opDoRet [[ "${someStr}" == "" ]]

  opDoRet [[ "${someStr}" == "${emptyStr}" ]]


  # Bad Syntax
  opDoRet test "${emptyStr}" == "some"

  # accepted Syntax
  opDoRet test "${emptyStr}" == "" 

  # bad Syntax
  opDoRet test "${someStr}" == ""

  opDoRet test "${someStr}" == "${emptyStr}"
}


function showParam {
    typeset -i i=0
   for arg in "$@" ; do
	print "$i: $arg"
	i=i+1
    done
}

# NOTYET opDo Stuff 
# Make it an alias, pass the function, file and lineNos
# Show Pipe and (process group examples)
# Should we do it with eval or without?
# When reporting failed exit code, show exit code
#

function vis_FN_fileSymlinkEndGet {

  FN_fileSymlinkEndGet $1
}

function vis_ATTR_leftSide {
    print "$*"
  ATTR_leftSide "$*"
}

function vis_ATTR_rightSide {
    print "$*"
  ATTR_rightSide "$*"
}


function vis_outputExamples {
    ANT_cooked "by ANT_cooked:  opRunSiteName=${opRunSiteName}"
    ANV_cooked "by ANV_cooked:  opRunSiteName=${opRunSiteName}"

    ANT_raw "by ANT_raw:  opRunSiteName=${opRunSiteName}"
    ANV_raw "by ANV_raw:  opRunSiteName=${opRunSiteName}"

    stdOutput "by stdOutput:  opRunSiteName=${opRunSiteName}"
}

