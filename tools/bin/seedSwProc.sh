#!/bin/osmtKsh

# TODO
#
# - tools and tools/bin need to explicitly be added to path based on curenvbase
# - summary and all other functions need to be functional and tested
# - All failures need to result in return
# - 
# - The procedures need to be tested on cygwin
# 
#set -x
typeset RcsId="$Id: seedSwProc.sh,v 1.12 2004/01/13 20:36:30 mohsen Exp $"


if [ "${loadFiles}X" == "X" ] ; then
     seedSubjectAction.sh -l $0 "$@"
     exit $?
fi

export PATH="${CURENVBASE}/tools:${CURENVBASE}/tools/bin:${PATH}"

. ${opBinBase}/mmaLib.sh

if [[ -f ${CURENVBASE}/tools/targetProc.libSh ]] ; then
    . ${CURENVBASE}/tools/targetProc.libSh
    #. ${CURENVBASE}/tools/targetSelect.sh

    targetParamsAnalyze
    buildEnvAnalyze
else
    EH_problem "${CURENVBASE}/tools/targetProc.libSh not found -- skipped"
    exit 1
fi


function vis_examples {
  typeset oneSubject="multiocp"
  typeset extraInfo="-v -n showRun"
  typeset doLibExamples=`doLibExamplesOutput ${G_myFullName}`
  typeset orderedListStr=`${G_myFullName} -i orderedList`
  cat  << _EOF_
EXAMPLES:
${doLibExamples}
--- INFORMATION ---
${G_myFullName} -s all -a summary
${G_myFullName} ${extraInfo} ${orderedListStr} -a describe
--- FULL BUILD ACTIONS ---
${G_myFullName} -s all -a fullBuild
${G_myFullName} ${orderedListStr} -a fullBuild
${G_myFullName} -i targetProcPrep
--- FULL CLEAN ACTIONS ---
${G_myFullName} -s all -a fullClean
${G_myFullName} ${orderedListStr} -a fullClean
${G_myFullName} -i fullClean
--- INFORMATION ---
${G_myFullName} -i orderedList
_EOF_
}

function vis_help {
  cat  << _EOF_
NOTYET
_EOF_
}

function noArgsHook {
  vis_examples
}


firstSubjectHook() {
  case ${action} in
    "summary")
	       typeset -L10 f1="Name" 
	       typeset -L20 f2="Directory"
	       typeset -L10 f3="CmdType"
	       typeset f4="TargetOS"
	       print "$f1$f2$f3$f4"
	       echo "----------------------------------------------------"
       ;;
    *)
       return
       ;;
  esac

}

lastSubjectHook() {
  case ${action} in
    "summary")
	       echo "----------------------------------------------------"
       ;;
    *)
       return
       ;;
  esac
}

function do_summary {
  subjectValidatePrepare
  typeset -L10  v1=${subject}
  typeset -L20 v2=${iv_swProc_dirName}
  typeset -L10 v3=${iv_swProc_cmdType}
  typeset v4=${iv_swProc_supportedTargetOs[@]}
  print "${v1}${v2}${v3}${v4}"
}

function vis_targetProcPrep {

#   if [ ! -f ${selectedTargetFamilyFile} ] ; then
#     ${CURENVBASE}/tools/targetSelect.sh -i defaultGen
#   fi

  if [ ! -f ${mkpDefaultFile} ] ; then
    ${CURENVBASE}/tools/targetProc.sh -i mkpParamsFilesGen
  else
    return 0
  fi
}

function do_fullBuild {

  if [[ "${CURENVBASE}_" == "_" ]] ; then
    EH_problem "CURENVBASE has not been set.  Please source the setenv.sh"
    exit 1
  fi

  opDoComplain vis_targetProcPrep

  subjectValidatePrepare
  typeset list=${itemOrderedList[@]}

  print "##### ${HERE}/${iv_swProc_dirName} #####"

  if IS_inList ${subject} "${list}" ; then
    if [[ -d ${HERE}/${iv_swProc_dirName} ]] ; then
      if isSupportedCompiler ; then
	if isSupportedTargetOs ; then
	  opDoComplain cd ${HERE}/${iv_swProc_dirName} || return
	  if [ "${iv_swProc_cmdType}_" == "buildall_" ] ; then
	    typeset orderedListStr=`./${G_myName} -i orderedList`
	    opDoComplain ./${G_myName} ${orderedListStr} -a fullBuild || return
	  elif [ "${iv_swProc_cmdType}_" == "mkp_" ] ; then
	    opDoComplain mkp.sh || return
	  fi
	else
	  print "Unsupported TargetOs in ${subject}.  Skipped."
	  return 1
	fi
      else
	print "Unsupported Compiler in ${subject}.  Skipped."
	return 1
      fi
    else
      print "Directory ${HERE}/${iv_swProc_dirName} does not exist.  Skipped."
      return 1
    fi
  fi
}

