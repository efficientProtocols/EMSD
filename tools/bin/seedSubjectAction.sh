#!/bin/osmtKsh 
#!/bin/osmtKsh

#
# RCS Revision: $Id: seedSubjectAction.sh,v 1.25 2004/01/13 20:36:30 mohsen Exp $
#
#set -x
if [ "${OSMTROOT}X" != "X" ] ; then
    if [ -f ${OSMTROOT}/bin/opConfig.sh ] ; then
      .  ${OSMTROOT}/bin/opConfig.sh 
    else
      echo "${OSMTROOT}/bin/opConfig.sh not found.\n"
      exit 1
    fi
else
    if [ -f `dirname $0`/opConfig.sh ] ; then
	.  `dirname $0`/opConfig.sh
    else
      echo "OSMTROOT not set and `dirname $0`/opConfig.sh not found.\n"
      exit 1
    fi
fi

. ${opBase}/lib/ocp-lib.sh
. ${opBase}/lib/doLib.sh

opBasePathSet

subject=""
actionDefault="itemActions"

#TagVar ItemsFile=MANDATORY
TagVar ItemsFile=""


#systemName=`uname -n`
#devlOsType=`uname -s`


###

#PATH=/usr/sbin:/usr/bin:/sbin

# Get options (defaults immediately below):

badUsage=
traceLevel=

Array paramInfo
#typeset -A  paramInfo   NOTYET, -A causes problems in ksh88 , works with ksh93

dotMeParamTmpFile=/tmp/${G_progName}.dotMe.$$

loadFiles=""


#G_checkMode=""  Defaults can be inherited
#G_forceMode=""

G_argsOrig="$@"
G_progName=`FN_nonDirsPart ${0}`

argsSubjectsList=("")

TM_trace 9 ${G_argsOrig}

dotMeParamTmpFile=/tmp/${G_progName}.dotMe.$$

while getopts a:c:T:s:i:p:l:u?n:vf c
do
    case $c in
    T)
	case "$OPTARG" in
	*[!0-9]* )
	    echo "$0: -T requires a numeric option"
 	    badUsage=TRUE
	    ;;
        [0-9]* )
	    traceLevel="$OPTARG"
	    ;;
	esac
	;;
    a)
        action="${OPTARG}"
	;;
    s)
        typeset t1=${argsSubjectsList[@]}
        argsSubjectsList=( $t1 "${OPTARG}" )
	;;
    i)
        visibleFunction="${OPTARG}"   # rest of argv is passed to visibleFunction
	;;
    p)
       leftSide=`ATTR_leftSide "${OPTARG}"`
       rightSide=`ATTR_rightSide "${OPTARG}"`
  
       echo "${leftSide}"="${rightSide}" >> ${dotMeParamTmpFile}
	;;
    c)
	G_checkMode="${OPTARG}"
	;;

    n)
	case "${OPTARG}" in 
	    "runOnly")
		G_runMode="runOnly"
		;;
	    "showOnly")
		G_runMode="showOnly"
		;;
	    "showRun")
		G_runMode="showRun"
		;;
	    "runSafe")
		G_runMode="runSafe"
		;;
	    "showProtected")
		G_runMode="showProtected"
		;;
	    "showRunProtected")
		G_runMode="showRunProtected"
		;;
	    *)
		badUsage=TRUE
		;;
	esac
	;;

    v)
	G_verbose="verbose"
	;;
    f)
	G_forceMode="force"
	;;
    l)
	loadFiles="${loadFiles} ${OPTARG}"
	G_myName=`FN_nonDirsPart ${OPTARG}`
	G_myInvokedName=${OPTARG}
	;;

    u | \?)
	badUsage=TRUE
	;;
    esac
done

typeset myOptind=$OPTIND

function opMyFullNameGet {
  typeset firstChar=${G_myInvokedName:0:1}

  if [[ "${firstChar}X" == "/X" ]] ; then 
    G_myFullName=${G_myInvokedName}
    return 
  fi
  
  if [[ -f ${G_myInvokedName} ]] ; then
    G_myFullName=${PWD}/${G_myInvokedName}
  else
    G_myFullName=`which ${G_myName}`
  fi
  
  if [[ ! -f ${G_myFullName} ]] ; then
     print -u2  ${G_myFullName} not found
     return 1
  fi
}

opMyFullNameGet

if [[ "${G_forceMode}_" != "_" ]] ; then
    export G_forceMode="${G_forceMode}"
