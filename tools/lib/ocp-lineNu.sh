#!/bin/ksh

#
# RCS Revision: $Id: ocp-lineNu.sh,v 1.6 2002/05/17 00:44:15 mohsen Exp $
#

#
# Author: Mohsen Banan.
# 
#


alias TM_trace='tm_fileName=$0; tm_lineNo=$LINENO; tm_trace'
alias LOG_event='tm_fileName=$0; tm_lineNo=$LINENO; log_event'
alias LOG_message='tm_fileName=$0; tm_lineNo=$LINENO; log_event'
alias EH_oops='print -u2 -- "EH_,$0,$LINENO: OOPS"'
alias EH_problem='tm_fileName=$0; tm_lineNo=$LINENO; eh_problem'
alias EH_fatal='tm_fileName=$0; tm_lineNo=$LINENO; eh_fatal'
alias EH_exit='tm_fileName=$0; tm_lineNo=$LINENO; eh_fatal'
alias EH_assert='opDoAssert eval '

tm_trace() {
  # $1 = traceNu
  # $2 = traceString

	#print "in $0"

  if [ ${traceLevel} ]
  then
    if test $1 -le ${traceLevel}
    then
      shift
      print -u2 -- "TM_,${tm_fileName},${tm_setFile},${tm_lineNo}: $*"
    fi
  fi
}

log_event() {
  print -u2 -r -- "LOG_,${tm_fileName},${tm_setFile},${tm_lineNo}: $*"
}

eh_problem() {
  print -u2 -r -- "EH_,${G_myName},${tm_fileName},${tm_lineNo}: PROBLEM: $*"
}

eh_fatal() {
  print -u2 -r -- "EH_,${tm_fileName},${tm_lineNo}: FATAL $*"
  exit 1
}
