#!/bin/osmtKsh
#!/bin/osmtKsh

typeset RcsId="$Id: leapRoadmap.sh,v 1.1 2002/11/06 04:41:33 mohsen Exp $"

if [ "${loadFiles}X" == "X" ] ; then
    seedActions.sh -l $0 "$@"
    exit $?
fi


function vis_examples {
  typeset visLibExamples=`visLibExamplesOutput ${G_myName}`
 cat  << _EOF_
EXAMPLES:
${visLibExamples}
--- Information
${G_myName} -i modelAndTerminology
${G_myName} -i todos
${G_myName} -i help
${G_myName} -i howTos
${G_myName} -i pointersAndReferences
${G_myName} -i printPackage
${G_myName} -i printItems
_EOF_
}

function noArgsHook {
  vis_examples
}


function vis_todos {
 cat  << _EOF_
 
  TODOs
  ============

Short Term:
----------


Long Term:
---------

_EOF_
}


function vis_help {
  opRunEnvGet
 cat  << _EOF_


DESCRIPTION


Basic  Scripts
--------------

  leapRoadmap.sh         -- This File. General Orientation and Information

  leapCommon.libSh       -- This File. General Orientation and Information

  leapEsroBinsPrep.sh    -- Prepare binary files for djbFtp
                         -- for relevant pltforms and versions
                         -- Install leapEsro binaries on opRunHostName

  leapEsroHosts.sh       -- For subject host, configure qmail

  leapEsroAdmin.sh       -- Start, stop and addNewAccounts


_EOF_

  exit 1
}

function vis_howTos {
 cat  << _EOF_
    A-1) How Do ...?
       Follow the steps below.

_EOF_
}


function vis_modelAndTerminology {
  opRunEnvGet
 cat  << _EOF_
 
   Terminology and Model:


_EOF_
}


function vis_pointersAndReferences {
 cat  << _EOF_

NOTYET, Pinneke, anything worth mentioning here.
_EOF_
}

function vis_printPackage {
  opDoComplain a2ps -s 2 leapRoadmap.sh leapCommon.libSh leapEsroAdmin.sh leapEsroOneAdmin.sh leapEsroTest.sh leapEmsdUaAdmin.sh leapEmsdMtaAdmin.sh 
    # binsPrep and Hosts later
}

function vis_printItems {
  typeset filesToPrint=""
  typeset filesList="leapEsroHostItems.site"

  typeset thisOne=""
  for thisOne in ${filesList} ; do
    filesToPrint="${filesToPrint} ${opSiteControlBase}/${opRunSiteName}/${thisOne}"
  done

  opDoComplain a2ps -s 2 ${filesToPrint}
}


