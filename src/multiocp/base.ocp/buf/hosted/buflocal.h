/*
 *  This file is part of the Open C Platform (OCP) Library. OCP is a
 *  portable library for development of Data Communication Applications.
 * 
 *  Copyright (C) 1995 Neda Communications, Inc.
 * 	Prepared by Mohsen Banan (mohsen@neda.com)
 * 
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.  This library is
 *  distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 *  License for more details.  You should have received a copy of the GNU
 *  Library General Public License along with this library; if not, write
 *  to the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139,
 *  USA.
 * 
 */

/*
 * Author: Derrell Lipman
 * History:
 *
 */


#ifndef __BUFLOCAL_H__
#define	__BUFLOCAL_H__

#include "estd.h"
#include "queue.h"
#include "strfunc.h"

/*
 * The minimum size data area in a buffer segment, when automatically allocated
 */
#define	BUF_MIN_DATA_AREA	128

/*
 * Each Buf_Segment contains a single segment of a PDU.  Segments
 * may be chained.
 */
typedef struct Buf_Segment
{
    /* Queue element fields */
    QU_ELEMENT;

    /* Pointer to the buffer of which this segment is a part */
    struct Buf_Head *		pHeader;

    /* Pointer to start of the PDU segment within the data buffer */
    unsigned char * 		pStart;

    /* Pointer to end of the PDU segment within the data buffer */
    unsigned char *		pEnd;

    /* Pointer to the data */
    STR_String			string;
} Buf_Segment;

typedef struct Buf_Head
{
    /* Queue head fields */
    QU_HEAD;

    /* Pointer to the "current" segment */
    Buf_Segment *		pCurrentSeg;

    /* Pointer to our "current" location, for updating or reading */
    unsigned char * 		pX;
} Buf_Head;

#endif /* __BUFLOCAL_H__ */
