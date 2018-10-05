#/bin/sh
# RCS Id: $Id: T0104.sh,v 1.1.1.1 2002/10/24 19:50:03 mohsen Exp $

TMP_FILE="/tmp/IT$$.tmp"

UA_SPOOL=${UA_SPOOL:-'../spool/emsd-ua/submitNew'}
TEST_SESSION=${TEST_SESSION:-`date "+Test Session on %D"`}

MIN=`date '+%M'`
SEC=`date '+%S'`
MSGID=`date '+T0104_%a.%T'`
DATE=`date '+%c'`

rm -f ${TMP_FILE}

echo "From emsduser@neda.com ${DATE}"		         > ${TMP_FILE}
echo "Date: ${DATE}"					>> ${TMP_FILE}
echo "From: EMSD=1234"					>> ${TMP_FILE}
echo "To: 2345@emsd.com"					>> ${TMP_FILE}
echo "Subject: Test 1.4  ${DATE}"			>> ${TMP_FILE}
echo "MIME-Version: 1.0"				>> ${TMP_FILE}
echo "Content-Type: TEXT/PLAIN; charset=US-ASCII"	>> ${TMP_FILE}
echo ""							>> ${TMP_FILE}
echo "Test Session ID: ${TEST_SESSION}"			>> ${TMP_FILE}
date							>> ${TMP_FILE}

cp ${TMP_FILE}  ${UA_SPOOL}/S_0104${MIN}
