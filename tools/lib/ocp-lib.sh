#!/bin/ksh

#
# RCS Revision: $Id: ocp-lib.sh,v 1.7 2002/09/30 22:15:59 mohsen Exp $
#

# Entering here, opConfig.sh has been sourced, but nothing else

. ${opBase}/bin/opBases.sh

# Now opBases variables are also available,

. ${opLibBase}/opOutput.sh
. ${opLibBase}/ocp-lineNu.sh

#. ${opLibBase}/ocp-tests.sh

. ${opLibBase}/printBash.sh 

. ${opLibBase}/ocp-general.sh

#. ${opLibBase}/ocp-sup.sh

# ./opRunEnvLib.sh 
. ${opLibBase}/opRunEnvLib.sh

opRunEnvGet
opRunUserGet

. ${opBinBase}/opBasesSite.sh

# ./autoItemsSelect.sh
# ./itemsLib.sh 
. ${opLibBase}/itemsLib.sh

# ./opWrappersLib.sh 
. ${opLibBase}/opWrappersLib.sh

# NOTYET, this should go into seedAction.sh
# ./visLib.sh
. ${opLibBase}/visLib.sh

. ${opLibBase}/opDoLib.sh


