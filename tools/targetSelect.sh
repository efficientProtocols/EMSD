#!/bin/osmtKsh
#!/bin/osmtKsh

typeset RcsId="$Id: targetSelect.sh,v 1.6 2003/03/28 23:27:24 pinneke Exp $"

if [ "${loadFiles}X" == "X" ] ; then
    seedActions.sh -l $0 "$@"
    exit $?
fi

#. ${CURENVBASE}/tools/targetProc.libSh

TagVar targetOS=""
TagVar targetArchType=""
TagVar targetProdStatus=""
TagVar targetProdRevNu=""
TagVar targetDistMethod=""
TagVar targetDistUser=""
TagVar targetName=""
TagVar buildEnvCompiler=""

function vis_examples {
  typeset visLibExamples=`visLibExamplesOutput ${G_myName}`
 cat  << _EOF_
EXAMPLES:
${visLibExamples}
--- DEFAULT ---
${G_myName} -i defaultGen
${G_myName} -v -i defaultGen
${G_myName} -v -i defaultReGen
--- CUSTOM ---
${G_myName} -v -p targetOS=wce3 -i defaultReGen
${G_myName} -v -p targetOS=winnt -p buildEnvCompiler=gcc -i defaultReGen
${G_myName} -v -p targetName=srcOpen -p targetOS=wce3 -p targetArchType=arm -p targetProdStatus=beta -p targetProdRevNu=2.3 -p targetDistMethod=net -p targetDistUser=lisa -p buildEnvCompiler=gcc -i defaultReGen
_EOF_
}


function noArgsHook {
  vis_examples
}

selectedTargetFamilyFile=${CURENVBASE}/tools/targetFamily.sh
selectedTargetFile=${CURENVBASE}/tools/target.sh
selectedBuildEnvFile=${CURENVBASE}/tools/buildEnv.sh

function parametersAnalyze {

  if [[ "${targetOS}_" == "_" ]] ; then
    targetOS="default"
  fi

  if [[ "${targetArchType}_" == "_" ]] ; then
    targetArchType="default"
  fi

  if [[ "${targetProdStatus}_" == "_" ]] ; then
    targetProdStatus="gold"
  fi

  if [[ "${targetProdRevNu}_" == "_" ]] ; then
    targetProdRevNu="default"
  fi

  if [[ "${targetDistMethod}_" == "_" ]] ; then
    targetDistMethod="local"
  fi

  if [[ "${targetName}_" == "_" ]] ; then
    targetName="srcOpen"
  fi

  if [[ "${targetDistUser}_" == "_" ]] ; then
    #print "PATH=$PATH"
    #targetDistUser=`whoami`
    targetDistUser=${opRunAcctName}
    # NOTYET, in sol2 whoami is not in the path.
    # The PATH is different from the system's PATH
    #targetDistUser="pinneke"
  fi

  if [[ "${buildEnvCompiler}_" == "_" ]] ; then
    buildEnvCompiler="default"
  fi


}

