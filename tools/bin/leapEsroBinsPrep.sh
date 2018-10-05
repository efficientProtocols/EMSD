#!/bin/osmtKsh
#!/bin/osmtKsh 

typeset RcsId="$Id: leapEsroBinsPrep.sh,v 1.1 2002/11/06 04:41:33 mohsen Exp $"

if [ "${loadFiles}X" == "X" ] ; then
    seedActions.sh -l $0 "$@"
    exit $?
fi


. ${opBinBase}/mmaLib.sh

#
# PKG Base Variables
# 

# /opt/public/mmaSrc/imapPlus/pine4.44-mma
mmaImapSrcBase=${mmaSrcBase}/imapPlus/pine4.44-mma

mmaImapBinBase=${mmaBinBase}
mmaImapManBase=${mmaManBase}


function vis_examples {
  typeset visLibExamples=`visLibExamplesOutput ${G_myName}`
 cat  << _EOF_
EXAMPLES:
${visLibExamples}
--- OBTAIN ---
${G_myName} -i obtain
--- UNPACK ---
${G_myName} -i unpack
--- GENERATE BINARIES ---
${G_myName} -i build
--- GENERATE PACKAGE ---
${G_myName} -i pkgMake
--- PAKAGE INSTALL/REMOVE  ---
${G_myName} -i pkgInstall
${G_myName} -i initInstall
${G_myName} -i pkgRemove
--- PAKAGE VERIFY ---
${G_myName} -i pkgVerify
--- CLEAN UP BINARIES ---
${G_myName} -i clean
--- FULL SERVICE ---
${G_myName} -i buildAndInstall
${G_myName} -i fullInstall
${G_myName} -i fullVerify
${G_myName} -i fullUpdate
--- PACKAGE INFORMATION ---
${G_myName} -i info
_EOF_
}


function vis_help {
  cat  << _EOF_

The package is built in:  ${mmaImapSrcBase}.

Sources were slightly modified for qmail
to specify INBOX not be /var/mail but instead
~/Main.mmaInbox.

NOTYET, do a context diff and keep that.

Binaries are installed in: ${mmaImapBinBase}.
ManPages are installed in: ${mmaImapManBase}.

BinsPrep for the following packages are supported:
  pine and imap ...

  - pine:        Architecture specific binary kits are
                 made.

  - imap:        Destined for multi-binary installation

_EOF_
}

noArgsHook() {
    vis_examples
}


function vis_info {
  vis_help
  vis_obtain
}


function vis_obtain {
  cat  << _EOF_
ftp://ftp.cac.washington.edu/imap/

http://www.imap.org/
_EOF_
}

function vis_unpack {
  EH_problem "NOTYET"
  return 0
}


# Prior to this 
# in pine4.33/imap directory
# make gso

function vis_build {

  # /opt/public/src/Sol-2/networking/mma/imapPlus/pine4.43-mma/mmaBuildIt.sh
  cd ${mmaImapSrcBase}
  
  ./build clean

  if [ "${opRunOsType}_" == "SunOS_" ] ; then
    #./build  so5
    ./build  NOSSL gs5
  elif [ "${opRunOsType}_" == "Linux_" ] ; then
    #./build  slx
    ./build  NOSSL slx
  else
    EH_problem "Unsupported OS: ${opRunOsType}"
  fi

  ls -l bin/*
}



function vis_pkgMake {
  print -- "No Package Made For Pine, Just build and install"
  return 0
}


function vis_pkgInstall {
  #
  # Install Binaries
  #

  print -- "Installing Bin Pages"

  opDoExit cp  ${mmaImapSrcBase}/bin/* ${mmaImapBinBase}

  #
  # Install Man Pages
  #
  typeset manPages=`ls ${mmaImapSrcBase}/doc/*.1`

  print -- "Installing Man Pages"

  typeset thisOne=""
  for thisOne in  ${manPages} ; do
    opDoExit cp ${thisOne} ${mmaImapManBase}/man1
  done
  # NOTYET, locate man pages for imapd and ipopd
}


function vis_initInstall {
  print -- "initInstall done in mmaImapServers.sh"
}

function vis_pkgRemove {
  EH_problem "NOTYET"
}

function vis_pkgVerify {
  #if [[ -x /opt/public/mma/
  EH_problem "NOTYET"
}

function vis_clean {
  cd ${mmaImapSrcBase}
  
  ./build clean
}

function vis_buildAndInstall {
  vis_build
  vis_pkgInstall
}

function vis_fullInstall {
  vis_pkgInstall
}


function vis_fullUpdate {
  if [[ -x /opt/public/mma/bin/imapd ]] ; then
    ANT_raw "${G_myName}: $0: imapd in place"
  else
    FN_multiArchSymLinks /opt/public/mma/bin /here/archLinks/opt/public/mma/bin /opt/public/mma/multiArch/${opRunOsType}/${opRunMachineArch}/bin
  fi

}

function vis_fullVerify {
  if [[ -x /opt/public/mma/bin/imapd ]] ; then
    ANT_raw "${G_myName}: $0: imapd verified"
  else
    ANT_raw "${G_myName}: $0: imapd symlinks missing -- run fullVerify"
  fi
}


