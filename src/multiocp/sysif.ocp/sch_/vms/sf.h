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
 * Description:
 *
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 * 
 */

/*
 * SCCS Revision: @(#)target.h	1.7    Released: 2/4/88
 */

#ifndef _SF_H_	/*{*/
#define _SF_H_

#include "oe.h"
#include "malloc.h"


/* 
 * Memory Allocation Mechanism.
 */
#define SF_memGet   malloc	/* For back wards compatibility */
#define SF_memObtain malloc 
#define SF_memRelease free 

/*
 * Critical Section Protection.
 */
typedef Int SF_Status;	    
#define SF_critBegin() 1
#define SF_critEnd(status) status = status

#define SF_block() ;

#ifdef LINT_ARGS	/* Arg. checking enabled */

Void SF_quInit(Ptr);
SuccFail SF_quInsert(Ptr, Ptr);
SuccFail SF_quRemove(Ptr, Ptr *);

#else

Void SF_quInit(Ptr);
SuccFail SF_quInsert(Ptr, Ptr);
SuccFail SF_quRemove(Ptr, Ptr *);

#endif /* LINT_ARGS */

#endif	/*}*/
