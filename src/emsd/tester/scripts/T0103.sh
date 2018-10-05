#/bin/sh
# RCS Id: $Id: T0103.sh,v 1.1.1.1 2002/10/24 19:50:03 mohsen Exp $

TMP_FILE="/tmp/IT$$.tmp"

UA_SPOOL=${UA_SPOOL:-'../spool/emsd-ua/submitNew'}
TEST_SESSION=${TEST_SESSION:-`date "+Test Session on %D"`}

MIN=`date '+%M'`
MSGID=`date '+T0103_%a.%T'`
DATE=`date '+%c'`

rm -f ${TMP_FILE}

echo "From emsduser@neda.com ${DATE}"		         > ${TMP_FILE}
echo "Date: ${DATE}"					>> ${TMP_FILE}
echo "From: EMSD=1234"					>> ${TMP_FILE}
echo "To: EMSD=1234"					>> ${TMP_FILE}
echo "Cc: EMSD=2345"					>> ${TMP_FILE}
echo "Bcc: EMSD=3456"					>> ${TMP_FILE}
echo "Subject: Test 1.3  ${DATE}"			>> ${TMP_FILE}
echo "MIME-Version: 1.0"				>> ${TMP_FILE}
echo "Content-Type: TEXT/PLAIN; charset=US-ASCII"	>> ${TMP_FILE}
echo ""							>> ${TMP_FILE}
echo "Test Session ID: ${TEST_SESSION}"			>> ${TMP_FILE}
date							>> ${TMP_FILE}

cp /tmp/NedaTest.tmp ${UA_SPOOL}/S_0103${MIN}
