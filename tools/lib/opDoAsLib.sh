typeset RcsId="$Id: opDoAsLib.sh,v 1.3 2002/09/04 17:58:38 mohsen Exp $"

if [[ -z ${include_opDoAsLib} ]] ; then
    include_opDoAsLib="doted"
    
#
# opDoXxxxAs
#

function opDoAs {
  EH_assert [[ $# -gt 1 ]]
  typeset doUser=$1
  shift
  if [[ "${opRunAcctName}_" = "${doUser}_" ]] ; then
    opDo "$@"
  elif [[ "${opRunAcctName}_" = "root_" ]] ; then
    opDo sudo -u ${doUser} "$@"
  elif [[ "${doUser}_" = "root_" ]] ; then
    opDo sudo -u ${doUser} env PATH=${PATH}  "$@"
  else
    EH_problem "${opRunAcctName} cant doAs ${doUser}"
  fi
}

function opDoComplainAs {
  EH_assert [[ $# -gt 1 ]]
  typeset doUser=$1
  shift
  if [[ "${opRunAcctName}_" = "${doUser}_" ]] ; then
    opDoComplain "$@"
  elif [[ "${opRunAcctName}_" = "root_" ]] ; then
    opDoComplain sudo -u ${doUser} "$@"
  elif [[ "${opThisAcctName}_" = "root_" ]] ; then
    opDoComplain sudo -u ${doUser} "$@"
  elif [[ "${doUser}_" = "root_" ]] ; then
    opDoComplain sudo -u ${doUser} env PATH=${PATH}  "$@"
  else
    EH_problem "${opRunAcctName} cant doAs ${doUser}"
  fi
}

function opDoRetAs {
  EH_assert [[ $# -gt 1 ]]
  typeset doUser=$1
  shift
  if [[ "${opRunAcctName}_" = "${doUser}_" ]] ; then
    opDoRet "$@"
  elif [[ "${opRunAcctName}_" = "root_" ]] ; then
    opDoRet sudo -u ${doUser} "$@"
  elif [[ "${doUser}_" = "root_" ]] ; then
    opDoRet sudo -u ${doUser} env PATH=${PATH}  "$@"
  else
    EH_problem "${opRunAcctName} cant doAs ${doUser}"
  fi
}

function opDoExitAs {
  EH_assert [[ $# -gt 1 ]]
  typeset doUser=$1
  shift
  if [[ "${opRunAcctName}_" = "${doUser}_" ]] ; then
    opDoExit "$@"
  elif [[ "${opRunAcctName}_" = "root_" ]] ; then
    opDoExit sudo -u ${doUser} "$@"
  elif [[ "${doUser}_" = "root_" ]] ; then
    opDoExit sudo -u ${doUser} env PATH=${PATH}  "$@"
  else
    EH_problem "${opRunAcctName} cant doAs ${doUser}"
  fi
}

#
# opDoXxxxAsAt
#

function opDoAsAt {
  EH_assert [[ $# -gt 2 ]]
  typeset doUser=$1
  shift
  typeset doHost=$2
  shift
  if [[ "${opRunAcctName}_" = "${doUser}_" ]] ; then
    opDo ssh ${doHost} "$@"
  elif [[ "${opRunAcctName}_" = "root_" ]] ; then
    opDo sudo -u ${doUser} ssh ${doHost} "$@"
  else
    EH_problem "${opRunAcctName} cant doAs ${doUser}"
  fi
}

function opDoComplainAsAt {
  EH_assert [[ $# -gt 2 ]]
  typeset doUser=$1
  shift
  typeset doHost=$2
  shift
  if [[ "${opRunAcctName}_" = "${doUser}_" ]] ; then
    opDoComplain ssh ${doHost} "$@"
  elif [[ "${opRunAcctName}_" = "root_" ]] ; then
    opDoComplain sudo -u ${doUser} ssh ${doHost} "$@"
  else
    EH_problem "${opRunAcctName} cant doAs ${doUser}"
  fi
}

function opDoRetAsAt {
  EH_assert [[ $# -gt 2 ]]
  typeset doUser=$1
  shift
  typeset doHost=$2
  shift
  if [[ "${opRunAcctName}_" = "${doUser}_" ]] ; then
    opDoRet ssh ${doHost} "$@"
  elif [[ "${opRunAcctName}_" = "root_" ]] ; then
    opDoRet sudo -u ${doUser} ssh ${doHost} "$@"
  else
    EH_problem "${opRunAcctName} cant doAs ${doUser}"
  fi
}

function opDoExitAsAt {
  EH_assert [[ $# -gt 2 ]]
  typeset doUser=$1
  shift
  typeset doHost=$2
  shift
  if [[ "${opRunAcctName}_" = "${doUser}_" ]] ; then
    opDoExit ssh ${doHost} "$@"
  elif [[ "${opRunAcctName}_" = "root_" ]] ; then
    opDoExit sudo -u ${doUser} ssh ${doHost} "$@"
  else
    EH_problem "${opRunAcctName} cant doAs ${doUser}"
  fi
}

fi