fi
if [[ "${G_checkMode}_" != "_" ]] ; then
    export G_checkMode="${G_checkMode}"
    #print -u2 G_checkMode="${G_checkMode}"
fi


#defaultLoadFile=${mmaBinDistBase}/bin/`FN_prefix $0`.load
#    TM_trace 7 "Loading ${defaultLoadFile}"

#if test -f ${defaultLoadFile} ; then
#    TM_trace 7 "Loading ${defaultLoadFile}"
#    . ${defaultLoadFile}
#fi

#
# Load In Parameter Specificication Code 
# and possibly item_ code ...
# 
#

for i in ${loadFiles} ; do
    if test -f ${i} ; then
	TM_trace 7 "Pre Loading: $i"
	loadSegment="PRE"
	. ${i}
    else
	EH_problem "${i} not found: skiping it."
	usage=TRUE
	#exit
    fi
done

#opParamMandatoryVerify

#
# Now that we have all the parameters, get the job done.
#

#
# Bring in post load code
# 

#TM_trace 7  "loadSegment=${loadSegment}"

if [ "${loadSegment}_" == "POST_" ] ; then
for i in ${loadFiles} ; do
    if test -f ${i} ; then
	TM_trace 7 "Post Loading: $i"
	. ${i}
    else
	EH_problem "${i} not found: skiping it."
	usage=TRUE
	#exit
    fi
done
fi

G_preParamHookVal=`ListFuncs | egrep '^G_preParamHook$' || true`
if [ "${G_preParamHookVal}X" != "X" ] ; then
    G_preParamHook
fi


#
# EXECUTE PARAMETER ASSIGNMENTS
#
if [[ -f ${dotMeParamTmpFile} ]] ; then
  . ${dotMeParamTmpFile}
  rm ${dotMeParamTmpFile}
fi

G_postParamHookVal=`ListFuncs | egrep '^G_postParamHook$' || true`
if [ "${G_postParamHookVal}X" != "X" ] ;   then
    G_postParamHook
fi

#
# VERIFY COMPLETENESS OF REQUIRED PARAMETERS
#

function opParamMandatoryVerify {

  if [[ "${BASH_VERSION}_" != "_" ]] ; then
    # Because bash does not implement 
    # typeset -t, this feature is not
    # functional in bash
    return 0
  fi

  typeset retVal=0
  typeset i=""
    
 paramsList=`TagVarList`

  for i in ${paramsList} ; do
    if  [ "${i}X" == "X" ] ; then continue; fi;
      print "if [ \"\${$i}_\" == \"MANDATORY_\" ]  || [ \"\${$i}_\" == \"mandatory_\" ] \n then\n echo \"$i must be specified\"\n badUsage=TRUE\n retVal=1\n fi\n" > ${dotMeParamTmpFile}

      #cat ${dotMeParamTmpFile}
      . ${dotMeParamTmpFile}
      
  done

  FN_fileRmIfThere ${dotMeParamTmpFile}

  return ${retVal}
}


# BEGIN item_ FUNCTIONS

#
if [ "${ItemsFiles}X" == "X" -o "${ItemsFiles}_" == "MANDATORY_" ] ; then 
  ItemsFiles=./itemsFile.sh
else
  for i in ${ItemsFiles} ; do
    if test -f ${i} ; then
      TM_trace 7 "Loading ItemsFile=${i}"
      . ${i}
    else
      if [  "${i}_" != "/dev/null_" ] ; then 
	echo "${i} not found: skiping it."
	usage=TRUE
      fi
    fi
  done
fi


    
# END item_ FUNCTIONS


subjectIsValid=TRUE


function subjectIsValid {
    #set -x
    itemsList=`ListFuncs | egrep '^item_'`
    #echo "${itemsList}"
    
    typeset targetSubject=item_${subject}

    # item_all is a special case
    if IS_inList ${targetSubject} "${itemsList}" ;  then
	return 0
    else
      itemsList=`ListFuncs | egrep '^itemSpecial_'`
      #echo "${itemsList}"
    
      targetSubject=itemSpecial_${subject}
      if IS_inList ${targetSubject} "${itemsList}" ;  then
	return 0
      else
	return 1
      fi
    fi
}

function subjectValidVerify {
    typeset gotVal=0
    subjectIsValid || gotVal=$?

    if [[ ${gotVal} != 0 ]] ; then
	EH_problem "Subject: ${subject} is not valid!"
	subjectIsValid=FALSE
	usage
	exit 1
    fi
    return ${gotVal}
}

