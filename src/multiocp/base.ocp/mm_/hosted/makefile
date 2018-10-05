#
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
# RCS: $Id: makefile,v 1.4 2002/03/22 23:05:47 mohsen Exp $
#
# :::::::::::::
#
#  File name: Makefile
#
#  Description: Makefile of OCP's Global Facilities library
#

CFLAGS= -g -Wall -I. -I ../include
LIBS_PATH = .
LB= ar r
#RANLIB= ranlib
RANLIB= echo
CC=gcc

PKG= $(LIBS_PATH)/mm.a
PUB_H= 
LCL_H= 
C_1= entity.c agent.c asncomp.c asninit.c remopmgr.c manager.c
LCL_C= $(C_1) $(C_2)
C_SRC=  $(PUB_H) $(LCL_H) $(LCL_C)
#
O_1= entity.o agent.o asncomp.o asninit.o remopmgr.o manager.o
OBJS= $(O_1) $(O_2)

#
$(PKG): $(OBJS) FORCE
	${LB} ${PKG} ${OBJS}
	${RANLIB} ${PKG}

all:	$(PKG) test_a test_m

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
e_pkg:
	@echo ${PKG}
#
ctags:  ${LCL_C}
	ctags $(LCL_C)

LIBS= $(PKG) $(LIBS_PATH)/asn.a $(LIBS_PATH)/sf.a $(LIBS_PATH)/gf.a $(SYSLIBS)

test_a:	test_a.o $(PKG)
	$(CC) -o test_a test_a.o $(LIBS)

test_m:	test_m.o $(PKG)
	$(CC) -o test_m test_m.o $(LIBS)


FORCE: 

