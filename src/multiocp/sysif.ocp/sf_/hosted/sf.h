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
 * RCS Revision: $Id: sf.h,v 1.3 1995/08/11 04:37:35 mohsen Exp $
 */

#ifndef _SF_H_	/*{*/
#define _SF_H_

#include "oe.h"
#include "malloc.h"
#include "queue.h"

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

#define ENTER() ;
#define LEAVE() ;

Void SF_quInit();	/* When Void * becomes real, use real prototypes */

SuccFail SF_quInsert();
SuccFail SF_quRemove();
SuccFail SF_quMove();

#endif	/*}*/
