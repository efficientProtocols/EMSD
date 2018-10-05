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
#  File name: makefile
#
#  Description: EROP Module (ESROS Engine)
#
# RCS: $Id: makefile,v 1.2 2002/10/25 19:37:32 mohsen Exp $
#
# :::::::::::::
SHELL=/bin/sh
LB=ar r
CFLAGS = 
LIBS_PATH = 
RANLIB = echo


PKG= $(LIBS_PATH)/esroprot.a
PUB_H= 
LCL_H= 
#LCL_C= eropfsm.c  trnsdgrm.c invoke.c layernm.c cpright.c
LCL_C=  invoke.c erop_cfg.c layernm.c  \
	pduin.c pduout.c pdupr.c tmr_if.c \
  	clinvktd.c clperftd.c invokact.c perfact.c erop_sap.c \
  	clinvtd2.c clpertd2.c invact2.c perfact2.c \
	userin.c userout.c keep.c lowerind.c  conffunc.c
C_SRC=  $(PUB_H) $(LCL_H) $(LCL_C)
#
# OBJS= eropfsm.o  trnsdgrm.o invoke.o layernm.o cpright.o
OBJS=   invoke.o erop_cfg.o layernm.o \
	pduin.o pduout.o pdupr.o tmr_if.o \
	clinvktd.o clperftd.o invokact.o perfact.o erop_sap.o \
  	clinvtd2.o clpertd2.o invact2.o perfact2.o \
	userin.o userout.o keep.o lowerind.o  conffunc.o
#
MAN= 

#
$(PKG): $(OBJS) FORCE
	${LB} ${PKG} ${OBJS}
	${RANLIB} ${PKG}
#
#CBASE = nedaemsd.dos
#
#cprtdos.c: cpright.tpl
#	gencpright.sh $(CBASE)/reldos.id cprtdos.c
#
#cprtwin.c: cpright.tpl
#	gencpright.sh ${CBASE}/relwin.id cprtwin.c
#
#cprtdos: 	cprtdos.o
#	${LK} -o cprtdos cprtdos.o $(LIBS_PATH)/gf.a
#
#cprtwin: 	cprtwin.o
#	${LK} -o cprtwin cprtwin.o $(LIBS_PATH)/gf.a

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

