#
#  Description: 
#
#
# SCCS Revision: %W%    Released: %G%
#
# :::::::::::::
BASE = uv2$dua1:[mb.c.]
#CFLAGS = /NOLIST/DEBUG/NOOPTIMIZE/OBJECT=$(MMS$TARGET_NAME)/DIAGNOSTICS=$(MMS$TARGET_NAME)/INCLUDE_DIRECTORY=$(BASE)[include]
CFLAGS = /NOLIST/OBJECT=$(MMS$TARGET_NAME)/DIAGNOSTICS=$(MMS$TARGET_NAME)/INCLUDE_DIRECTORY=$(BASE)[include]
#LINKFLAGS = /DEBUG
LIBS_PATH = $(BASE)[olb]
EXE_PATH = uv2$dua1:[mb.exe]

PDT = all
PDT1 = $(EXE_PATH)tmr_ex.exe
PDT2 = $(EXE_PATH)sf_quex.exe
PDT3 = $(EXE_PATH)sch_ex.exe
PDT4 = $(EXE_PATH)sf_sch.exe
PUB_H =
LCL_H =
LCL_C = tmr.c sch.c sf_qu.c
C_SRC =  ${PUB_H} ${LCL_H} ${LCL_C}
OBJS = tmr.obj,sch.obj,sf_qu.obj
#
GF = $(LIBS_PATH)gf
PORTF = $(LIBS_PATH)osiportf
SYSF = $(LIBS_PATH)sysf
LIBS = $(GF)/lib,$(PORTF)/lib,$(SYSF)/lib
LIBS_DEP = $(GF).olb,$(PORTF).olb,$(SYSF).olb

$(PDT)  : $(PDT1) $(PDT2) $(PDT3) $(PDT4)

#
$(PDT1) :       tmr_ex.obj tmr.obj sch.obj sf_qu.obj $(LIBS_DEP)
	$(LINK) $(LINKFLAGS) tmr_ex.obj,tmr.obj,sch.obj,sf_qu.obj,$(LIBS)
#
$(PDT2) :       sf_quex.obj sf_qu.obj $(LIBS_DEP)
	$(LINK) $(LINKFLAGS) sf_quex.obj,sf_qu.obj,$(LIBS)
#
$(PDT3) :       sch_ex.obj sch.obj sf_qu.obj  $(LIBS_DEP)
	$(LINK) $(LINKFLAGS) sch_ex.obj,sch.obj,sf_qu.obj,$(LIBS)
#
$(PDT4) :       sf_sch.obj sch_ex.obj sf_qu.obj $(LIBS_DEP)
	$(LINK) $(LINKFLAGS) sf_sch.obj,sch_ex.obj,sf_qu.obj,$(LIBS)
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

