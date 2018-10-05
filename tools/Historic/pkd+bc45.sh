# RCS: $Id: pkd+bc45.sh,v 1.1 2002/10/25 23:47:43 mohsen Exp $

SHELL=sh
MAKE=make	
#MAKE=echo	
CC="bcc"
AS="tasm"

PKDBASE="${KSHCURENVBASE}\\\\src\\\\cscdpd.pkd"

. ${CURENVBASE}/tools/model.sh

case ${mkpModel} in
'SMALL' )
    modelLetter="s"
    ;;
'MEDIUM' )
    modelLetter="m"
    #SYSLIBS="${PKDBASE}\\\\tools\\\\algaem.lib ${PKDBASE}\\\\tools\\\\cm.lib"
    SYSLIBS="algaem.lib cm.lib"
    ;;
'LARGE' )
    modelLetter="l"
    SYSLIBS="${PKDBASE}\\\\tools\\\\algael.lib"
    ;;
* )
    echo "$0: invalid Model: ${model}"
    exit 1
    ;;
esac

#When Debigging, LK="${MKSPATH}\sh.exe doslink.ksh -T 9"
LK="${MKSPATH}\sh.exe doslink.ksh -m ${mkpModel}"
LB="${MKSPATH}\sh.exe aelib.ksh -m ${mkpModel}"

RANLIB="echo Done Building"
ETAGS=etags

LIBS_PATH=""
BIN_PATH=""
MAN_PATH=""
DATA_PATH=""

DEFS=""
DEFS="${DEFS} -m${modelLetter}"
DEFS="${DEFS} -v"
DEFS="${DEFS} -DCPU=PC"
DEFS="${DEFS} -DBOARD=SIM"
DEFS="${DEFS} -DdbgASSERT_LEVEL=2"

INCL="-I."
INCL="${INCL};${PKDBASE}\\\\cc"
INCL="${INCL};${PKDBASE}\\\\cdpi"
INCL="${INCL};${PKDBASE}\\\\cmd"
INCL="${INCL};${PKDBASE}\\\\csccp"
INCL="${INCL};${PKDBASE}\\\\csme"
INCL="${INCL};${PKDBASE}\\\\mc"
INCL="${INCL};${PKDBASE}\\\\mgmt"
INCL="${INCL};${PKDBASE}\\\\os"
INCL="${INCL};${PKDBASE}\\\\sn"
INCL="${INCL};${PKDBASE}\\\\main"
INCL="${INCL};${PKDBASE}\\\\inc"
INCL="${INCL};c:\\\\bc45\\\\include"


CFLAGS="${DEFS} ${INCL}"

#CFLAGS="$CFLAGS "


