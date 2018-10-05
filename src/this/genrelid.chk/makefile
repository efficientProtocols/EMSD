#
#  (C) Copyright Neda Communications, Inc 1995. All rights reserved.
# 
#  All Rights Reserved.
# 
#  This software is furnished under a license and use, duplication,
#  disclosure and all other uses are restricted to the rights specified
#  in the written license between the licensee and copyright holders.
#
#  File name: makefile
#
#
# RCS: $Id: makefile,v 1.3 2002/10/25 19:38:06 mohsen Exp $
#
# :::::::::::::
SHELL=/bin/sh
LB=ar r
CFLAGS = 
LIBS_PATH = 
RANLIB = echo


PKG= $(LIBS_PATH)/relid.a
PUB_H= relid.h
LCL_H= 
LCL_C= relid.c compat.c
C_SRC=  $(PUB_H) $(LCL_H) $(LCL_C)
#
OBJS= relid.o compat.o
#
MAN= 

#
$(PKG): $(OBJS) FORCE
	${LB} ${PKG} ${OBJS}
	${RANLIB} ${PKG}
#
GF = $(LIBS_PATH)/gf.a
#SF = $(LIBS_PATH)/sf.a
#LIBS =   $(GF) $(SF) $(GF)
LIBS =   $(GF) 

GENRELID= ${SHELL} ./genrelid.sh -t ./relid.tpl -r ../../relnotes/dos-emsd.tex 

relid_p.c:	relid.tpl
	${GENRELID}

# If everything worked the way it should under NT/Win95
#This is all we needed to do:

#relid.c:		relid_p.c 
#	${CC} ${CFLAGS} -DSHOW_CRC -c relid_p.c
#	${LK} -o relid_p relid_p.o $(LIBS)
#	sed -e "s/XXX_NOTICE_LEN_XXX/`relid_p -l`/" \
#	    -e "s/XXX_NOTICE_CRC_XXX/`relid_p -c`/" \
#	    relid_p.c > sedit.sh
#	-rm relid_p
#	echo "Generated relid.c"

# Because of NT/Win95 it is done as:

relid.c:		relid_p.c CCrelid_p
	${LK} -o relid_p relid_p.o $(LIBS)
	${SHELL} ./sedit.sh relid_p > relid.c
	echo "Generated relid.c"

CCrelid_p:
	${MAKE} CFLAGS="${CFLAGS} -DSHOW_CRC" CC=${CC} relid_p.o

clean:
	-rm relid.c relid_p.c relid_p

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

