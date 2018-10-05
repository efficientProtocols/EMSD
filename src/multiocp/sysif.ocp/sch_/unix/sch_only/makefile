#
#  Description: 
#
#
# RCS  Revision: $Id: makefile,v 1.3 2002/03/22 23:06:17 mohsen Exp $
#
# :::::::::::::

PDT = sch_ex
PUB_H =
LCL_H = 
LCL_C = sch_ex.c tmr.c
C_SRC =  ${PUB_H} ${LCL_H} ${LCL_C}
OBJS  = sch_ex.o tmr.o 
C_SRC =  ${PUB_H} ${LCL_H} ${LCL_C}
#
GF = $(LIBS_PATH)/gf.a
SF = $(LIBS_PATH)/sf.a
LIBS = $(GF) $(SF)


#
All:       $(PDT) sendt

$(PDT):       $(OBJS) ${LIBS}
	 ${LK} -o ${PDT} ${OBJS} ${LIBS} -lsocket -lnsl -lucb 

sendt: sendt.o sendt.c ${LIBS}
	 ${LK} -o sendloc sendt.o ${LIBS} -lsocket -lnsl -lucb 
	 cp sendloc sendrem
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
e_ds:
	@echo ${DS}
#
tags:  ${LCL_C}
	ctags $(LCL_C)


FORCE: 

