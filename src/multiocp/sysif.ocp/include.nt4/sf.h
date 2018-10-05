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
 * File name: sf.h
 *
 * Description: System Specific Facilities
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 * 
 */

/*
 * RCS Revision: $Id: sf.h,v 1.1.1.1 1999/09/19 21:48:44 mohsen Exp $
 */

#ifndef _SF_H_	/*{*/
#define _SF_H_

#include <stdlib.h>
#include <malloc.h>

#ifdef MSDOS
#include <dos.h>
#endif  /* MSDOS */

#include "estd.h"
#include "oe.h"
#include "queue.h"


/* 
 * Memory Allocation Mechanism.
 */
#ifdef OS_MALLOC_DEBUG

void OS_allocDebugInit(char *debugFileName);

#define SF_memGet(numOctets)	(OS_allocDebug(numOctets, __FILE__, __LINE__))
#define SF_memObtain(numOctets)	(OS_allocDebug(numOctets, __FILE__, __LINE__))
#define SF_memRelease(mem)	(OS_freeDebug(mem, 	  __FILE__, __LINE__))

#else
#define SF_memGet   malloc	/* For back wards compatibility */
#define SF_memObtain malloc 
#define SF_memRelease free 
#endif

/*
 * Critical Section Protection.
 */
typedef Int SF_Status;	    
#define SF_critBegin() 1
#define SF_critEnd(status) status = status

Void SF_quInit();	/* When Void * becomes real, use real prototypes */

SuccFail SF_quInsert();
SuccFail SF_quRemove();
SuccFail SF_quMove();

#define SF_QUINSERT(head, elem)					\
	SF_quInsert((QU_Head *) head, (QU_Element *) elem)

#define SF_QUREMOVE(head, elemPtr)				\
	SF_quRemove((QU_Head *) head, (QU_Element **) elemPtr)

#define SF_QUMOVE(head, elem)					\
	SF_quMove((QU_Head *) head, (QU_Element *) elem)

#endif	/*}*/
