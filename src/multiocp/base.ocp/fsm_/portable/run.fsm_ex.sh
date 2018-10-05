#!/bin/csh
# RCS Version: $Id: run.fsm_ex.sh,v 1.2 1996/01/19 19:32:48 mohsen Exp $

if ( "$1" != "" &&  "$1" != "2" ) then 
echo Usage: "$0 {2}"
exit
endif

fsm_ex$1 -T FSM_,fff3
#fsm_ex$1 -T FSM_,fff3 -T SCH_,ffff
