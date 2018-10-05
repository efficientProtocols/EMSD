#  
#  (C) Copyright Neda Communications, Inc 1995. All rights reserved.
# 
#  All Rights Reserved.
# 
#  This software is furnished under a license and use, duplication,
#  disclosure and all other uses are restricted to the rights specified
#  in the written license between the licensee and copyright holders.
#
#  File name: makefile
#
#  Description: Scheduler with UDP 
#
# RCS: $Id: makefile,v 1.6 2002/10/25 19:38:01 mohsen Exp $
#
# :::::::::::::

PDT1 = sch_ex1 
PDT2 = sch_ex2
PUB_H =
LCL_H = 
LCL_C1 = sch_ex1.c nm.c 
LCL_C2 = sch_ex2.c  
C_SRC =  ${PUB_H} ${LCL_H} ${LCL_C1}
C_SRC =  ${PUB_H} ${LCL_H} ${LCL_C2}
OBJS1  = sch_ex1.o nm.o
OBJS2  = sch_ex2.o 
#
UPQ = $(LIBS_PATH)/upq_bsd.a
UDP = $(LIBS_PATH)/udp_pco.a
PROT_ENG = $(LIBS_PATH)/esroprot.a
GF = $(LIBS_PATH)/gf.a
SF = $(LIBS_PATH)/sf.a
LIBS =  $(UDP) $(UPQ) $(GF) $(SF) 


#
all:	$(PDT1) $(PDT2) sendt

$(PDT1):       $(OBJS1) ${LIBS}
	 ${LK} -o ${PDT1} ${OBJS1} ${LIBS} ${SYSLIBS}
#-lsocket -lnsl -lucb 

$(PDT2):       $(OBJS2)
	 ${LK} -o ${PDT2} ${OBJS2} ${LIBS} ${SYSLIBS}

sendt: 	sendt.o sendt.c nm.c ${LIBS}
	 ${LK} -o sendrem sendt.o nm.o ${LIBS} ${SYSLIBS}
#-lsocket -lnsl -lucb 
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

