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
 * SCCS Revision: @(#)fifo.h	1.1    Released: 13 Mar 1990
 */

#ifndef FIFO_H
#define FIFO_H

typedef Ptr FIFO_Element;

typedef struct FIFO_CtrlBlk {
	FIFO_Element *in;		
	FIFO_Element *out;		
	FIFO_Element *begin;
	FIFO_Element *end;
} FIFO_CtrlBlk;


EXTERN SuccFail FIFO_put();
EXTERN FIFO_Element *FIFO_get();
EXTERN Bool FIFO_full();

#endif
