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
# RCS: $Id: makefile,v 1.2 2002/03/22 23:06:14 mohsen Exp $
#
# :::::::::::::
#
#  File name: Makefile
#
#  Description: Makefile for a portion of the Open C Platform (OCP)
#

PUB_H= dir.h
LCL_H= 
LCL_C= dir_win32.c
OBJS= dir_win32.o

PKG= $(LIBS_PATH)/sf.a

C_SRC= $(PUB_H) $(LCL_H) $(LCL_C)

$(PKG): $(OBJS) FORCE
	${LB} ${PKG} ${OBJS}
	${RANLIB} ${PKG}

tester:


FORCE: 

