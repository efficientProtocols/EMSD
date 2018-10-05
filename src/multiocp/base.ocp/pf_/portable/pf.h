/*
 * This file is part of the Open C Platform (OCP) Library. OCP is a
 * portable library for development of Data Communication Applications.
 *
 * Copyright (C) 1995 Neda Communications, Inc.
 *	Prepared by Mohsen Banan (mohsen@neda.com)
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.  This library is
 * distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 * License for more details.  You should have received a copy of the GNU
 * Library General Public License along with this library; if not, write
 * to the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139,
 * USA.
 *
*/
/*+
 * File name: pf.h
 *
 * Description: Public Facilities
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 * 
 */

/*
 * RCS Revision: $Id: pf.h,v 1.8 1995/10/04 22:30:36 hshane Exp $
 */

#ifndef _PF_H_	/*{*/
#define _PF_H_

#include "estd.h"

#ifdef LINT_ARGS	/* Arg. checking enabled */

/* strflds.c */
Char *PF_strIgnore(Char  *, Char  *);
Char PF_strNextFld(Char **, Char  *, Char  *);
Int PF_strToFlds(Char *, Char *[], Int , Char  *);
Int PF_strToArgs(Char *, Char *[], Int );

#ifdef NO_STDLIB_H
/* strdup.c */
Char  *strdup(const Char *);

/* bsearch.c */
Char *bsearch(Char *, Char *, unsigned int, unsigned int, int (*)());
#endif /* NO_STDLIB_H */

/* strfind.c */
Char  *PF_strFind(Char  *,Char  *);

/* nlelim.c */
void PF_nlElim(Char []);

/* strmcat.c */
Char  *PF_strmcat(Char  *,Char  *,Char  *);

/* strmcpy.c */
Char  *PF_strmcpy(Char  *,Char  *,Char  *);

/* getint.c */
int PF_getInt(Char  *,int  *,int ,int ,int );

/* getlgint.c */
int PF_getLgInt(Char  *,long  *,long ,long ,long );

/* getuns.c */
int PF_getUns(Char  *,unsigned int  *,unsigned int ,unsigned int ,unsigned int );

/* getlguns.c */
int PF_getLgUns(Char *,unsigned long *,unsigned long,unsigned long, unsigned long );

/* crc16.c */
unsigned short PF_crc16(unsigned char *,int ,unsigned short );

/* strlwr.c */
Char  *PF_strLower(Char  *);

/* strupr.c */
Char  *PF_strUpper(Char  *);

extern PF_charCaseIgnoreCmp(Char, Char);

#else  /* LINT_ARGS undefined */

/* strflds.c */
EXTERN Char  *PF_strIgnore();
EXTERN Char PF_strNextFld();
EXTERN int PF_strToFlds();
EXTERN int PF_strToArgs();

/* strfind.c */
EXTERN Char  *PF_strFind();

/* strdup.c */
EXTERN Char  *strdup();

/* nlelim.c */
EXTERN void PF_nlElim();

/* strmcat.c */
EXTERN Char  *PF_strmcat();

/* strmcpy.c */
EXTERN Char  *PF_strmcpy();

/* getint.c */
EXTERN int PF_getInt();

/* getlgint.c */
EXTERN int PF_getLgInt();

/* getuns.c */
EXTERN int PF_getUns();

/* getlguns.c */
EXTERN int PF_getLgUns();

/* crc16.c */
EXTERN unsigned short PF_crc16();

/* strlwr.c */
EXTERN Char  *PF_strLower();

/* strupr.c */
EXTERN Char  *PF_strUpper();

/* bsearch.c */
EXTERN Void *bsearch();

extern PF_charCaseIgnoreCmp();

#endif /* LINT_ARGS */

#endif	/*}*/
