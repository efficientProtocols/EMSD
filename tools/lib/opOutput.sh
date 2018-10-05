#!/bin/osmtKsh

#
# RCS Revision: $Id: opOutput.sh,v 1.1 2002/03/20 00:20:00 mohsen Exp $
#

#
# Author: Mohsen Banan.
# 
#

#
# ANT_   Announce/Note Terse
# ANV_   Announce/Note Verbose
# 
# stdOutput  echo/print to stdout
#

alias ANT_cooked='tm_fileName=$0; tm_lineNo=$LINENO; ant_cooked'
alias ANV_cooked='tm_fileName=$0; tm_lineNo=$LINENO; anv_cooked'

alias stdOutput='print -r --'

function ANT_raw {
  print -u2 -r -- "$*"
}

function ANV_raw {
  if [[ "${G_verbose}_" == "verbose_" ]] ; then
    print -u2 -r -- "$*"
  fi
}

function ant_cooked {
  print -u2 -r -- "NOTE,${G_progName},${tm_fileName},${tm_lineNo}: $*"
}

function anv_cooked {
  if [[ "${G_verbose}_" == "verbose_" ]] ; then
    print -u2 -r -- "NOTE,${G_progName},${tm_fileName},${tm_lineNo}: $*"
  fi
}




