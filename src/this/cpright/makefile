#
#  (C) Copyright Neda Communications, Inc 1995. All rights reserved.
# 
#  All Rights Reserved.
# 
#  This software is furnished under a license and use, duplication,
#  disclosure and all other uses are restricted to the rights specified
#  in the written license between the licensee and copyright holders.
#
#  File name: makefile
#
#
# RCS: $Id: makefile,v 1.5 2002/03/22 23:06:28 mohsen Exp $
#
# :::::::::::::
SHELL=/bin/sh
LB=ar r
CFLAGS = 
LIBS_PATH = 
RANLIB = echo


PKG= $(LIBS_PATH)/relid.a

PUB_H= 
LCL_H= 
LCL_C= notice.c  neda.c 
C_SRC=  $(PUB_H) $(LCL_H) $(LCL_C)
#
OBJS= notice.o neda.o
#
MAN= 

#echo ${SUFFIXES}
#
$(PKG): $(OBJS) FORCE
	${LB}  ${PKG} ${OBJS}
	${RANLIB} ${PKG}
#

FORCE:

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


FORCE: 

