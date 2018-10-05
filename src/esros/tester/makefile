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
#  File name: makefile
#
#  Description: ESROS Scenario Interpreter, Function Call API, two process 
#
# RCS Revision: $Id: makefile,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $
#
# :::::::::::::

PKG= $(LIBS_PATH)/esrossi.a
PDT = $(BIN_PATH)/esrossi
PUB_H = esrossi.h
LCL_H = 
YFLAGS = -d
LCL_C =  esros.c esrossi.c lex.l tester.y 
C_SRC =  ${PUB_H} ${LCL_H} ${LCL_C}
OBJS  =  tester.o lex.o esros.o esrossi.o
C_SRC =  ${PUB_H} ${LCL_H} ${LCL_C}
CFLAGS = 
LB     = ar r
RANLIB = echo
LIBS_PATH = .

USER_SH = $(LIBS_PATH)/esro_ush.a 
NMM = $(LIBS_PATH)/nmmus.a
UPQ = $(LIBS_PATH)/upq_bsd.a
GF = $(LIBS_PATH)/gf.a
SF = $(LIBS_PATH)/sf.a
RELID = $(LIBS_PATH)/relid.a

LIBS = $(USER_SH) $(UPQ) $(GF) $(NMM) $(SF) $(RELID) $(GF) 

$(PDT):	  main.o  $(PKG) $(LIBS)
	  $(CC) -o $(PDT) main.o $(PKG) $(LIBS) ${SYSLIBS}
#
#SYSLIBS = -L/usr/ucblib -R/usr/ucblib -lresolv -lsocket -lnsl -lelf -lucb
#

$(PKG): $(OBJS) FORCE
	${LB} ${PKG} ${OBJS}
	${RANLIB} ${PKG}
#
e_pub_h:
	@echo ${PUB_H}
e_cl_h:
	@echo ${LCL_H}
e_lcl_c:
	@echo ${LCL_C}
e_c_src:
	@echo ${C_SRC}
e_objs:
	@echo ${OBJS}
e_pkg:
	@echo ${PKG}
#
ctags:  ${LCL_C}
	ctags $(LCL_C)
#

FORCE: 

