#
#  Description: Simele sender (sendt.c) and receiver (sch_ex.c) using socket
#
#
# RCS  Revision: $Id: makefile,v 1.3 2002/03/22 23:06:18 mohsen Exp $
#
# :::::::::::::

PDT1 = sch_ex
PDT2 = sendt
PUB_H =
LCL_H = 
LCL_C =  sch_ex.c 
C_SRC =  ${PUB_H} ${LCL_H} ${LCL_C}
OBJS  =  sch_ex.o
C_SRC =  ${PUB_H} ${LCL_H} ${LCL_C}
#
all:	$(PDT1) $(PDT2)

$(PDT1):       $(OBJS)
	 ${LK} -o ${PDT1} ${OBJS}  ${SYSLIBS}

# -lsocket -lnsl -lucb 

$(PDT2): 	sendt.o sendt.c
	 ${LK} -o ${PDT2} sendt.o ${LIBS} ${SYSLIBS}

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
e_pdt1:
	@echo ${PDT1}
e_pdt2:
	@echo ${PDT2}
e_ds:
	@echo ${DS}
#
tags:  ${LCL_C}
	ctags $(LCL_C)


FORCE: 

