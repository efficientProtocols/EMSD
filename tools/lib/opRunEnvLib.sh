#!/bin/ksh

#
# RCS Revision: $Id: opRunEnvLib.sh,v 1.10 2003/05/28 22:11:25 mohsen Exp $
#

# NOTYET, add opHostid based on hostid
#

function opRunEnvGet {
  # Requires no arguments
  # return Value: 0 if host information was complete and consistant
  #               1 if host not found
  #


  if [ "${opRunEnvStoreFile}_" == "_" ] ; then
    EH_problem "opRunEnvStoreFile=${opRunEnvStoreFile}"
    return 1
  fi

  if [ -f ${opRunEnvStoreFile} ] ; then
    . ${opRunEnvStoreFile}


    # For backwards compatibility
    #
    #opSiteName=${opRunSiteName}
    #opClusterName=${opRunClusterName}
    #opDomainName=${opRunDomainName}
    #opHostName=${opRunHostName}
    #opHostMode=${opRunHostMode}

    return 0
  else
    EH_problem "use opSetIdentities.sh to set /etc/osmtInfo"
    return 1
  fi

}

function opRunUserGet {
  #set -x
  typeset cmdId=""

  if [[ "${opRunOsType}_" == "_" ]] ; then 
    opRunOsType=`uname -s`
  fi

  if [[ "${opRunOsType}_" == "SunOS_" ]] ; then 
    cmdId=/usr/xpg4/bin/id
  else
    cmdId=id 
  fi

  # NOTYET, based on UID range, determine acctType
  opRunAcctType=employee

  if [[ "${opRunAcctName}_" == "_" ]] ; then 
    opRunAcctName=`${cmdId} -u -n`
    export opRunAcctName
  fi

  opThisAcctName=`${cmdId} -u -n`
  export opThisAcctName

}



function opRunEnvPrint {
  #opRunEnvGet

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

function opRunEnvSet {

  # $1 = hostName 
  # return Value: 0 if host located
  #               1 if host not found
  #
  # opSiteName, opClusterName, ... 
  # could have been set through the command line
  
  # set -x

  typeset gotVal=0
  typeset retVal=0
  typeset siteName=""
  typeset opHostDesc=""
  typeset i=""

  if  test $# -ne 1 ; then
    EH_problem "$0 requires one arg: Args=$*"
    return 1
  fi


  if [ "${opSiteName}X" == "X" ] ; then
    gotVal=0
    siteName=`opSites.sh givenHostWhichSites ${1}` || gotVal=$?
    if [  ${gotVal} -ne 0 ] ; then
      EH_problem "${1} not found and opSiteName not specified"
      return 1
    fi
    opSiteName=${siteName}
  fi

  siteParam="-p opSiteName=${opSiteName}"

  if [ "${opClusterName}X" != "X" ] ; then
    siteParam="${siteParam} -p opClusterName=${opClusterName}"
  fi
  
  if [ "${opHostMode}X" != "X" ] ; then
    siteParam="${siteParam} -p 	opHostMode=${opHostMode}"
  fi

  gotVal=0
  opHostDesc=`opHosts.sh ${siteParam} -i isValidHost $1` || gotVal=$?
  
  if [  ${gotVal} -ne 0 ] ; then
    EH_problem "${1} opHosts: ${opHostDesc}"
    return 1
  fi

  # Since NO /etc/osmtInfo in place, environment variable set.
  export opRunSiteName=${opSiteName}

  for i in ${opHostDesc}
  do
    # NOTYET, In next release of KSH this can be done with typeset -n
    leftSide=`ATTR_leftSide ${i}`
    rightSide=`ATTR_rightSide ${i}`

    #TM_trace 7  "${leftSide}=${rightSide}"

    if [ "opClusterName" == ${leftSide} ] ; then
      opRunClusterName=${rightSide}
    elif [ "opHostMode" == ${leftSide} ] ; then
      opRunHostMode=${rightSide}
    else
      EH_problem "${leftSide}=${rightSide}"
    fi
  done

  opRunHostName=${1}

  opRunOsType=`uname -s`
  opRunOsRev=`uname -r`
  opRunMachineArch=`uname -m`

  g_opRunParamStandaloneValidList="strict clustered"
  g_opRunParamStandaloneDefault="strict"

  if [ "${opParamStandalone}_" == "_" ] ; then
    opRunParamStandalone=${g_opRunParamStandaloneDefault}
  else
    gotVal=0
    IS_inList "${opParamStandalone}" "${g_opRunParamStandaloneValidList}" || gotVal=$?
    if [ ${gotVal} != 0 ] ; then
      EH_problem "${opParamStandalone} not supported in {g_opRunParamStandaloneValidList}"
      return 1
    else
      opRunParamStandalone=${opParamStandalone}
    fi
  fi

  opNetCfg_paramsGet ${opRunClusterName} ${opRunHostName}
  opRunDomainName=${opNetCfg_domainName}

  return 0

}


function opRunEnvOutput {
#set -x
  typeset gotVal=0

  gotVal=0
  opRunEnvSet $* || gotVal=$?
  if [  ${gotVal} -ne 0 ] ; then
    EH_problem "opRunEnvSet $* failed."
    return 1
  fi
  opRunEnvPrint
}


function opRunEnvVerify
{
  # Readin file and verify that it is consistent
  # with what is configured in siteControl and the system
  # 
  # return 0:  if consistent
  # retunr 1:  if inconsistent
  #

  typeset retVal=0

  opRunEnvGet

  # First Verify the easy ones
  #
  if [ "X${opRunHostName}" != "X`uname -n`" ] ; then
    print "opRunHostName=${opRunHostName} uname -n says: `uname -n`"
    retVal=1
  fi

  if [ "X${opRunOsType}" != "X`uname -s`" ] ; then
    print "opRunOsType=${opRunOsType} uname -s says: `uname -s`"
    retVal=2
  fi

  if [ "X${opRunOsRev}" != "X`uname -r`" ] ; then
    print "opRunOsRev=${opRunOsRev} uname -r says: `uname -r`"
    retVal=3
  fi

  if [ "X${opRunMachineArch}" != "X`uname -m`" ] ; then
    print "opRunMachineArch=${opRunMachineArch} uname -m says: `uname -m`"
    retVal=4
  fi
   
  if [ "X${retVal}" == "X0" ] ; then
    print "Verified -- All parameters are consistent \ncat /etc/osmtInfo" 
  fi

  return ${retVal}
}

