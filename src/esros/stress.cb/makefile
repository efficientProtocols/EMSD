# 
# Copyright (C) 1997-2002  Neda Communications, Inc.
# 
# This file is part of ESRO. An implementation of RFC-2188.
# 
# ESRO is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License (GPL) as 
# published by the Free Software Foundation; either version 2,
# or (at your option) any later version.
# 
# ESRO is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
# 
# You should have received a copy of the GNU General Public License
# along with ESRO; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.  
# 
#
#  
#  (C) Copyright Neda Communications, Inc 1995. All rights reserved.
# 
#  All Rights Reserved.
# 
#  License for uses of this software with GPL-incompatible software
#  (e.g., proprietary, non Free) can be obtained from Neda Communications, Inc.
#  Visit http://www.neda.com/ for more information.
#
#
#  Description: 
#	Example of ESROS Operations in stress test: Call back API, one process
#
#
# RCS: $Id: makefile,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $
#
# :::::::::::::

CFLAGS= -g 
LIBS_PATH = .
LB= ar r
RANLIB= ranlib
CC=gcc
LK=gcc

PDT = all
PUB_H =
LCL_H =
LCL_C = stress_i.c stress_p.c bindunbi.c ginit.c nm.c 
C_SRC =  ${PUB_H} ${LCL_H} ${LCL_C}
OBJS  = ginit.o nm.o 
C_SRC =  ${PUB_H} ${LCL_H} ${LCL_C}
#
UDP_IF = $(LIBS_PATH)/udp_pco.a
PROT_ENG = $(LIBS_PATH)/esroprot.a
EROP_SH = $(LIBS_PATH)/erop_sh.a
GF = $(LIBS_PATH)/gf.a
SF = $(LIBS_PATH)/sf.a
FSM = $(LIBS_PATH)/fsm.a
RELID = $(LIBS_PATH)/relid.a
LIBS = $(EROP_SH) $(PROT_ENG) $(UDP_IF) $(SF) $(RELID) $(GF) $(FSM)

PDT1 = $(BIN_PATH)/stress_i.cb.one
PDT2 = $(BIN_PATH)/stress_p.cb.one
PDT3 = $(BIN_PATH)/bindunbi.cb.one

$(PDT):	 $(PDT1) $(PDT2) $(PDT3)
	
$(PDT1):       stress_i.o $(OBJS) ${LIBS}
	 ${LK} -o $(PDT1) $(OBJS) stress_i.o ${LIBS} ${SYSLIBS}

$(PDT2):       stress_p.o $(OBJS) ${LIBS}
	 ${LK} -o $(PDT2) $(OBJS) stress_p.o ${LIBS} ${SYSLIBS}

$(PDT3):       bindunbi.o $(OBJS) ${LIBS}
	 ${LK} -o $(PDT3) $(OBJS) bindunbi.o ${LIBS} ${SYSLIBS}
#
#
#SYSLIBS = -lsocket -lnsl -lucb 
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
#
tags:  ${LCL_C}
	ctags $(LCL_C)
#

FORCE: 

