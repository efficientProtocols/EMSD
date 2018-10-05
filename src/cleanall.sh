#!/bin/sh
TEMP_FILE=/tmp/cleanAll.$$
rm -f $TEMP_FILE

SRCS=" \
    ."



echo "In `pwd`, Creating list ..."

find . -type f    \( -name '#*#'   \
    -o  -name '~*'     \
    -o  -name '*~'     \)    -print  > $TEMP_FILE

find ${SRCS} -type f    \( -name '*.o' \
    -o  -name '*.map'     \
    -o  -name '*.lst'     \
    -o  -name '*.rsp'     \
    -o  -name '*tlink.cfg'     \
    -o  -name '*.exe'     \
    -o  -name '*.obj' \)    -print  >> $TEMP_FILE

echo "$TEMP_FILE" >> $TEMP_FILE

TO_DELETE=`cat $TEMP_FILE | tr x x`
echo "In `pwd`, Removing ..."
echo $TO_DELETE
rm $TO_DELETE


exit 0
