

function opPathIsIn {
  EH_assert [[ $# -eq 2 ]]
  typeset pathVal
  eval pathVal='$'{$1}

  if [[ "${pathVal}_" == "_" ]] ; then
    return 1
  else
    if env | FN_grep $1 | FN_egrep "=$2\$" > /dev/null ; then
      ANV_raw "$0: $2 Already In $1 -- (=) skipped"
      return 0
    elif env | FN_grep $1 | FN_egrep "=$2:" > /dev/null  ; then
      ANV_raw "$0: $2 Already In $1 -- (=:) skipped"
      return 0
    elif env | FN_grep $1 | FN_egrep ":$2:" > /dev/null ; then
      ANV_raw "$0: $2 Already In $1 -- (::) skipped"
      return 0
    elif env | FN_grep $1 | FN_egrep ":$2\$" > /dev/null ; then
      ANV_raw "$0: $2 Already In $1 -- (:) skipped"
      return 0
    else
      return 1
    fi
  fi
  # NOT REACHED
}

function opPathSet {
  EH_assert [[ $# -eq 2 ]]
  typeset pathVal
  eval pathVal='$'{$1}

  if [[ "${pathVal}_" == "_" ]] ; then
    export $1=$2
  else 
    ANT_raw "$0: previously set $1=${pathVal} -- set to $2"
    export $1=$2
  fi
  #env | grep $1=
}

function opPathSetList {
    #set -x
  EH_assert [[ $# -eq 2 ]]

  typeset thisOne=""
  for thisOne in $2 ;  do
    opPathAppend $1 "${thisOne}"
  done
}


function opPathAppend {
  EH_assert [[ $# -eq 2 ]]
  typeset pathVal
  eval pathVal='$'{$1}

  if [[ "${pathVal}_" == "_" ]] ; then
    export $1=$2
  else 
    if opPathIsIn "$1" "$2"  ; then 
      doNothing
    else
      export $1="${pathVal}:$2"
    fi
  fi
  #env | grep $1=
}


function opPathPrepend {
  EH_assert [[ $# -eq 2 ]]
  typeset pathVal
  eval pathVal='$'{$1}

  if [[ "${pathVal}_" == "_" ]] ; then
    export $1=$2
  else 
    if opPathIsIn "$1" "$2"  ; then 
      doNothing
    else
      export $1="$2:${pathVal}"
    fi
  fi
  #env | grep $1=
}
