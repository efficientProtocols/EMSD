#!/bin/osmtKsh
#!/bin/osmtKsh


if [ "${loadFiles}X" == "X" ] ; then
  seedActions.sh -l $0 $*
  exit $?
fi

vis_examples () {
  cat  << _EOF_
EXAMPLES:
${G_myName} -i  opNetCfg_paramsGet
${G_myName} -i opCluster_listDomains
_EOF_
}


vis_help () {
  cat  << _EOF_
Usage and testing of examples.
_EOF_
}

noArgsHook() {
  vis_examples
}

# NOTYET, just an example

vis_opNetCfg_paramsGet () {
  opNetCfg_paramsGet ${opRunClusterName} ${opRunHostName}

  echo   opNetCfg_ipAddr=${opNetCfg_ipAddr}
  echo   opNetCfg_domainName=${opNetCfg_domainName}
  echo   opNetCfg_netmask=${opNetCfg_netmask}
  echo   opNetCfg_networkAddr=${opNetCfg_networkAddr}
  echo   opNetCfg_defaultRoute=${opNetCfg_defaultRoute}
}

vis_opCluster_listDomains () {
    opCluster_listDomains ${opRunClusterName}
    echo "opCluster_domains=${opCluster_domains}"
}
