
function visLibExamplesOutput {
    # $1=callingsName
 cat  << _EOF_
COMMON EXAMPLES ---
${1} -i showMe
${1} -i seedHelp
${1} -n showOnly -T 9 -i help
${1} -i runFunc func args
_EOF_
}

function vis_runFunc {
  #EH_assert [[ $# -gt 0 ]]
  typeset func=$1
  shift
  ${func} "$@"
}

function vis_showMe {
 cat  << _EOF_
--- Program Parameters
RcsId=${RcsId}
G_argsOrig=${G_argsOrig}
G_progName=${G_progName}
G_myName=${G_myName}
G_myInvokedName=${G_myInvokedName}
G_myFullName=${G_myFullName}
G_runMode=${G_runMode}
G_verbose=${G_verbose}
G_checkMode=${G_checkMode}
G_forceMode=${G_forceMode}
--- OSMT Configurable Parameters
opShellVersion=${.sh.version}
opBase=${opBase}
--- User Parameters
opRunAcctName=${opRunAcctName}
_EOF_
# Also Trace Level
# NOTYET, Also show other relevant opBase and config parameters

  print -- "--- OSMT INFO ---"
  # NOTYET, test that it exists, use the right variable
  cat /etc/osmtInfo
}

function vis_seedHelp {
 cat  << _EOF_

The following features are available to all scripts based on
seedActions.sh and seedSubjectActions.sh

Tracing:     -T <runLevelNumber>  -- Ex: ${G_myName} -T 9 ...
Run Mode:    -n <runMode>         -- Ex: ${G_myName} -n runSafe ...
Verbose:     -v                   -- Ex: ${G_myName} -v
Force Mode:  -f                   -- Ex: ${G_myName} -f
Check Mode:  -c                   -- Ex: ${G_myName} -c fast

Tracing
=======
            DEFAULT: -T 0

            Trace Number Conventions:

	    0: No Tracing
	    1: Application Basic Info
	    2: Application General Info
	    3: Application Function Entry and Exit
    	    4: Application Debugging
	    5: Wrappers Library
	    6: Seed Script
	    7: Seed Supporting Libraries (eg, doLib.sh)
	    8: ocp_library
	    9: Quick Debug, usually temporary

Run Mode:
=========
           DEFAULT: runOnly

G_runMode=
   showOnly:           at opDo* just show the args always return 0
   runOnly:            at opDo* just execute
   showRun:            at opDo both runOnly and showOnly
   runSafe:            at opDo both show and run, but if protected
                       then just show
   showProtected:      Run everything and don't show except for
                       show only protected
   showRunProtected:   Run everything and don't show except for
                       run and show rotected

runSafe = unprotected: showRun, protected: show
showProtected = unprotected: run, protected: show
showRunProtected = unprotected: run, protected: showRun 

Verbose Mode:
=============

G_verbose=
   verbose        When Set, verbose format (eg, line nu, time tag, ...)
                  of Tracing and RunMode are selected.

Force Mode:
=============

G_forceMode=
   force          When Set, force/overwrite mode of operation
                  is selected.

Check Mode:
===========

G_checkMode={fast,strict,full}
   fast:          1) Skip asserting and consistency checks.
                  2) Do less than default, invoker will 
                     compensate
   strict:        Do asserts and consistency checks.
   full:          1) Do more than default

_EOF_
}