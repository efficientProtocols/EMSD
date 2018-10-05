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
#  Description: mtsd
#
# RCS: $Id: makefile,v 1.2 2002/10/25 08:23:51 mohsen Exp $
#
# :::::::::::::
#

#
# Product Specific Stuff
#
PDT = $(BIN_PATH)/emsd-mts
PKG= $(LIBS_PATH)/
PUB_H =
LCL_H =
LCL_C =
C_SRC = ${PUB_H} ${LCL_H} ${LCL_C}

OBJS =	deliver.o	\
	esrosevt.o	\
	mmm.o		\
	mnondel.o	\
	mperform.o	\
	mpreproc.o	\
	msgstore.o	\
	mts.o		\
	mtssubr.o	\
	oper.o		\
	rfc2emsd.o	\
	subpro.o	\
	todevice.o	\
	tomailer.o	\
	nm.o

MAN = 
INFO = 
#
LIBS =		$(LIBS_PATH)/mtsua.a		\
		$(LIBS_PATH)/emsd.a		\
		$(LIBS_PATH)/mm.a		\
		$(LIBS_PATH)/fsm.a		\
		$(LIBS_PATH)/asn.a		\
		$(LIBS_PATH)/esro_ushcb.a	\
		$(LIBS_PATH)/esroprot.a		\
		$(LIBS_PATH)/upq_bsd.a		\
		$(LIBS_PATH)/udp_pco.a		\
		$(LIBS_PATH)/relid.a		\
		$(LIBS_PATH)/gf.a		\
		$(LIBS_PATH)/sf.a		\
		$(LIBS_PATH)/gf.a		\


all:	$(PDT)

$(PDT):		$(OBJS) $(LIBS)
	${LK} -o ${PDT} $(OBJS) ${LIBS} ${SYSLIBS}

$(LIBS):

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

clean:
	/bin/rm *.o $(PROG1) $(PROG2) $(PROG3)

FORCE: 

