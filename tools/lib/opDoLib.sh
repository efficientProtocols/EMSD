typeset RcsId="$Id: opDoLib.sh,v 1.4 2002/07/07 05:28:16 mohsen Exp $"

#
# See seedHelp in visLib.sh for usage and high level
# description of opDo Run level modes and selections.
#

#
#  Report And Return After a given invokation
#  After an invokation, In case of a failure (||),
#  Report (RepV "string";)  and return (Ret)
#

# function vis_testMe {
#   print "start"
#   true ||  RepV "Some true";  Ret
#   print "did true"
#   false ||  RepV "Some Error"; Ret
#   print "did false"
# }

alias RepV='(retRepGotVal=$? &&  repAnv_ "${G_myName}" "$LINENO" "$0" '
alias RepA='(retRepGotVal=$? &&  repAnt_ "${G_myName}" "$LINENO" "$0" '
alias Ret=') ||  return ${retRepGotVal}'

function repAnv_ {
  typeset progName=$1
  typeset lineNu=$2
  typeset callingFunc=$3

  shift; shift; shift
  ANV_raw "${progName}:${callingFunc}:${lineNu}  returned: ${retRepGotVal} -- $@"
  return ${retRepGotVal}
}

function repAnt_ {
  typeset progName=$1
  typeset lineNu=$2
  typeset callingFunc=$3

  shift; shift; shift
  ANT_raw "${progName}:${callingFunc}:${lineNu}  returned: ${retRepGotVal} -- $@"
  return ${retRepGotVal}
}


#### public thingie

if [[ -z "${G_runMode}" ]] ; then
    export G_runMode="runOnly"
fi
if [[ -z "${G_verbose}" ]] ; then
    export G_verbose="notVerbose"
fi

# Die if fails. But be quiet otherwise
alias opDoAssert='_opDoAssert "${G_myName}" "$0" $LINENO'

# Complain and die if fails
alias opDoExit='_opDo "${G_myName}" "$0" $LINENO "EXIT"'

# Complain and return to calling function if fails
#alias opDoRet="trap 'echo HAI; return \$?' ERR ;_opDo '${G_myName}' '$0' $LINENO 'DONT_EXIT'"
#alias opDoRet='_opDo "${G_myName}" "$0" $LINENO "DONT_EXIT"'
alias opDoRet='opDoComplain '

# Complain if fails
#alias opDoComplain='trap "incomplain" ERR ; _opDo "${G_myName}" "$0" $LINENO "DONT_EXIT"'
alias opDoComplain='_opDo "${G_myName}" "$0" $LINENO "DONT_EXIT"'

# Just do it
#alias opDo='trap "" ERR ;  _opDo "${G_myName}" "$0" $LINENO "JUST_DO"'
alias opDo='_opDo "${G_myName}" "$0" $LINENO "JUST_DO"'

function opDoProtectedBegin {
    if [[ -z "${__withinCritical}" ]] ; then
      # NOTYET, When all modernized and consistent, export not needed
	export __opDo_withinCritical=1
    else
	let __opDo_withinCritical=__opDo_withinCritical+1
    fi
}
function opDoProtectedEnd {
    if [[ -z "${__opDo_withinCritical}" ]] ; then
	export __opDo_withinCritical=0
    else
	let __opDo_withinCritical=__opDo_withinCritical
    fi
}
function opDoProtected {
    typeset err=0
    opDoProtectedBegin
    "$@" || err=$?
    opDoProtectedEnd
    return $err
}

#### private thingie

function _opDoRunOnly {
  #
  # $1: mode
  # $2: msg
  # $3: failed msg
  # $4-: command

  if [[ "${1}X" == "JUST_DOX" ]] ; then
    shift; shift; shift;
    "$@" || return $?
  else
    typeset err=0
    typeset mode="${1}"
    typeset failedMsg="${3}"
    shift; shift; shift;
    "$@" || err=$?
    if [[ $err -ne 0 ]] ; then
      EH_problem "$failedMsg" "$err"
      if [[ "${mode}X" == "EXITX" ]] ; then
	exit $err
      fi
      return $err
    fi
  fi
}

function _opDoShowOnly {
  #
  # $1: mode
  # $2: msg
  # $3: failed msg
  # $4-: command
  print -u2 "${2}"
}

function _opDoShowRun {
  #
  # $1: mode
  # $2: msg
  # $3: failed msg
  # $4-: command
  _opDoShowOnly "$@"
  _opDoRunOnly "$@"
}

