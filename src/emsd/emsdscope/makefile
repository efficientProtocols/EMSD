#  
#  (C) Copyright Neda Communications, Inc 1995. All rights reserved.
# 
#  All Rights Reserved.
# 
#  License for uses of this software with GPL-incompatible software
#  (e.g., proprietary, non Free) can be obtained from Neda Communications, Inc.
#  Visit http://www.neda.com/ for more information.
#
#  File name: makefile
#
#  Description: Logger Module
#
# RCS: $Id: makefile,v 1.2 2002/10/26 07:56:10 mohsen Exp $
#
# :::::::::::::
SHELL=/bin/sh
LB=ar r
CFLAGS = -g 
LIBS_PATH = 

PDT = $(BIN_PATH)/emsdscope
PKG= 
PUB_H= 
LCL_H= 
LCL_C=

C_SRC=  $(PUB_H) $(LCL_H) $(LCL_C)
#
OBJS=
LIBS=	$(LIBS_PATH)/gf.a 	\
	$(LIBS_PATH)/sf.a 	\
	$(LIBS_PATH)/asn.a 	\
	$(LIBS_PATH)/uacore.a 	\
	$(LIBS_PATH)/mtsua.a 	\
	$(LIBS_PATH)/emsd.a	\
	$(LIBS_PATH)/relid.a	\
	$(LIBS_PATH)/gf.a	

#
MAN= 

#
all: 	$(PKG) $(PDT)

$(PKG): $(OBJS) FORCE
	${LB} ${PKG} ${OBJS}
	${RANLIB} ${PKG}

$(PDT):	emsdscope.o $(OBJS) $(LIBS)
	${LK} -o $(PDT) emsdscope.o ${OBJS} ${LIBS} ${SYSLIBS}
	#cp $(PDT) .

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

