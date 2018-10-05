mkpDefaultFile=${CURENVBASE}/tools/mkpDefault.parSh

if [[ -f ${mkpDefaultFile} ]] ; then
  . ${mkpDefaultFile}
else
  ANT_raw "$0: Use mkpParamsFilesReGen. ${mkpDefaultFile}"
  return 1
fi

kitPrepBase="${CURENVBASE}/kitPrepBase/${targetProdName}-${targetOS}-${targetArchType}"
