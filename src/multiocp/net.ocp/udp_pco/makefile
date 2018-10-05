#  
#  (C) Copyright Neda Communications, Inc 1995. All rights reserved.
# 
#  All Rights Reserved.
# 
#  License for uses of this software with GPL-incompatible software
#  (e.g., proprietary, non Free) can be obtained from Neda Communications, Inc.
#  Visit http://www.neda.com/ for more information.
#
#
#  File name: makefile
#
#  Description: UDP Interface module
#
# RCS: $Id: makefile,v 1.4 2002/10/25 19:37:54 mohsen Exp $
#
# :::::::::::::
SHELL=/bin/sh
LB=ar r
CFLAGS = 
LIBS_PATH =
RANLIB = echo


PKG= $(LIBS_PATH)/udp_pco.a
PUB_H= 
LCL_H= 
LCL_C= udp_if.c udp_po.c udp_pc.c
C_SRC=  $(PUB_H) $(LCL_H) $(LCL_C)
#
OBJS= udp_if.o udp_po.o udp_pc.o
#
MAN= 

#
$(PKG): $(OBJS) FORCE
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
e_pkg:
	@echo ${PKG}
e_man:
	@echo ${MAN}
#
ctags:  ${LCL_C}
	ctags ${LCL_C}
#

FORCE: 

