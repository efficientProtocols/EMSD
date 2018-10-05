#/bin/sh
# RCS Id: $Id: T0301B.sh,v 1.1.1.1 2002/10/24 19:50:03 mohsen Exp $

TMP_FILE="/tmp/IT$$.tmp"

UA_SPOOL=${UA_SPOOL:-'../spool/emsd-ua/submitNew'}
TEST_SESSION=${TEST_SESSION:-`date "+Test Session on %D"`}

MIN=`date '+%M'`
SEC=`date '+%S'`
MSGID=`date '+T0301B_%a.%T'`
DATE=`date '+%c'`

rm -f ${TMP_FILE}

echo "From emsduser@neda.com ${DATE}"		         > ${TMP_FILE}
echo "Date: ${DATE}"					>> ${TMP_FILE}
echo "From: EMSD=1234"					>> ${TMP_FILE}
echo "To: EMSD=3456"					>> ${TMP_FILE}
echo "Subject: Test 3.1  ${DATE}"			>> ${TMP_FILE}
echo "MIME-Version: 1.0"				>> ${TMP_FILE}
echo "Content-Type: TEXT/PLAIN; charset=US-ASCII"	>> ${TMP_FILE}
echo "Priority: High"					>> ${TMP_FILE}
echo ""							>> ${TMP_FILE}
echo "Test Session ID: ${TEST_SESSION}"			>> ${TMP_FILE}
date							>> ${TMP_FILE}

cp ${TMP_FILE} ${UA_SPOOL}/S_01${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_02${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_03${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_04${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_05${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_06${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_07${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_08${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_09${MIN}${SEC}

cp ${TMP_FILE} ${UA_SPOOL}/S_10${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_11${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_12${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_13${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_14${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_15${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_16${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_17${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_18${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_19${MIN}${SEC}

cp ${TMP_FILE} ${UA_SPOOL}/S_20${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_21${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_22${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_23${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_24${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_25${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_26${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_27${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_28${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_29${MIN}${SEC}

cp ${TMP_FILE} ${UA_SPOOL}/S_30${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_31${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_32${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_33${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_34${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_35${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_36${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_37${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_38${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_39${MIN}${SEC}

cp ${TMP_FILE} ${UA_SPOOL}/S_40${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_41${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_42${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_43${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_44${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_45${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_46${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_47${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_48${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_49${MIN}${SEC}
cp ${TMP_FILE} ${UA_SPOOL}/S_50${MIN}${SEC}
