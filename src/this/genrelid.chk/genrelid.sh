#!/bin/ksh
#
# RCS: $Id: genrelid.sh,v 1.1.1.1 1998/01/30 00:50:07 mohsen Exp $
#
# Syntax:
# Description:
#
#

#
# Customize the following on a per-product basis
# Default Values are set below.
# 

RELID_RELINFO_FILE=${CURENVBASE}/relnotes/moduleName.tex
RELID_TEMPLATE_FILE=./relid.tpl
STATUS_STRING="is derivative work based on sources which were released on"
#STATUS_STRING="released on"

#set -- $(getopt -c $0 t: "$@")
set -- `getopt  t:r:s: $*`

if [ $? -ne 0 ]
then 
    print >&2 "Usage: $0 [-t relidTemplateFile] [-r relInfoFile] [-s statusString]"
    print >&2 "statusString: is text for either derivative work or final release"
    exit
fi

for i in $* 
do
    case $i in
    -t)   RELID_TEMPLATE_FILE=$2; shift 2;;
    -r)   RELID_RELINFO_FILE=$2; shift 2;;
    -s)   RELID_RELINFO_FILE=$2; shift 2;;
    --)   shift; break;;
    esac
done

echo "RELID_RELINFO_FILE is set to: $RELID_RELINFO_FILE"


#
# grovel around for the release information
#

RELID_PROD_NAME=`grep RELID-INFO-Product-Name $RELID_RELINFO_FILE | cut -d: -f2`
RELID_REVNUM=`grep RELID-INFO-Rev-Number $RELID_RELINFO_FILE | cut -d$ -f2 | cut -d' ' -f 2`
RELID_REL_DATE=`grep RELID-INFO-Rel-Date $RELID_RELINFO_FILE | cut -d$ -f2 | cut -d' ' -f 2,3`
RELID_CVS_TAG=`grep RELID-INFO-CVS-Tag $RELID_RELINFO_FILE | cut -d$ -f2 | tr -s ' ' | cut -d' ' -f 2`
RELID_BUILD_USER=${USER}
RELID_BUILD_HOST="`uname -n` `uname -svrm`"
RELID_BUILD_DATE=`date | tr -s ' '`

if [ "${RELID_PROD_NAME}" = "" ]
then
    RELID_PROD_NAME="product unspecified"
fi

if [ "${RELID_REVNUM}" = "" ]
then
    RELID_REVNUM="revision unspecified"
fi

if [ "${RELID_REL_DATE}" = "" ]
then
    RELID_REL_DATE="release date unspecified"
fi


if [ "${RELID_CVS_TAG}" = "" ]
then
    RELID_CVS_TAG="CVS tag unspecified"
fi

echo "   RELID-INFO-Product-Name=${RELID_PROD_NAME}"
echo "   RELID-INFO-Rev-Number=${RELID_REVNUM}"
echo "   RELID-INFO-Rel-Date=${RELID_REL_DATE}"
echo "   RELID-INFO-CVS-Tag=${RELID_CVS_TAG}"
echo "   RELID-INFO-BUILD_DATE=${RELID_BUILD_DATE}"
echo "   RELID-INFO-BUILD_USER=${RELID_BUILD_USER}"
echo "   RELID-INFO-BUILD_HOST=${RELID_BUILD_HOST}"

#
# munge RELID_XXX to make work with sed -e s///
#
RELID_REL_DATE=`echo $RELID_REL_DATE | sed -e "s/\//\\\\\\\\\//g"`
RELID_BUILD_DATE=`echo $RELID_BUILD_DATE | sed -e "s/\//\\\\\\\\\//g"`


#
# 

RELID_NOTICE="${RELID_PROD_NAME} ${RELID_REVNUM} (${RELID_CVS_TAG}) \\\\n\
${STATUS_STRING} ${RELID_REL_DATE}.\\\\n\
Built on ${RELID_BUILD_DATE} by ${RELID_BUILD_USER} on ${RELID_BUILD_HOST}.\\\\n"

sed	-e "s/%RELID_NOTICE%/${RELID_NOTICE}/" \
${RELID_TEMPLATE_FILE}  > relid_p.c
