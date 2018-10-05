#!/bin/ksh


. ${CURENVBASE}/tools/ocp-lib.sh

. ${CURENVBASE}/tools/buildallhead.sh


# don't touch these top two! 


HERE=`pwd`

#
# Pre Build Work
#

# verify that the right target.h is in place
# Announce the specific product being built
#


#
# Build the product
#

buildAndRecord ${HERE}/src "${buildAllCmd}"

exit

buildAndRecord ${HERE}/openSrc "${buildAllCmd}"

#
# Install The Product
#

#
# Clean Up
#



exit 0;
