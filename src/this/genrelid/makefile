#
#
# RCS: $Id: makefile,v 1.15 2002/11/05 04:05:01 mohsen Exp $
#
# :::::::::::::
#SHELL=/bin/sh
LB=ar r
CFLAGS = 
LIBS_PATH = 
RANLIB = echo


PKG= $(LIBS_PATH)/relid.a
PUB_H= relid.h
LCL_H= 
LCL_C= relid.c
C_SRC=  $(PUB_H) $(LCL_H) $(LCL_C)
#
OBJS= relid.o
#
MAN= 

#
$(PKG): alwaysDo $(OBJS) 
	${LB} ${PKG} ${OBJS}
	${RANLIB} ${PKG}
#

#GENRELID= ${SHELL} ./genrelid.sh -t ./relid.tpl -r ${CURENVBASE}/relnotes/ocpdevl-mul.tex -o relid.c

#GENRELID= ${SHELL} ./genrelid.sh -t ./relid.tpl -r ${CURENVBASE}/relnotes/baseua-mul-beta.ttytex -o relid.c

#GENRELID= ${SHELL} ./genrelid.sh -t ./relid.tpl -r ${CURENVBASE}/relnotes/esrosFull-mul-gold.ttytex -o relid.c

# Name of the relid file comes from tools/target.sh
GENRELID= ${SHELL} ./genrelid.sh -t ./relid.tpl  -o relid.c
#GENRELID= genrelid.sh -t ./relid.tpl  -o relid.c

alwaysDo:
	${GENRELID}

relid.c:	relid.tpl
	${GENRELID}

clean:
	-rm relid.c

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