# function vis_fullClean {

#   print "Removing....."
#   find . -print | egrep '[a-z]\.o$|[0-9]\.o$|[a-z]\.pdb$|[0-9]\.pdb$'
#   find . -print | egrep '[a-z]\.o$|[0-9]\.o$|[a-z]\.pdb$|[0-9]\.pdb$' | xargs /bin/rm
# }

function do_fullClean {

  subjectValidatePrepare

  if [[ -d ${HERE}/${iv_swProc_dirName} ]] ; then
    typeset thisDir="${HERE}/${iv_swProc_dirName}"
    opDoComplain cd ${thisDir} || return
    print "##### ${HERE}/${iv_swProc_dirName} #####"
    cleanAllFiles ${thisDir}
    if [ "${iv_swProc_cmdType}_" == "buildall_" ] ; then
      if [[ -f ${thisDir}/${G_myName} ]] ; then
	opDoComplain ${HERE}/${iv_swProc_dirName}/${G_myName} -s all -a fullClean || return
      fi
    elif [ "${iv_swProc_cmdType}_" == "mkp_" ] ; then
      opDoComplain mkp.sh clean || return
    fi
  else
    print "Directory ${HERE}/${iv_swProc_dirName} does not exist.  Skipped."
    return 1
  fi

  if [[ -d ${HERE}/results ]] ; then
    # ../../results/ppc3/
    typeset toBecleanUpDirList="linux/x86/bin   \
                                linux/x86/libc  \ 
                                sol2/sun4m/bin  \
                                sol2/sun4m/libc \
                                sol2/sun4u/bin  \
                                sol2/sun4u/libc \
                                sol2/x86/bin    \
                                sol2/x86/libc   \
                                wce/mips/bin    \
                                wce/mips/libc   \
                                wce/sh3/bin     \
                                wce/sh3/libc    \
                                ppc3/mips/bin   \
                                ppc3/mips/libc  \
                                ppc3/sh3/bin    \
                                ppc3/sh3/libc   \
                                ppc3/x86/bin    \
                                ppc3/x86/libc   \
                                winnt/x86/bin   \
                                winnt/x86/libc"

    typeset thisDir
    for thisDir in ${toBecleanUpDirList} ; do
      opDoComplain cd ${HERE}/results/${thisDir} || return
      typeset existingFiles=`ls | grep -v CVS`
      if [[ "${existingFiles}_" != "_" ]] ; then
	print "Cleaning up: ${HERE}/results/${thisDir}"
	rm ${existingFiles}
      fi
    done
  fi
}

function cleanAllFiles {

  # $1 = the directory that need to be cleaned.

  typeset TEMP_FILE=/tmp/cleanAll.$$
  rm -f $TEMP_FILE

  echo "In ${1}, Creating list ..."
  cd ${1}

  find . -type f    \( -name '#*#'   \
    -o  -name '~*'     \
    -o  -name '*~'     \)    -print  > $TEMP_FILE

  echo "$TEMP_FILE" >> $TEMP_FILE

  TO_DELETE=`cat $TEMP_FILE | tr x x`
  echo "In ${1}, Removing ..."
  echo $TO_DELETE
  rm $TO_DELETE
}

function vis_orderedList {
  typeset thisList
  for thisList in ${itemOrderedList[@]} ; do
    print -n -- "-s ${thisList} "
  done
  print
}

function isSupportedCompiler {
  typeset supportedCompilerList=${iv_swProc_supportedCompilers[@]}
  if IS_inList "all" "${supportedCompilerList}" ; then
    return 0
  elif IS_inList ${buildEnvCompiler} "${supportedCompilerList}" ; then
    return 0
  else
    return 1
  fi
}

function isSupportedTargetOs {
  typeset supportedTargetOsList=${iv_swProc_supportedTargetOs[@]}
  if IS_inList "all" "${supportedTargetOsList}" ; then
    return 0
  elif IS_inList ${targetOS} "${supportedTargetOsList}" ; then
    return 0
  else
    return 1
  fi
}

function itemPre_swProc {
  iv_swProc_dirName=""
  iv_swProc_cmdType="buildall"
  iv_swProc_supportedCompilers=("all")
  iv_swProc_supportedTargetOs=("all")
}

function itemPost_swProc {
  iv_itemName=${subject}
}