function vis_defaultGen {
  EH_assert [[ $# -eq 0 ]]

  if [[ -f ${selectedTargetFamilyFile} ]] ; then
    ANT_raw "$0: Use defaultReGen. ${selectedTargetFamilyFile}"
    return 1
  elif [[ -f ${selectedTargetFile} ]] ; then
    ANT_raw "$0: Use defaultReGen. ${selectedTargetFile}"
    return 1
  elif [[ -f ${selectedBuildEnvFile} ]] ; then
    ANT_raw "$0: Use defaultReGen. ${selectedBuildEnvFile}"
    return 1
  fi

  if [[ "${CURENVBASE}_" == "_" ]] ; then
    EH_problem "CURENVBASE not set"
    return 1
  fi

  parametersAnalyze || return

  targetFamilyName=`FN_nonDirsPart "${CURENVBASE}"`

  echo "#### Generating ${selectedTargetFamilyFile} ####"

  if [[ -f ${CURENVBASE}/tools/targets/${targetFamilyName}-${targetName}.targetFamily ]] ; then
    ANV_raw "Using ${targetFamilyName}-${targetName}.targetFamily"
    cp  ${CURENVBASE}/tools/targets/${targetFamilyName}-${targetName}.targetFamily ${CURENVBASE}/tools/targetFamily.sh
  else
    ANV_raw "Using generic-${targetName}.targetFamily"
    EH_assert [[ -f ${CURENVBASE}/tools/targets/generic-${targetName}.targetFamily ]]
    cp  ${CURENVBASE}/tools/targets/generic-${targetName}.targetFamily ${CURENVBASE}/tools
    cat ${CURENVBASE}/tools/generic-${targetName}.targetFamily | \
            sed -e "s:%targetFamilyName%:${targetFamilyName}:g" \
                -e "s:%targetName%:${targetName}:g" \
                -e "s:%targetProdStatus%:${targetProdStatus}:g" > ${selectedTargetFamilyFile}
    rm ${CURENVBASE}/tools/generic-${targetName}.targetFamily
  fi

  echo "#### Generating ${selectedTargetFile} ####"

  if [[ -f ${CURENVBASE}/tools/targets/${targetFamilyName}-${targetName}-${targetOS}.target ]] ; then
    ANV_raw "Using ${targetFamilyName}-${targetName}-${targetOS}.target"
    cp  ${CURENVBASE}/tools/targets/${targetFamilyName}-${targetName}-${targetOS}.target ${selectedTargetFile}
  else
    ANV_raw "Using generic-${targetName}.target"
    EH_assert [[ -f ${CURENVBASE}/tools/targets/generic-${targetName}.target ]]
    cp  ${CURENVBASE}/tools/targets/generic-${targetName}.target ${CURENVBASE}/tools
    cat ${CURENVBASE}/tools/generic-${targetName}.target | \
            sed -e "s:%selectedBuildEnvFile%:${selectedBuildEnvFile}:g" \
                -e "s:%targetName%:${targetName}:g"                     \
                -e "s:%targetOS%:${targetOS}:g"                         \
                -e "s:%targetArchType%:${targetArchType}:g"             \
                -e "s:%targetProdRevNu%:${targetProdRevNu}:g"           \
                -e "s:%targetDistMethod%:${targetDistMethod}:g"         \
                -e "s:%targetDistUser%:${targetDistUser}:g"             \
                -e "s:%targetProdStatus%:${targetProdStatus}:g" > ${selectedTargetFile}

    rm ${CURENVBASE}/tools/generic-${targetName}.target

    if [[ "${targetDistMethod}_" == "net_" ]] ; then
      cat  << _EOF_ >> ${selectedTargetFile}
# These definitions should come last, 
# because they use the previous definitions
#
targetFtpInstallDirRelative="products/MobileMsgWare/${targetOS}/${targetDistFileBase}/${targetProdStatus}-${RELID_REVNUM}"
targetFtpInstallDir="/h8/var/ftp/pub/${targetFtpInstallDirRelative}"
targetWebInstallDirRelative="products/${targetDistFileBase}/${targetProdStatus}-${RELID_REVNUM}/${targetOS}"
targetWebInstallDir="/usr/public/doc/web/htdocs/${targetWebInstallDirRelative}"
_EOF_
    fi
  fi

  echo "#### Generating ${selectedBuildEnvFile} ####"

  if [[ -f ${CURENVBASE}/tools/targets/${targetFamilyName}-${targetName}-${buildEnvCompiler}.buildEnv ]] ; then
    ANV_raw "Using ${targetFamilyName}-${targetName}-${buildEnvCompiler}.buildEnv"
    cp  ${CURENVBASE}/tools/targets/${targetFamilyName}-${targetName}-${buildEnvCompiler}.buildEnv ${selectedBuildEnvFile}
  else
    ANV_raw "Using generic-${targetName}.buildEnv"
    EH_assert [[ -f ${CURENVBASE}/tools/targets/generic-${targetName}.buildEnv ]]
    cp  ${CURENVBASE}/tools/targets/generic-${targetName}.buildEnv ${CURENVBASE}/tools
    cat ${CURENVBASE}/tools/generic-${targetName}.buildEnv | \
            sed -e "s:%buildEnvCompiler%:${buildEnvCompiler}:g" > ${selectedBuildEnvFile}

    rm ${CURENVBASE}/tools/generic-${targetName}.buildEnv
  fi
}

function vis_defaultReGen {
  EH_assert [[ $# -eq 0 ]]

  PN_rmIfThere ${selectedTargetFamilyFile}
  opDo rm ${selectedTargetFamilyFile}

  PN_rmIfThere ${selectedTargetFile}
  opDo rm ${selectedTargetFile}

  PN_rmIfThere ${selectedBuildEnvFile}
  opDo rm ${selectedBuildEnvFile}

  opDoComplain vis_defaultGen || return
}

