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
#  Description: mailer
#
# RCS: $Id: makefile,v 1.1.1.1 2002/10/24 19:50:03 mohsen Exp $
#
# :::::::::::::
#

#
# Product Specific Stuff
#
PDT= $(BIN_PATH)/emsd-mailer
PUB_H =
LCL_H =
LCL_C = mailer.c
C_SRC =  ${PUB_H} ${LCL_H} ${LCL_C}
OBJS = mailer.o
MAN = 
INFO = 
#

LIBS = 		$(LIBS_PATH)/emsd.a		\
		$(LIBS_PATH)/mtsua.a		\
		$(LIBS_PATH)/relid.a		\
		$(LIBS_PATH)/sf.a		\
		$(LIBS_PATH)/gf.a
		

all:	$(PDT) 

$(PDT):		$(OBJS) $(LIBS)
	${LK} -o ${PDT} ${OBJS} ${LIBS}

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
	/bin/rm *.o $(PROG) 

FORCE: 

