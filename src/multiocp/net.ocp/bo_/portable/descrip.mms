#
#  Description: 
#
#
# SCCS Revision: @(#)descrip.mms	1.1    Released: 13 Mar 1990
#
# :::::::::::::
BASE = uv2$dua1:[mb.c.]
CFLAGS = /NOLIST/OBJECT=$(MMS$TARGET_NAME)/DIAGNOSTICS=$(MMS$TARGET_NAME)/INCLUDE_DIRECTORY=$(BASE)[include]
#LINKFLAGS = 
LIBS_PATH = $(BASE)[olb]
EXE_PATH = uv2$dua1:[mb.exe]

PDT = $(EXE_PATH)bo_ex.exe
PUB_H =
LCL_H =
LCL_C = bo_ex.c
C_SRC =  ${PUB_H} ${LCL_H} ${LCL_C}
OBJS = bo_ex.obj
#
GF = $(LIBS_PATH)gf
PORTF = $(LIBS_PATH)osiportf
SYSF = $(LIBS_PATH)sysf
#LIBS = $(GF)/lib,$(PORTF)/lib,$(SYSF)/lib
LIBS = $(GF)/lib
#LIBS_DEP = $(GF).olb,$(PORTF).olb,$(SYSF).olb
LIBS_DEP = $(GF).olb

#
$(PDT) :       $(OBJS) $(LIBS_DEP)
	$(LINK) $(LINKFLAGS) $(OBJS),$(LIBS)
#
e_pub_h :
	@echo ${PUB_H}
e_lcl_h :
	@echo ${LCL_H}
e_lcl_c :
	@echo ${LCL_C}
e_c_src :
	@echo ${C_SRC}
e_objs :
	@echo ${OBJS}
e_libs :
	@echo ${LIBS}
e_pdt :
	@echo ${PDT}
#
tags :  ${LCL_C}
	ctags $(LCL_C)

