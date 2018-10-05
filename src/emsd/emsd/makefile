#
#  Description:
#
#
# RCS: $Id: makefile,v 1.1.1.1 2002/10/24 19:50:03 mohsen Exp $
#
# :::::::::::::
#
#
# Product Specific Stuff
#
#PDT = $(BIN_PATH)/
PKG= $(LIBS_PATH)/emsd.a
PUB_H =
LCL_H =
LCL_C =
C_SRC =  ${PUB_H} ${LCL_H} ${LCL_C}

OBJS =	envcomp0.o envcomp1.o \
	ipmcomp0.o ipmcomp1.o \
	prbcomp0.o prbcomp1.o \
	rptcomp0.o rptcomp1.o \
	errcomp0.o errcomp1.o
MAN =
INFO =
#
LIBS = ${LIBS_PATH}/ocp.a

#
#$(PDT):       $(OBJS) ${LIBS}
#	${LK} -o ${PDT} ${OBJS} ${LIBS}
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


FORCE: 

