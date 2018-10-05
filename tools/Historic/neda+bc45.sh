# RCS: $Id: neda+bc45.sh,v 1.1 2002/10/25 23:47:41 mohsen Exp $

SHELL=/bin/sh
MAKE=make	
#MAKE=echo	
CC="bcc"
AS="tasm"
LB="${MKSPATH}\sh.exe aelib.ksh"
RANLIB="echo Done Building"
LK="${MKSPATH}\sh.exe aelink.ksh"
ETAGS=etags


PINEDIR=""

#SYSLIBS="${KSHCURENVBASE}/results/sol2/libc/sf.a -lsocket -lnsl -lucb"

LIBS_PATH=""
BIN_PATH=""
MAN_PATH=""
DATA_PATH=""

DEFS=""
DEFS="${DEFS} -ml"                    #Large Model

INCL="-I."
INCL="${INCL};${KSHCURENVBASE}\\\\src\\\\ae-if\\\\hw-inc"
INCL="${INCL};${KSHCURENVBASE}\\\\src\\\\neda\\\\this\\\\ae\\\\include"
INCL="${INCL};${KSHCURENVBASE}\\\\src\\\\neda\\\\sf.ocp\\\\ae\\\\include"
INCL="${INCL};${KSHCURENVBASE}\\\\src\\\\neda\\\\pf.ocp\\\\include"
INCL="${INCL};c:\\\\bc45\\\\include"

CFLAGS="${DEFS} ${INCL}"

CFLAGS="$CFLAGS -r- -DPDREMOTE -1- -x- -f- -d -w  -Od"


