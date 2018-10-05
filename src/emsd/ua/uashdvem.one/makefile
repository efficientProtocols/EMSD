# 
#  License for uses of this software with GPL-incompatible software
#  (e.g., proprietary, non Free) can be obtained from Neda Communications, Inc.
#  Visit http://www.neda.com/ for more information.
#
#  File name: makefile
#
#  Description: uash
#
# RCS: $Id: makefile,v 1.3 2002/10/26 07:56:11 mohsen Exp $
#
# :::::::::::::
#

#
#
PDT = $(BIN_PATH)/emsd-dev.one
PKG= $(LIBS_PATH)/
PUB_H =
LCL_H =
LCL_C =
C_SRC = ${PUB_H} ${LCL_H} ${LCL_C}

OBJS =	uash.o uacoreif.o nm.o

MAN = 
INFO = 
#
LIBS =		$(LIBS_PATH)/uacore.a		\
		$(LIBS_PATH)/mtsua.a		\
		$(LIBS_PATH)/emsd.a		\
		$(LIBS_PATH)/erop_sh.a		\
		$(LIBS_PATH)/esroprot.a		\
		$(LIBS_PATH)/udp_pco.a		\
		$(LIBS_PATH)/fsm.a		\
		$(LIBS_PATH)/asn.a		\
		$(LIBS_PATH)/mm.a		\
		$(LIBS_PATH)/relid.a		\
		$(LIBS_PATH)/gf.a		\
		$(LIBS_PATH)/sf.a		\
		$(LIBS_PATH)/gf.a		




all:	$(PDT)

$(PDT):		$(OBJS) $(LIBS)
	${LK} -o ${PDT} $(OBJS) ${LIBS} ${SYSLIBS} ${UCBLIBS}
        #cp ${PDT} .

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

