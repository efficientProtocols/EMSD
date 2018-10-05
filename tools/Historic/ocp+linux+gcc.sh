# RCS: $Id: ocp+linux+gcc.sh,v 1.1 2002/10/25 23:47:42 mohsen Exp $

#SHELL=/bin/sh
SHELL=/bin/ksh
MAKE=gmake	
CC=gcc
AS="as -"
LB="ar r"
RANLIB=ranlib
LK="gcc -g"
ETAGS=etags
#SYSLIBS="-lsocket -lnsl -lucb -lcurses -ltermcap"
#SYSLIBS="-lsocket -lnsl -lcurses -ltermcap /usr/ucblib/libucb.a
SYSLIBS="-lnsl -lcurses"
#
# ../results/linux/
LIBS_PATH=${CURENVBASE}/results/linux/libc
BIN_PATH=${CURENVBASE}/results/linux/bin
MAN_PATH=${CURENVBASE}/results/share/man
DATA_PATH=${CURENVBASE}/results/share/data

DEFS=""
DEFS="${DEFS} -D TM_ENABLED"
DEFS="${DEFS} -D SOLARIS_2_5"
#DEFS="${DEFS} -DUCBSIGNALS -DUCBCURSES"
DEFS="${DEFS} -g  -Wall"


# NOTYET: This should come from sourcing of 
# target.sh
#

if [ "${targetPDT}" = "" ] ; then
  targetPDT=UADEVL.MUL
  #targetPDT=OCPDEVL.MUL
  #targetPDT=BASEUA.ALL
fi

case ${targetPDT} in
  'esrosSDK' )
      INCL="-I."
      INCL="${INCL} -I${CURENVBASE}/include -I${CURENVBASE}/include/esros"
      INCL="${INCL} -I${CURENVBASE}/include/sysif"
      ;;
  'OCPDEVL.MUL' )
    INCL="-I."
    INCL="${INCL} -I${CURENVBASE}/src/esros/include"
    INCL="${INCL} -I${CURENVBASE}/src/this/include.linux"
    INCL="${INCL} -I${CURENVBASE}/src/multiocp/sysif.ocp/include.linux"
    INCL="${INCL} -I${CURENVBASE}/src/multiocp/sysif.ocp/os/linux"
    INCL="${INCL} -I${CURENVBASE}/src/multiocp/sysif.ocp/oe/unix/sysv"
    INCL="${INCL} -I${CURENVBASE}/src/multiocp/include.ocp"
    ;;
  'VORDE' )
    INCL="-I."
    INCL="${INCL} -I${CURENVBASE}/src/mts_ua/include"
    INCL="${INCL} -I${CURENVBASE}/src/esros/include"
    INCL="${INCL} -I${CURENVBASE}/src/this/include.linux"
    INCL="${INCL} -I${CURENVBASE}/src/multiocp/sysif.ocp/include.linux"
    INCL="${INCL} -I${CURENVBASE}/src/multiocp/sysif.ocp/os/linux"
    INCL="${INCL} -I${CURENVBASE}/src/multiocp/sysif.ocp/oe/unix/sysv"
    INCL="${INCL} -I${CURENVBASE}/src/multiocp/include.ocp"
    ;;
  'UADEVL.MUL' )
    INCL="-I."
    INCL="${INCL} -I${CURENVBASE}/src/this/include.linux"
    INCL="${INCL} -I${CURENVBASE}/src/multiocp/sysif.ocp/include.linux"
    INCL="${INCL} -I${CURENVBASE}/src/mts_ua/include"
    INCL="${INCL} -I${CURENVBASE}/src/esros/include"
    INCL="${INCL} -I${CURENVBASE}/src/multiocp/include.ocp"
    ;;
  * )
    echo "$0: invalid Target CPU: ${targetCPU}"
    exit 1
    ;;
esac


CFLAGS="${DEFS} ${INCL}"

