#!/bin/csh 
#
# Standalone compilation of a header file (.h)
#
if ( "$1" == "" ) then 
echo Usage: "$0 {1|2}"
exit
endif

#./sch_ex$1 -T SCH_,01
./sch_ex$1
