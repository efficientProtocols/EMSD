#!/bin/osmtKsh
#!/bin/osmtKsh 

typeset RcsId="$Id: oceKitPrepTar.sh,v 1.1 2003/03/27 21:05:14 mohsen Exp $"

if [ "${loadFiles}X" == "X" ] ; then
    seedActions.sh -l $0 "$@"
    exit $?
fi

. oceKitLibs.sh

function vis_examples {
  typeset visLibExamples=`visLibExamplesOutput ${G_myName}`
  typeset extraInfo="-n showRun"
 cat  << _EOF_
EXAMPLES:
${visLibExamples}
--- GENERATE BINARY KIT ---
${G_myName} ${extraInfo} -i kitPrep
${G_myName} ${extraInfo} -i binKitMake
--- PUBLISHING BINARY KIT ---
${G_myName} ${extraInfo} -i updateContentDir
${G_myName} ${extraInfo} -i webPublished
--- FULL ACTION ---
${G_myName} ${extraInfo} -i fullUpdate
_EOF_
}

function vis_help {
  cat  << _EOF_
The complete process of making binary kit: 
   - src build --> results in results/targetOS/targetArch/bin
   - run this makeKitTar to produce .tar.gz
   - publish it to the website through mmaCntnt
For testing purposes:
   - download the .tar.gz from the web
   - unzip and untar it
   - run mmaLeapBinsPrep.sh
   - run mmaLeapAdmin.sh
_EOF_
}


function noArgsHook {
    vis_examples
}

function vis_kitPrep {

  ANV_raw "Prepare for making binary tar file."
  FN_dirCreatePathIfNotThere ${kitPrepBase}
  typeset leapDirLists="lib log bin config spool doc man osmtBin osmtLib" oneDir
  for oneDir in ${leapDirLists} ; do
    FN_dirCreateIfNotThere ${kitPrepBase}/${oneDir}
  done

  if [[ -d ${BIN_PATH} ]] ; then
    opDoComplain cp -p ${BIN_PATH}/* ${kitPrepBase}/bin
  fi

  if [[ -d ${LIBS_PATH} ]] ; then
    opDoComplain cp -p ${LIBS_PATH}/* ${kitPrepBase}/lib
  fi

  if [[ -d ${MAN_PATH} ]] ; then
    opDoComplain cp -p ${MAN_PATH}/* ${kitPrepBase}/man
  fi

  opDoComplain cp -p ${CURENVBASE}/COPYING ${kitPrepBase}
  opDoComplain cp -p ${CURENVBASE}/COPYING.LIB ${kitPrepBase}

  opDoComplain cp -p ${CURENVBASE}/oceModule/mmaDev/base/leapBinsPrep.sh ${kitPrepBase}/osmtBin


  ################### NOTYET ##############
  # This leap tools should come from      #
  # ${CURENVMASE}/install -- LATER        #
  # For now just skip this part           #
  # until the install dir is created      #
  #########################################

#   typeset leapToolLists="leapCommon.libSh leapEmsdMtaAdmin.sh leapEmsdUaAdmin.sh leapEsroAdmin.sh leapEsroBinsPrep.sh leapEsroOneAdmin.sh leapEsroTest.sh leapRoadmap.sh" #`ls ${opBinBase}/leap*` 
#   typeset oneLeapTool
#   for oneLeapTool in ${leapToolLists} ; do
#     opDoComplain cp -p ${INSTALL_DIR}/${oneLeapTool} ${kitPrepBase}/osmtBin
#   done

}

function vis_binKitMake {

  if [[ ! -d ${kitPrepBase} ]] ; then
    opDoComplain vis_kitPrep
  fi

  typeset kitPrepBase_dirsPart=`FN_dirsPart ${kitPrepBase}`
  typeset kitPrepBase_nonDirsPart=`FN_nonDirsPart ${kitPrepBase}`
  opDoRet cd ${kitPrepBase_dirsPart}
  opDoRet tar -cvf - ${kitPrepBase_nonDirsPart} | gzip > ${kitPrepBase_dirsPart}/${kitPrepBase_nonDirsPart}.tar.gz
}

function vis_updateContentDir {
  opDoComplain env kitPrepBase=${kitPrepBase} mmaCntntPkgs.sh -n showRun -p cntntItemsFile=mmaCntntPkgItems.mmaDev -s cntntPkg_mmaDevMod -a fullUpdate

  opDoComplain mmaCntntPkgCombs.sh -n showRun -p pkgCombContainer=mmaCntntPkgCombItems.proj -s cntntPkgComb_nedaOpenSrc -s cntntPkgComb_byTitle -a fullUpdate
}

function vis_webPublished {
  ANV_raw "Publish it to MailMeAnywhere.org"
  opDoComplain mmaCntntPubServers.sh -n showRun -p asUser=osmt -s cntntPubSrvr_arash -a fullUpdate
}

function vis_fullUpdate {
  opDoComplain vis_binKitMake
  opDoComplain vis_updateContentDir
  opDoComplain vis_webPublished
}

