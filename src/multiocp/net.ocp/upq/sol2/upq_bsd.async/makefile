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
#
#  File name: makefile
#
#  Description: 
#	Service Provider Shell
#
#
# RCS Revision: $Id: makefile,v 1.3 2002/03/22 23:06:07 mohsen Exp $
#
# :::::::::::::

PKG= $(LIBS_PATH)/upq_bsd.a
PUB_H = imq.h sp_shell.h
LCL_H = local.h psq.h usq.h
LCL_C = imq.c psq.c usq.c 
C_SRC =  ${PUB_H} ${LCL_H} ${LCL_C}
OBJS  = imq.o psq.o usq.o 

#
$(PKG): $(OBJS) FORCE
	${LB} ${PKG} ${OBJS}
	${RANLIB} ${PKG}
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

FORCE: 

