#  File name: makefile
#
#  Description: SBM
# 
# RCS: $Id: makefile,v 1.3 2002/03/22 23:05:52 mohsen Exp $
#
# :::::::::::::
SHELL=/bin/sh
LB=ar r
CFLAGS = 
LIBS_PATH = 
RANLIB = echo


PKG= $(LIBS_PATH)/gf.a
PUB_H= 
LCL_H= 
LCL_C= sbm.c
C_SRC=  $(PUB_H) $(LCL_H) $(LCL_C)
#
OBJS= sbm.o
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

all:	$(PKG) sbmtest sbmtest2

sbmtest:  sbmtest.o ${LIBS}
	${LK} -o sbmtest sbmtest.o ${LIBS} ${SYSLIBS} 

sbmtest2:  sbmtest2.o ${LIBS}
	${LK} -o sbmtest2 sbmtest2.o ${LIBS} ${SYSLIBS} 
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

