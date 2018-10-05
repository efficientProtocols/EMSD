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
 * File name: seq.h
 *
 * Description: Sequence 
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 * 
 */

/*
 * RCS Revision: $Id: seq.h,v 1.3 1995/08/11 04:37:32 mohsen Exp $
 */

#ifndef _SEQ_H_	/*{*/
#define _SEQ_H_

#include "estd.h"

typedef Ptr SEQ_PoolDesc;  

#ifdef LINT_ARGS /*{*/

char  *SEQ_poolCreate(int ,int );
void  SEQ_poolFree(char  *);
char  *SEQ_elemObtain(char  *);
#if 0
The second argument should be a void *
I dont want to be casting it all the time
void SEQ_elemRelease(char  *,char  *);
#endif
void SEQ_elemRelease();

#else /*}{*/

char  *SEQ_poolCreate();
void  SEQ_poolFree();
char  *SEQ_elemObtain();
void  SEQ_elemRelease();

#endif /*}*/

#endif	/*}*/
