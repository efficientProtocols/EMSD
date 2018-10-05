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
# RCS: $Id: makefile,v 1.3 2002/10/25 19:38:03 mohsen Exp $
#
# :::::::::::::
#
#  File name: Makefile
#
#  Description: Makefile for a portion of the Open C Platform (OCP)
#

PUB_H= 
LCL_H= 
LCL_C= tmr.c
OBJS= tmr.o

PKG= $(LIBS_PATH)/ocp.a

C_SRC= $(PUB_H) $(LCL_H) $(LCL_C)


$(PKG): $(OBJS) FORCE
	${LB} ${PKG} ${OBJS}
	${RANLIB} ${PKG}

tester:	tmrtest

tmrtest:	tmrtest.o $(PKG)
	$(LK) -o tmrtest tmrtest.o $(PKG)


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
#
INIT:
	if [ ! -d ${EMSDBASE}/.sb ]; then mkdir ${EMSDBASE}/.sb; fi
	if [ ! -d .sb/. ]; then ln -s ${EMSDBASE}/.sb .sb; fi

FORCE: 

