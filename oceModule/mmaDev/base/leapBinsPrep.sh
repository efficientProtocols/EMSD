#!/bin/osmtKsh
#!/bin/osmtKsh 

typeset RcsId="$Id: leapBinsPrep.sh,v 1.1 2003/03/27 21:05:13 mohsen Exp $"

if [ "${loadFiles}X" == "X" ] ; then
    seedActions.sh -l $0 "$@"
    exit $?
fi

typeset leapInstallDir="/opt/public/mma/leap"

function vis_examples {
  typeset visLibExamples=`visLibExamplesOutput ${G_myName}`
  typeset extraInfo="-n showRun"
 cat  << _EOF_
EXAMPLES:
${visLibExamples}
--- INSTALL ---
${G_myName} ${extraInfo} -i compInstall
${G_myName} ${extraInfo} -f -i compInstall
${G_myName} ${extraInfo} -i compUninstall
--- INFO ---
${G_myName} ${extraInfo} -i compVerify
_EOF_
}

function vis_help {
  cat  << _EOF_
NOTYET
_EOF_
}


function noArgsHook {
    vis_examples
}

function vis_compInstall {
  integer thisRetVal=0
  vis_compVerify || thisRetVal=$?
  if [[ ${thisRetVal} -eq 0 ]] ; then
    if [[ "${G_forceMode}_" == "force_" ]] ; then
      opDoComplain compUpdateStart
    fi
  else
    opDoComplain compUpdateStart
  fi
}

function compUpdateStart {
  typeset CURRENT_DIR=`pwd`
  export BASE_CURRENT_DIR=${CURRENT_DIR%/*}
  print "set BASE_CURRENT_DIR to ${BASE_CURRENT_DIR}"
  mkdir -p ${leapInstallDir}
  print "Copying files to ${leapInstallDir}......"
  cp -rp ${BASE_CURRENT_DIR}/* ${leapInstallDir}
}

function vis_compVerify {

  if [[ -d ${leapInstallDir} ]] ; then
    print "mmaDev has been installed.."
    return 0
  else
    print "mmaDev has not been installed.."
    return 1
  fi
}

function vis_compUninstall {
  integer thisRetVal=0
  vis_compVerify || thisRetVal=$?
  if [[ ${thisRetVal} -eq 0 ]] ; then
    print "Uninstalling mmaDev....."
    opDoComplain rm -r ${leapInstallDir}
  else
    print "Skipping the uninstallation."
  fi
}

