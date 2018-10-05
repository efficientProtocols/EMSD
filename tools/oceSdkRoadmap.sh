#!/bin/osmtKsh
#!/bin/osmtKsh

typeset RcsId="$Id: oceSdkRoadmap.sh,v 1.10 2003/03/18 01:44:19 pinneke Exp $"

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
${G_myName} -i overview
${G_myName} -i modelAndTerminology
${G_myName} -i todos
${G_myName} -i help
${G_myName} -i howTos
${G_myName} -i pointersAndReferences
${G_myName} -i printPackage
--- See Also
./win32SdkRoadmap.sh 
_EOF_
}


function noArgsHook {
  vis_examples
}

function vis_todos {
 cat  << _EOF_


Short Term:
----------

x- targetProcSpec.libSh to include msc and gcc for cygwin
- test in all environments

- Path to compiler should come from the user path.
  rest of the path is set in the scripts.
  Mohsen to take care of this.

- Enhance recBuild.sh, seedActions model
x- Add COPYING file
- Fix mkp.sh for clean to remove only if obj is there
- Fix mkp.sh for clean with msc to remove response files
- swProc.sh cleanAll lots of glitches.
- swProc.sh module preparation for review with mohsen
- setenv.sh set osmtroot to tools if not set.

- Build swProcs.sh for all of mts directory only test for Linux

- Add itemAction test scripts for invoker/performer esros ...

- create a emsdMul.pub mapFile (no vorde, no kpatch, no cpp, ...)
  just esros amd for now (mts) later renamed to emsd.


Done
----

-

_EOF_
}


function vis_overview {
 cat  << _EOF_

###############
                                              +====================+
GLOBAL                                        |  target templates  |
CONFIGURATION                                 |  family & target   |
SELECTIONS                                    +====================+

                            +------------------+ 
                            | targetSelect.sh  | 
                            +------------------+ 

###############
                                                   +=================+
SELECTED        +=============+                    | targetFamily.sh |
GLOBAL          | BuildEnv.sh |                    | target.sh    |
CONFIGURATION   +=============+                    +=================+


###############

                   +------------------+         +---------------+
                   | targetProc.libSh |         | targetProc.sh |
                   +------------------+         +---------------+

###############
                         +===================================+
                         |              mkpDefault.parSh     |
                         +===================================+

                                +--------+
                                | mkp.sh |
                                +--------+

              +-------------------------------------------------+
Cononical     | win32cc.sh                                      |
Compilers     | win32lib.sh                 ...                 |
              | win32link.sh                ...                 |
              +-------------------------------------------------+

###############

              +-------------------------------------------------+
Make          | +----------+      +-------+    +------+         |
Layer         | | OpusMake | .... | gmake |    | make |         |
              | +----------+      +-------+    +------+         |
              +-------------------------------------------------+



              +-------------------------------------------------+
Compilers     |  MSC     GCC     CC    PURIFY   ...             |
              +-------------------------------------------------+


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

Configuration Parameters to be doted    =======
Executable Shell Scripts                -------   .sh
Library Shell Scripts to be doted       -------   .libSh

_EOF_
}


function vis_modelAndTerminology {
 cat  << _EOF_

   Terminology and Model:
   ======================

  oceSdkRoadmap.sh         -- This File. General Orientation and Information

  targetSelect.sh          -- The first step in running the build procedure is to
                              run this script.  This script has to be run for initial build and
                              everytime you need to change any variables regarding
                              target and build environment.  The following variables are 
                              available through targetSelect.sh:
                              - targetFamily, targetName, targetOS, targetArchType, targetProdStatus,
                                targetProdRevNu, targetDistMethod, targetDistUser.
                              targetSelect.sh generate the following file:
                              - targetFamily.sh
                              - target.sh
                              - buildEnv.sh
                              targetSelect.sh use templates that reflect your choice combination of
                              targetFamily,targetName, and targetOS to generate those 3 files.
                              All of these templates are stored in the targets directory
                              (i.e. <targetFamily>-<targetName>.targetFamily
                                    <targetFamily>-<targetName>.buildEnv
                                    <targetFamily>-<targetName>-<targetOS>.target)


  buildEnv.sh              -- This file setup the build environment variable.
                              - buildEnvOS
                              - buildEnvShell
                              - buildEnvCompiler
                              - buildEnvMake
                              - buildEnvUtils

  targetFamily.sh          -- Information about the base directory that supports a number of directories.
                              This file is also sourced by target.sh.
                                - targetFamilyName
                                - targetFamilyReleaseNotes

  target.sh             -- Setup the following variables:
                                - targetProdBaseName
                                - targetProdFlavor
                                - targetProdName
                                - targetReleaseNotesSpecificFile
                                - targetBuildEnvFile
                                - targetProcLibFile
                                - targetArchType
                                - targetOS
                                - targetProdStatus
                                - targetProdRevNu
                                - targetDistMethod
                                - targetDistFileBase
                                - targetDistUser

  targetProc.libSh          -- Library file for targetProc.sh

  targetProc.sh             -- Script to generate mkpDefault.parSh file.

  mkpDefault.parSh          -- Mandatory file to use mkp.sh
                               This file is generated by targetProc.sh.  
                               Everytime you run swProc.sh, it checked
                               for mkpDefault.parSh.  If it doesn't exist,
                               it will create it.  Alternatively, if you 
                               change any of the target.sh or 
                               targetFamily.sh variables, you have to 
                               re-generated this file by running 
                               targetProc.sh -i mkpParamsFilesReGen.

_EOF_
}


function vis_help {
 cat  << _EOF_

At A Glance
-----------

  mmaSdkRoadmap.sh         -- This File. General Orientation and Information

  xxxxxxxxxxxxxxxxx.sh     -- To be included in all mapVerModules.sh
                              General configuration parameters and
                              general useful functions go here
_EOF_
}

function vis_howTos {
 cat  << _EOF_

 - How do I create a new Module mapFile?


_EOF_
}


function vis_pointersAndReferences {
 cat  << _EOF_


_EOF_
}

function vis_printPackage {
  a2ps -s 2 oceSdkRoadmap.sh buildEnv.sh targetFamily.sh newTarget.sh  mkp.sh
targetProc.libSh targetProcSpec.libSh targetProc.sh targetSelect.sh
}
