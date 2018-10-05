#!/bin/ksh

#
# RCS Revision: $Id: ocp-sup.sh,v 1.1 2003/09/24 00:45:02 mohsen Exp $
#

#
# Description:
#


function hookRun {
  typeset hookFuncName=""

  EH_assert [[ $# -eq 1 ]]

  hookFuncName=`ListFuncs | egrep "^$1$"`

  if [ "${hookFuncName}X" != "X" ] ;   then
    ${hookFuncName}
  fi
}

