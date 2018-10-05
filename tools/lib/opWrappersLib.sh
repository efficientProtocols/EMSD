

function opNetCfg_paramsGet {
  # clusterName=${1} -p hostName=${2}

  integer gotVal=0
  integer retVal=0

  if  test $# -ne 2 ; then
    EH_problem "Bad Args Usage: Needed 2 got $#"
    return 1
  fi
  
  if [ "${opRunOsType}_" == "SunOS_" -o "${opRunOsType}_" == "UWIN-NT_" ] ; then
    mmaLayer3Hosts.sh -p clusterName=${1} -p hostName=${2} -i givenHostGetNetworkSetting |
    while read  t_ipAddr t_domainName t_netmask t_network t_defaultRoute ; do
      gotVal=0
      i_nedaNetParamsGet ${t_ipAddr} ${t_domainName} ${t_netmask} ${t_network} ${t_defaultRoute} || gotVal=$?
      if [ ${gotVal} != 0 ] ; then
	EH_problem "clusterName=${1} hostName=${2}"
	mmaLayer3Hosts.sh -p clusterName=${1} -p hostName=${2} -i givenHostGetNetworkSetting 
	return 1
      fi
    done # of while read

  elif [ "${opRunOsType}_" == "Linux_" ] ; then
    # Unless done with a co-process args are not visible after func terminates.
    # This seems to be a bug in pdksh
    #mmaLayer3Hosts.sh -p clusterName=${1} -p hostName=${2} -i givenHostGetNetworkSetting |&
    #while read -p t_ipAddr t_domainName t_netmask t_network t_defaultRoute ; do

    mmaLayer3Hosts.sh -p clusterName=${1} -p hostName=${2} -i givenHostGetNetworkSetting |
    while read  t_ipAddr t_domainName t_netmask t_network t_defaultRoute ; do
      gotVal=0
      i_nedaNetParamsGet ${t_ipAddr} ${t_domainName} ${t_netmask} ${t_network} ${t_defaultRoute} || gotVal=$?
      if [ ${gotVal} != 0 ] ; then
	EH_problem "clusterName=${1} hostName=${2}"
	mmaLayer3Hosts.sh -p clusterName=${1} -p hostName=${2} -i givenHostGetNetworkSetting 
	return 1
      fi
    done # of while read
  else
    EH_problem "Unsupported OS: ${opRunOsType}"
  fi

  return 0
}

function i_nedaNetParamsGet {
  if  test $# -ne 5 ; then
    EH_problem "Bad Args Usage: Needed 5 got $#"
    return 1
  fi

  opNetCfg_ipAddr=${1}
  opNetCfg_domainName=${2}
  opNetCfg_netmask=${3}
  opNetCfg_networkAddr=${4}
  opNetCfg_defaultRoute=${5}
}

function opCluster_listDomains {
    # $1=clusterName

  integer gotVal=0
  integer retVal=0
  typeset  progOutPut=""

  if  test $# -ne 1 ; then
    EH_problem "Bad Args Usage: Needed 1 got $#"
    return 1
  fi
  
  gotVal=0
  progOutPut=`opClusters.sh -s $1 -a listDomains` || gotVal=$?
  if [ ${gotVal} != 0 ] ; then
    EH_problem ""
    return 1
  fi

  opCluster_domains=${progOutPut}

  return 0
}
