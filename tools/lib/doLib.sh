
function doLibExamplesOutput {
    # $1=callingsName
 cat  << _EOF_ 
COMMON EXAMPLES ---
${1} -i showMe
${1} -i seedSubjectHelp
${1} -i runFunc seedSubjectInternalsHelp
${1} -i ls
${1} -s all -a list
${1} -s all -a describe
${1} -s all -a schemaVerify
${1} -s ${opRunHostName} -a itemActions
COMMON DEBUGGING ---
${1} -v -n showSafe -T 9 -s all -a describe
_EOF_
}


function vis_seedSubjectHelp {
 cat  << _EOF_

In addition to all features availble to scripts based on
seedActions.sh, the following features are available to all scripts
based on seedSubjectActions.sh

Subject:  -s <subjectName>          -- Ex: ${G_myName}  -s someSubj ...
Subjects: -s <sub1> ... -s <subn>   -- Ex: ${G_myName}  -s sub1 ... -s subn
Subjects: -s all                    -- Ex: ${G_myName}  -s all
action:   -a <actionName>           -- Ex: ${G_myName} -a actionName

  - If more than one subject is specified,
    the firstSubjectHook is invoked.

  - The "-a" action is invoked repeatedly 
    for each of the subjects in the order
    that the subjects were specified on the 
    command line.

  - If more than one subject is specified,
    the lastSubjectHook is invoked.

Unlike, special subject "all", itemIsSubjectHook
is not invoked.

_EOF_

vis_seedHelp
}

vis_ls () {
  typeset itemsList=""
  itemsList=`typeset +f | egrep '^item_'`
  typeset thisItem=""
  for thisItem in  ${itemsList} ; do
    print -- ${thisItem##item_} 
  done
}

do_list () {
    targetSubject=item_${subject}
    subjectValidVerify
    typeset -ft ${targetSubject}
    ${targetSubject}
}


function do_describe {
  targetSubject=item_${subject}
  subjectValidVerify
  ${targetSubject}
  opItem_description
}


function subjectOrDefaultPrepare {
  #set -x
  EH_assert [[ $# -eq 1 ]]
  typeset moduleTag="$1"
  subject="${moduleTag}_${subject}"
  typeset targetSubject=item_${subject}

  if subjectIsValid ; then
    ${targetSubject}
    ANV_raw "Using ${targetSubject}"
    return 0
  else
    typeset defaultFunc=`typeset +f | egrep "itemDefault_${moduleTag}"`

    if [[ "${defaultFunc}_" != "_" ]] ; then
      ${defaultFunc}
      ANV_raw "Using ${defaultFunc}"
      return 0
    else
      EH_problem "${subject} not valid and No Default Function"
      return 1
    fi
  fi
}

function subjectIsRunHostOnly {
  if [[ "${subject}_" != "${opRunHostName}_" ]] ; then
    ANT_raw "Wrong Machine -- Re-run this script on ${subject}"
    return 1
  fi
  return 0
}


function do_itemActions  {
    targetSubject=item_${subject}
    subjectValidVerify
    ${targetSubject}
    TM_trace 7 "${itemActions}"
    
    typeset thisAction=""
    # if the item has a list of itemActions, then perform them all
    for thisAction in "${iv_itemActions[@]}" ;  do
      opDoComplain ${thisAction}
    done

    if [[ "${thisAction}_" == "_" ]] ; then
      ANT_raw "No ItemAction Specified -- $0: skipped"
    fi
}


function seedSubjectInternalsHelp {
 cat  << _EOF_

HOOKS
=====

The following Hooks are called in the order listed below:

firstSubjectHook:     When more than one subject, before
                      processing of the first subject.

itemIsSubjectHook:    When subject is \"all\", if 
                      Hook return value is 0, item is 
                      considered.

lastSubjectHook:      When more than one subject, after
                      processing of the last subject.


Seed Co-Routines
================

Execution Order should be re-produced here.

_EOF_
}