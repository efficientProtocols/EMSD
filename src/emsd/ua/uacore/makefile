#
#  Description:
#
#
# RCS: $Id: makefile,v 1.2 2002/10/25 07:46:36 mohsen Exp $
#
# :::::::::::::
#
#
# Product Specific Stuff
#
#PDT = $(BIN_PATH)/
PKG= $(LIBS_PATH)/uacore.a
PUB_H =
LCL_H =
LCL_C =
C_SRC =  ${PUB_H} ${LCL_H} ${LCL_C}
OBJS =	fromsh.o	\
	emsdcomp.o	\
	esroevt.o	\
	msgid.o		\
	oper.o		\
	submit.o	\
	tosh.o		\
	ua.o		\
	uadup.o		\
	uasubr.o	\
	uperform.o	\
	upreproc.o	\
	delctrl.o

MAN =
INFO =
#

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

