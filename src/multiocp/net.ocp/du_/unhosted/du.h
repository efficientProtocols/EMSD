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
 * File name: du.h
 *
 * Description: "Data Unit" data structures and functions.
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 * 
 */

/*
 * RCS Revision: $Id: du.h,v 1.5 1996/08/29 17:35:59 kamran Exp $
 */

#ifndef _DU_H_	/*{*/
#define _DU_H_

#include "estd.h"

typedef struct DU_Pool {
    struct DU_BufInfo *bufFreeList;	/* free list of buffers */
    Int size;			/* size of each buffer in pool */
    Int count;			/* number of free buffers */
} DU_Pool;

/*
 *  Global buffer header.
 *  Each memory buffer is preceeded by this structure in memory.
 */

typedef struct DU_BufInfo {
    union {
	struct DU_BufInfo *next; /* next free buffer, when in free list */
	struct DU_Pool *pool;	 /* buffer pool pointer, when in use */
    } un;
    Byte *end;		/* address of last byte of buffer */
    MdInt refcnt;	/* number of links to this buffer */
} DU_BufInfo;


/*
 *  The buffer structure itself.
 *  All layers recognize this structure as the actual buffer itself.
 *  Each user of the buffer can have a private copy of this header.
 */


typedef struct DU_Elem {
    struct DU_Elem *next;	/* Next buffer in chain */
    struct DU_Elem *prev;	/* Previous buffer in chain */
    struct DU_BufInfo *bufInfo;	/* Actual data buffer structure */
    unsigned char *addr;	/* Pointer to start of buffer data */
    /* User Specific Data */
    union {
#define DU_MAXUSERINFO 48
	/* Be very careful about DU_MAXUSERINFO.
	 * it MUST be the largest data size used by any 
	 * DU_ module user. 
	 * Its definition really belongs to target.h
	 */
	Byte d[DU_MAXUSERINFO];
    } uInfo;
} DU_Elem;

/* This is the Data type that everything operates on */
typedef DU_Elem *DU_View;

/*
 *  Simple operations on a stream.
 */

#define DU_prepend(sp, n)	((sp)->addr -= (n))
#define DU_adjust(sp, n)	((sp)->bufInfo->end -= (n))
#define DU_strip(sp, n)		((sp)->addr += (n))
#define DU_data(sp)		((sp)->addr)
#define DU_size(sp)		((sp) ? (sp)->bufInfo->end - (sp)->addr : 0)
#define DU_next(sp)		((sp)->next)
#define DU_maxSize(v)		((v)->bufInfo->un.pool->size)
#define DU_allocMax(dp)		DU_alloc((dp), (dp)->size)

#ifdef LINT_ARGS	/* Arg. checking enabled */

/* du.c */
struct DU_Elem  *DU_get(struct DU_Pool  *,int );
struct DU_Elem  *DU_alloc(struct DU_Pool  *,int );
Void DU_free(struct DU_Elem  *);
SuccFail DU_freeLast(struct DU_Elem  *);
struct DU_Elem  *DU_link(struct DU_Elem  *);
struct DU_Pool  *DU_buildPool(int ,int ,int );
void DU_check(DU_Pool *, char *);

#else

extern DU_View DU_get();
extern DU_View DU_alloc();
extern DU_View DU_link();
extern Void DU_free();
extern DU_Pool * DU_buildPool();
extern void DU_check();

#endif /* LINT_ARGS */

#endif	/*}*/
