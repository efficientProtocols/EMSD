#  This file is part of the Open C Platform (OCP) Library. OCP is a
#  portable library for development of Data Communication Applications.
# 
#  Copyright (C) 1995 Neda Communications, Inc.
# 	Prepared by Mohsen Banan (mohsen@neda.com)
# 
#  This library is free software; you can redistribute it and/or modify
#  it under the terms of the GNU Library General Public License as
#  published by the Free Software Foundation; either version 2 of the
#  License, or (at your option) any later version.  This library is
#  distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or
#  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
#  License for more details.  You should have received a copy of the GNU
#  Library General Public License along with this library; if not, write
#  to the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139,
#  USA.
# 
#
#
#  Description: 
#	LOCAL TRANSPORT SCENARIO INTERPRETER
#
#
# SCCS Revision: @(#)makefile	1.1    Released: 1/14/88
#
# :::::::::::::

PDT = tmr.exe
PUB_H =
LCL_H = 
LCL_C = main.c tmr_start.c tmr_intr.s critical.s
C_SRC =  ${PUB_H} ${LCL_H} ${LCL_C}
OBJS  = main.o tmr_start.o tmr_intr.o critical.o
C_SRC =  ${PUB_H} ${LCL_H} ${LCL_C}
#
SYS = $(LIBS_PATH)/sysf.a
DOSSUP = $(LIBS_PATH)/dossup.a
GF = $(LIBS_PATH)/gf.a
#
LIBS =  $(SYS) $(DOSSUP) $(GF)

#
$(PDT):       $(OBJS) ${LIBS}
	 ${LK} -o ${PDT} ${OBJS} ${LIBS} 

tmr_intr.o:	tmr_intr.s
	$(AS) tmr_intr.s

critical.o:	critical.s
	$(AS) critical.s
#
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

