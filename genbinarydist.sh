#!/bin/ksh

if test "" = $CURENVBASE
then
  echo you must set CURENVBASE by running sourceme.csh or dotme.sh
  exit 1
fi

# which platform are we on?

os=`uname -s`

if test $os = "SunOS"
then
  platform='sol2'
else
  platform='nt4'
fi

cd $CURENVBASE
pwd
mkdir bindist
cd results/$platform
pwd
tar cf - bin libc man | (cd $CURENVBASE/bindist; tar xpf -)
cd $CURENVBASE
tar cf - examples | (cd $CURENVBASE/bindist; tar xpf -)

