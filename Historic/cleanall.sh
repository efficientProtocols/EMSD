#!/bin/sh
TEMP_FILE=/tmp/cleanAll.$$
rm -f $TEMP_FILE

#cd ./src

echo "In `pwd`, Creating list ..."

find . -type f    \( -name '#*#'   \
    -o  -name '~*'     \
    -o  -name '*~'     \)    -print  > $TEMP_FILE

find ./src  -type f    \( -name '*.o' \
    -o  -name '*.cfg' \
    -o  -name '*.lst' \
    -o  -name '*.rom' \
    -o  -name '*.axe' \
    -o  -name '*.loc' \
    -o  -name '*.map' \
    -o  -name '*.rsp' \
    -o  -name '*.rt'  \
    -o  -name '*.tr'  \
    -o  -name '*.bak'  \
    -o  -name '*.TR'  \
    -o  -name '*vc50.pdb'  \
    -o  -name '*vc60.pdb'  \
    -o  -name 'vc60.pdb'  \
    -o  -name '*.pdb'  \
    -o  -name '*.ilk'  \
    -o  -name '*.exe'  \
    -o  -name 'monitor.vox'  \
    -o  -name '*.obj' \)    -print  >> $TEMP_FILE

#    -o  -name '*.PD'  \

find ./results\
     -type f    \( -name '*.lib' \
     -o  -name '*.a' \
     -o  -name '*.bak' \)    -print  >> $TEMP_FILE

echo "$TEMP_FILE" >> $TEMP_FILE

TO_DELETE=`cat $TEMP_FILE | tr x x`
echo "In `pwd`, Removing ..."
echo $TO_DELETE
rm $TO_DELETE

rm -fr bindist

# NOTYET: Verify that $CURENVBASE is set
# NOTYET: These should be moved into the 
#         appropriate dirs cleanall.sh
#         just like buildall.sh
#

if [ ! -d $CURENVBASE/openSrc/other ] ;  then
  exit
fi



case `uname -s` in
'SunOS')
  cd $CURENVBASE/openSrc/other/fetchMail/cygwin
  make distclean

  #cd $CURENVBASE/results/sol2
  #rm -fr bin/* man/* libc/*
  cd $CURENVBASE/openSrc/other/procMail/unix
  make clean
  ;;
'CYGWIN-NT4.0')
  cd $CURENVBASE/results/nt4
  rm -fr bin/* man/* libc/*
  cd $CURENVBASE/openSrc/other/procMail/cygwin
  rm autoconf.h
  rm install.bin
  make clean
  ;;
'CYGWIN_NT-5.0')
  cd $CURENVBASE/results/winnt/x86
  rm -fr bin/* man/* libc/*
  #cd $CURENVBASE/openSrc/other/procMail/cygwin
  #rm autoconf.h
  #rm install.bin
  #make clean
  ;;
esac

exit 0