function subjectValidatePrepare {

  itemsList=`ListFuncs | egrep '^item_'`
  #echo "${itemsList}"
  
  targetSubject=item_${subject}

  # item_all is a special case
  if IS_inList ${targetSubject} "${itemsList}" ;  then
    if [[ "${G_verbose}_" == "verbose_" ]] ; then
      print -- "Subject in Use: ${targetSubject}"
    fi
  else
    itemsList=`ListFuncs | egrep '^itemSpecial_'`
    #echo "${itemsList}"
    
    targetSubject=itemSpecial_${subject}
    if IS_inList ${targetSubject} "${itemsList}" ;  then
      if [[ "${G_verbose}_" == "verbose_" ]] ; then
	print -- "Subject in Use: ${targetSubject}"
      fi
    else
      EH_problem "Subject: ${subject} is not valid!"
      subjectIsValid=FALSE
      usage
      exit 1
    fi
  fi

  ${targetSubject}

  return 0
}


function usage {

  # to get subject specific itemCmd_ lines

  if [ "${subjectIsValid}_" == "TRUE_" ] ; then    

    if [ "${subject}X" != "X" -a "${subject}" != "all"  ] ; then
      targetSubject=item_${subject}

      itemsList=`ListFuncs | egrep '^item_'`
      #echo "${itemsList}"
      
      if IS_inList ${targetSubject} "${itemsList}"
      then
	${targetSubject}
      else
	EH_problem "Subject: ${subject} is not valid!"
      fi
    fi
  fi


  printf >&2 "Usage: ${G_progName} [ -T traceLevel ] [ -u ] [ -p parameter=value ] [-s subject] [-a action] [-i visibleFunction] \n"
    
  printf >&2 "Usage: subject is one of: "


  hookFuncVal=`ListFuncs | egrep '^itemIsSubjectHook$'`

  itemsList=`ListFuncs | egrep '^item_'`
  for i in ${itemsList} ;   do
    if [ "${hookFuncVal}X" != "X" ] ;   then
      ${hookFuncVal} ${i##item_}
      if [ $? != 0 ] ; then
	# substring to get item_ out
	printf >&2 "${i##item_} "
      fi
    else
      printf >&2 "${i##item_} "
    fi
  done
  printf >&2 "\n"

    printf >&2 "Usage: action is one of: "
    dosList=`ListFuncs | egrep '^do_'`
    for i in ${dosList}
    do
      # substring to get dos_ out
      printf >&2 "${i##do_} "
    done
    printf >&2 "\n"

    printf >&2 "Usage: itemCmd_ is one of: "
    itemCmdsList=`TagVarList | egrep '^itemCmd_'`
    for i in ${itemCmdsList}
    do
      printf >&2 "${i} "
    done
    printf >&2 "\n"

    visFuncsList=`ListFuncs | egrep '^vis_'`
    printf >&2 "Usage: visibleFunction is one of: "
    for i in ${visFuncsList}
    do
      printf >&2 "${i##vis_} "
    done
    printf >&2 "\n"

    paramsList=`TagVarList | grep -v itemCmd_`
    printf >&2 "Usage: parameter is one of: "
    for i in ${paramsList}
    do
      printf >&2 "$i "
    done
    printf >&2 "\n"

	if [ "${ItemsFiles}X" != "X" -a "${ItemsFiles}_" != "MANDATORY_" ] ; then 
	        printf >&2 "Usage: ItemsFiles = ${ItemsFiles}\n"
	fi


    
    exit 1
}


itemCmdProcessor () {
    targetSubject=item_${subject}

    subjectValidVerify
    
    ${targetSubject}
    #prepVars

    applyCmd="${action}"

    #typeset

    # 
    itemCmdsList=`TagVarList  | egrep '^itemCmd_'`
    #echo ${itemCmdsList}

    IS_inList ${applyCmd} "${itemCmdsList}"

    if [ $? != 0 ] ; then 
      EH_problem "itemCmd_: ${action} is not valid"
      usage
      exit 1
    fi

    dotMeParamTmpFile=/tmp/${G_progName}.dotMe.$$

    print "echo About to eval: \${${applyCmd}} ${G_argv}" > ${dotMeParamTmpFile}
    print "eval \${${applyCmd}} ${G_argv}" >> ${dotMeParamTmpFile}
  
	#TM_trace 5 "About to eval: ${applyCmd} ${G_argv}"
    #cat ${dotMeParamTmpFile}
    . ${dotMeParamTmpFile}

    FN_fileRmIfThere ${dotMeParamTmpFile}
}


actionProcessor () {

  typeset tmp=${action##itemCmd_}

  TM_trace 7 ${action}

  if [ "${tmp}X" != "${action}X" ] ; then
    # So this is an itemCmd_someThing
    itemCmdProcessor
    TM_trace 7 ${action}
    return
  fi

  funcName="do_${action}"
  
  dosList=`ListFuncs | egrep '^do_'`

  if IS_inList ${funcName} "${dosList}"
  then
    ${funcName} "$@"
  else
    echo "action: ${action} is not valid"
    usage
    exit 1
  fi
}





#########  EXECUTION STARTS HERE ############

#G_validateOption ${action}  "${actionValidList}"

#G_validateRunOS "${runOSValidList}"


#
# REPORT USAGE PROBLEMS
#

if [ ${badUsage} ] ; then
    usage
    exit 1
fi

#shift `expr $OPTIND - 1`
shift `expr $myOptind - 1`

G_argv="$@"

#TM_trace 9 Args="$@"


#
# PERFORM REQUESTED TASKS
#

if [ "${visibleFunction}X" != "X" ]
then
    #echo "running ${visibleFunction}"
    # NOTYET: check exit codes
    vis_${visibleFunction} "$@"
    exit
fi

runNoArgsHook() {
  noArgsHookVal=`ListFuncs | egrep '^noArgsHook$'`
  if [ "${noArgsHookVal}X" != "X" ] ;   then
    noArgsHook
  else
    echo "No action taken. Specify options."
    usage
  fi
  exit
}

runNoSubjectHook() {
  noSubjectHookVal=`ListFuncs | egrep '^noSubjectHook$'`
  if [ "${noSubjectHookVal}X" != "X" ] ;   then
    noSubjectHook "$@"
  else
    echo "No action taken. Specify Subject."
    usage
  fi
}

runLastSubjectHook() {
  lastSubjectHookVal=`ListFuncs | egrep '^lastSubjectHook$'`
  if [ "${lastSubjectHookVal}X" != "X" ] ;   then
    lastSubjectHook "$@"
  fi
}

runFirstSubjectHook() {
  firstSubjectHookVal=`ListFuncs | egrep '^firstSubjectHook$'`
  if [ "${firstSubjectHookVal}X" != "X" ] ;   then
    firstSubjectHook "$@"
  fi
}

subject="${argsSubjectsList[0]}"

if [ "${action}X" == "X" -a "${subject}X" == "X" ] ; then
  runNoArgsHook
  exit
fi

if [ "${subject}X" == "X" ] ; then
  runNoSubjectHook "$@"
fi


if [ "${action}X" == "X" ]
then
  # NOTYET, itemAction was made obsolete by itemCmd_
  # Perhaps all of itemAction should be taken out.
  #action=${actionDefault}
  action=list
fi

if [ "${subject}" == "all" ]
then
  itemsList=`ListFuncs | egrep '^item_'`
  subjectList=""
  for i in ${itemsList} ; do
    subjectList="${subjectList} ${i##item_} "
  done

  hookFuncVal=`ListFuncs | egrep '^itemIsSubjectHook$'`

  runFirstSubjectHook "$@"

  TM_trace 7 ${subjectList}
  for i in ${subjectList} ; do
    subject=$i
    TM_trace 7 ${subject}
    if [ "${hookFuncVal}X" != "X" ] ;   then
      ${hookFuncVal} ${subject}
      if [ $? == 0 ] ; then   # itemIsSubjectHook returned FALSE
	continue
      fi
    fi
    actionProcessor "$@"
  done
  runLastSubjectHook "$@"
else
  if [[ ${#argsSubjectsList[@]} -gt 1 ]] ; then
    runFirstSubjectHook "$@"
  fi
  for i in "${argsSubjectsList[@]}" ; do
    subject=$i
    actionProcessor "$@"
  done
  if [[ ${#argsSubjectsList[@]} -gt 1 ]] ; then
    runLastSubjectHook "$@"
  fi
fi

exit


# Done List (2002 Feb 19)
# Add a global -v which sets G_verbose
# export G_runMode mapIt to G_runMode
# Check For existing G_runMode
# When opDo invokes an external op* or mma* script,
# we can be exporting G_runMode so that it gets inherited.
# EH_assert{,exit,ret} as special form of opDo with eval in for [[ use.

# NOTYET, Usage needs -n documented in both subjectAction and action.
#
#
#
# Example cleanup of opDo usage in opLibUse.sh
#