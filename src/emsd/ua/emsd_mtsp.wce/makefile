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
# RCS: $Id: makefile,v 1.2 2002/10/25 07:46:35 mohsen Exp $
#
# :::::::::::::
#

#
# Product Specific Stuff
#
PDT = $(BIN_PATH)\emsd_mtsp.dll
PKG= $(LIBS_PATH)/
PUB_H =
LCL_H =
LCL_C =
C_SRC = ${PUB_H} ${LCL_H} ${LCL_C}

OBJS =	service.o support.o

MAN = 
INFO = 
#
LIBS =		msgstore.lib		\
		sf.lib		\
		gf.lib


all:	$(PDT)


$(PDT):		$(OBJS) $(LIBS) emsd_mtsp.rsc
	${LK} -t DLL -o ${PDT} emsd_mtsp.rsc $(OBJS) ${LIBS} ${SYSLIBS}


emsd_mtsp.rsc: emsd_mtsp.rc
	$(RC) $(RCFLAGS) -fo"emsd_mtsp.rsc" emsd_mtsp.rc
	#$(RSC) $(RSC_PROJ) emsd_mtsp.rc

PFILE=pfile.exe


DOWNLOAD : 
	$(PFILE) COPY ${PDT}  "$(PFILE_DEST)WCE:\windows\emsd_mtsp.dll"

RUN :
	$(PFILE) RUN  "$(PFILE_DEST)WCE:\windows\emsd_mtsp.dll"

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

