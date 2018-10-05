#  File name: makefile
#
#  Description: NVM
#
# RCS: $Id: makefile,v 1.3 2002/03/22 23:06:08 mohsen Exp $
#
# :::::::::::::
SHELL=/bin/sh
LB=ar r
CFLAGS = 
LIBS_PATH = 
RANLIB = echo


PKG= $(LIBS_PATH)/sf.a
PUB_H= 
LCL_H= 
LCL_C= nvm.c 
C_SRC=  $(PUB_H) $(LCL_H) $(LCL_C)
#
OBJS= nvm.o
#
GF = $(LIBS_PATH)/gf.a
SF = $(LIBS_PATH)/sf.a
FSM = $(LIBS_PATH)/fsm.a
LIBS= $(GF) $(SF) $(FSM)
MAN= 

#
$(PKG): $(OBJS) FORCE
	${LB} ${PKG} ${OBJS}
	${RANLIB} ${PKG}

all:	$(PKG) nvmtest

nvmtest:  nvmtest.o ${LIBS}
	${LK} -o nvmtest nvmtest.o ${LIBS} ${SYSLIBS} 
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
e_pkg:
	@echo ${PKG}
e_man:
	@echo ${MAN}
#
ctags:  ${LCL_C}
	ctags ${LCL_C}



FORCE: 

