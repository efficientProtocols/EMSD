#  File name: makefile
#
#  Description: EROP Module (ESROS Engine)
#
# RCS: $Id: makefile,v 1.6 2002/10/25 19:37:48 mohsen Exp $
#
# :::::::::::::
SHELL=/bin/sh
LB=ar r
CFLAGS = 
LIBS_PATH = 
RANLIB = echo


PKG= $(LIBS_PATH)/fsm.a
PUB_H= 
LCL_H= 
LCL_C= fsm.c fsmdepen.c trnsdgrm.c 
C_SRC=  $(PUB_H) $(LCL_H) $(LCL_C)
#
OBJS= fsm.o fsmdepen.o trnsdgrm.o 
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

all:	$(PKG) fsm_ex fsm_ex2


fsm_ex:  fsm_ex.o ${LIBS}
	${LK} -o fsm_ex fsm_ex.o ${LIBS} ${SYSLIBS} 

fsm_ex2:  fsm_ex2.o ${LIBS}
	${LK} -o fsm_ex2 fsm_ex2.o ${LIBS} ${SYSLIBS} 
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