function _opDo {
    typeset scriptName="$1"; shift || exit 
    typeset functionName="$1"; shift || exit
    typeset lineNumber="$1"; shift || exit
    typeset mode="$1"; shift || exit
    if [[ -z "${__opDo_prevScriptName}" ]] ; then
	export __opDo_prevScriptName=""
    fi
    if [[ -z "${__opDo_prevFunctionName}" ]] ; then
	export __opDo_prevFunctionName=""
    fi
    if [[ "${__opDo_prevScriptName}X" == "${scriptName}X" ]] ; then
	scriptName=""
    else
	__opDo_prevScriptName="${scriptName}"
	scriptName="${scriptName}::"
    fi
    if [[ "${__opDo_prevFunctionName}X" == "${functionName}X" ]] ; then
	functionName=""
    else
	__opDo_prevFunctionName="${functionName}"
    fi
    lineNumber="[${lineNumber}]"
    typeset msg
    typeset failedMsg
    case "${G_verbose}" in
	"verbose")
	    msg="${scriptName}${functionName}${lineNumber}: $@"
	    failedMsg="FAILED: ${scriptName}${functionName}${lineNumber}: $@ [ErrCode]="
	    ;;
	*)
	    msg="$@"
	    failedMsg="FAILED: $@ [ErrCode]="
	    ;;
    esac
    case "${G_runMode}" in
	"runOnly")
	    _opDoRunOnly "$mode" "$msg" "$failedMsg" "$@" || return $?
	    ;;
	"showOnly")
	    _opDoShowOnly "$mode" "$msg" "$failedMsg" "$@" || return
	    ;;
	"showRun")
	    _opDoShowRun "$mode" "$msg" "$failedMsg" "$@" || return
	    ;;
	"runSafe")
	    #runSafe = unprotected: showRun, protected: show
	    if [[ ${__opDo_withinCritical} -eq 0 ]] ; then
		_opDoShowRun "$mode" "$msg" "$failedMsg" "$@" || return
	    else
		_opDoShowOnly "$mode" "$msg" "$failedMsg" "$@" || return
	    fi
	    ;;
	"showCritical"|"showProtected")
	    #showCritical    = unprotected: run,     protected: show
	    if [[ ${__opDo_withinCritical} -eq 0 ]] ; then
		_opDoRunOnly "$mode" "$msg" "$failedMsg" "$@" || return
	    else
		_opDoShowOnly "$mode" "$msg" "$failedMsg" "$@" || return
	    fi
	    ;;
	"showRunCritical"|"showRunProtected")
	    #showRunCritical = unprotected: run,     protected: showRun 
	    if [[ ${__opDo_withinCritical} -eq 0 ]] ; then
		_opDoRunOnly "$mode" "$msg" "$failedMsg" "$@" || return
	    else
		_opDoShowRun "$mode" "$msg" "$failedMsg" "$@" || return
	    fi
	    ;;
    esac
    return $err
}



function _opDoAssert {
    typeset err=0
    typeset scriptName="$1"; shift || exit 
    typeset functionName="$1"; shift || exit
    typeset lineNumber="$1"; shift || exit
    scriptName="${scriptName}::"
    lineNumber="[${lineNumber}]"
    typeset failedMsg="ASSERTION FAILED: ${scriptName}${functionName}${lineNumber}: $@ [ErrCode]="
    "$@" || err=$?
    if [[ $err -ne 0 ]] ; then
	EH_problem "$failedMsg" "$err"
	exit $err
    fi
}

#
# Os Dependent opDos
#

function opDoOsRun {
  typeset preName=""
  typeset postName=""
  
  if [[ $# = 1 ]] ; then
    postName=$1
    preName=""
  elif [[ $# = 2 ]] ; then
    postName=$1
    preName="$2"
  else
    EH_problem "Too many args"
    return 1
  fi

  if [ "${opRunOsType}_" == "SunOS_" ] ; then
    # NOTYET, verify that the function exists
    ${preName}${opRunOsType}${postName}
  elif [ "${opRunOsType}_" == "Linux_" ] ; then
    ${preName}${opRunOsType}${postName}
  else
    EH_problem "Unsupported OS: ${opRunOsType}"
  fi

  return 
}

# # Examples

# opDoOsRun "someFunc"

# function  Linux_someFunc {
#   print $0
#   return    
# }

# function  SunOs_someFunc {
#   print $0
#   return    
# }

