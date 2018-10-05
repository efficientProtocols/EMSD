#
#  Description: 
#
#
# RCS  Revision: $Id: Makefile,v 1.2 1999/09/27 06:18:49 mohsen Exp $
#
# :::::::::::::
#
PUB_H =  int2english.h
LCL_H = 
LCL_C = int2english.c
C_SRC =  ${PUB_H} ${LCL_H} ${LCL_C}
OBJS  = int2english.o
#
PKG= $(LIBS_PATH)/gf.a

#
$(PKG):       $(OBJS)
	${LB} ${PKG} ${OBJS}  
	${RANLIB} ${PKG}
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
e_ds:
	@echo ${DS}
#
tags:  ${LCL_C}
	ctags $(LCL_C)

