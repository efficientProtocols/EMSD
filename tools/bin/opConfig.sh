

if [ "${OSMTROOT}X" == "X" ] ; then
    typeset currDir=`dirname $0`
    if [[ "${currDir}_" == "._" ]] ; then 
      currDir=`pwd`
    fi
    export OSMTROOT=${currDir%/bin}
fi

export opBase=${OSMTROOT}

export opVarBase=/var/osmt

export mmaBase=/opt/public/mma

export mmaSrcBase=/opt/public/mmaSrc

export mmaPkgBase=/opt/public/mmaPkgs

export adoptedSrcBase=/opt/public/src

export adoptedPkgBase=/opt/public/pkgs


PATH="${opBinBase}:${mmaBinDistBase}/bin:${mmaEzmlmBinDistBase}:${PATH}"

function opRunEnvOutputGeneric {
  opRunSiteName=genericSite
  opRunClusterName=genericCluster
  opRunHostMode=standalone
  opRunHostName=`uname -n`
  opRunOsType=`uname -s`
  opRunOsRev=`uname -r`
  opRunMachineArch=`uname -m`
  opRunParamStandalone=strict
  opRunDomainName=genericDomain

  cat  << _EOF_  
# THIS FILE MACHINE GENERATED with ${G_myName} and $0
opRunSiteName=${opRunSiteName}
opRunClusterName=${opRunClusterName}
opRunDomainName=${opRunDomainName}
opRunHostName=${opRunHostName}
opRunHostMode=${opRunHostMode}
# Read Only Parameters
opRunOsType=${opRunOsType}
opRunOsRev=${opRunOsRev}
opRunMachineArch=${opRunMachineArch}
# Hostwide Parameters
opRunParamStandalone=${opRunParamStandalone}
_EOF_
}

if [[ -f /etc/osmtInfo ]] ; then
    opRunEnvStoreFile=/etc/osmtInfo
else
  if [[ "${CURENVBASE}_" == "_" ]] ; then
    opRunEnvOutputGeneric > /etc/osmtInfo
    opRunEnvStoreFile=/etc/osmtInfo
    echo "Using the generic ${opRunEnvStoreFile}"
  else
    if [[ -f ${CURENVBASE}/tools/osmtInfo ]] ; then 
      opRunEnvStoreFile=${CURENVBASE}/tools/osmtInfo
    else
      opRunEnvOutputGeneric > ${CURENVBASE}/tools/osmtInfo
      opRunEnvStoreFile=${CURENVBASE}/tools/osmtInfo
      echo "Using the generic ${opRunEnvStoreFile}"
    fi
  fi
fi
