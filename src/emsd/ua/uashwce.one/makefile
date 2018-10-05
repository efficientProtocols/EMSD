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
#  Description: uash
#
# RCS: $Id: makefile,v 1.2 2002/10/25 07:46:40 mohsen Exp $
#
# :::::::::::::
#

#
# Product Specific Stuff
#
PDT = $(BIN_PATH)/uashwce_one.exe
#PDT = emsd-ua.one.exe
#PDT = uashwce_one.exe
PKG= $(LIBS_PATH)/
PUB_H =
LCL_H =
LCL_C =
C_SRC = ${PUB_H} ${LCL_H} ${LCL_C}

OBJS =	uash.o uacoreif.o nm.o 

MAN = 
INFO = 
#
LIBS =		uacore.lib		\
		mtsua.lib		\
		emsd.lib		\
		erop_sh.lib		\
		esroprot.lib		\
		udp_pco.lib		\
		fsm.lib		\
		asn.lib		\
		lic.lib		\
		relid.lib		\
		gf.lib		\
		sf.lib		\
		gf.lib		


all:	$(PDT)


$(PDT):		$(OBJS) $(LIBS) emsd-ua.rsc
	${LK} -o ${PDT} emsd-ua.rsc $(OBJS) ${LIBS} ${SYSLIBS}


DEP_RSC_EMSD_U=\
	".\icon1.ico"\
	".\icon2.ico"\
	".\icon4.ico"\
	".\icon5.ico"\
	".\resource.h"

emsd-ua.rsc: emsd-ua.rc
	$(RC) $(RCFLAGS) -fo"emsd-ua.rsc" emsd-ua.rc

PFILE=pfile.exe


DOWNLOAD : 
	$(PFILE) COPY ${PDT}  "$(PFILE_DEST)WCE:\windows\uashwce_one.exe"

RUN :
	$(PFILE) RUN  "$(PFILE_DEST)WCE:\windows\uashwce_one.exe"

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

