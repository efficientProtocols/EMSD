#
#  Description: 
#
#
# RCS: $Id: makefile,v 1.3 2002/03/22 23:06:02 mohsen Exp $
#
# :::::::::::::
#
# The following are expected to be over written by mkp,
# Defaults are:
SHELL = /bin/sh
PKG= $(LIBS_PATH)/gf.a
PUB_H =
LCL_H =
LCL_C =
C_SRC =  ${PUB_H} ${LCL_H} ${LCL_C}
OBJS = sap.o sapcmp.o sapget.o
MAN = 
INFO = 
#
GF = ${LIBS_PATH}/gf.a
LIBS = # ${GF}

#
#$(PDT):       $(OBJS) ${LIBS}
#	${LK} -o ${PDT} ${OBJS} ${LIBS} 
#
$(PKG): $(OBJS) FORCE
	${LB} ${PKG} ${OBJS}
	${RANLIB} ${PKG}
#
FORCE:

all:	$(PROG1) $(PROG2) $(PROG3)

#
e_pub_h:
	@echo ${PUB_H}
e_lcl_h:
	@echo ${LCL_H}
e_lcl_c:
	@echo ${LCL_C}
e_c_src:
	@echo ${C_SRC}
e_objs:
	@echo ${OBJS}
e_libs:
	@echo ${LIBS}
e_pdt:
	@echo ${PDT}
e_man:
	@echo ${MAN}
e_info:
	@echo ${INFO}
#
TAGS:  ${LCL_C}
	${ETAGS} ${LCL_C}


FORCE: 

