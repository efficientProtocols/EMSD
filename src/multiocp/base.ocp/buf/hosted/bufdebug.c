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


#include "estd.h"
#include "queue.h"
#include "buf.h"
#include "buflocal.h"
#include "strfunc.h"

/*
 * Function:	BUF_dump()
 *
 * Description:	Display, on the TM output file (or STDOUT), the entire 
 *		contents of the buffer.
 *
 * Arguments:	Buffer, message.
 *
 * Returns:	None.
 *
 */
void
BUF_dump(void * hBuf, char * pMsg)
{
    Buf_Head *      pBuf = (Buf_Head *) hBuf;
    Buf_Segment *   pSeg;
    OS_Uint32	    length;
#ifdef TM_ENABLED
    extern FILE *		tmFile;
    FILE *			hOutput = tmFile;
#elif ! defined(OS_TYPE_MSDOS) || ! defined(OS_VARIANT_Windows)
    FILE *			hOutput = stdout;
#else
    FILE *			hOutput = NULL;
#endif

    fputs(pMsg, hOutput);

    for (pSeg = QU_FIRST(pBuf), length = 0;
	 ! QU_EQUAL(pSeg, pBuf);
	 pSeg = QU_NEXT(pSeg))
    {
	length += pSeg->pEnd - pSeg->pStart;
	STR_dump(pSeg->pStart, pSeg->pEnd - pSeg->pStart, 0);
    }

    if (hOutput != NULL)
    {
	fprintf(hOutput, "\tLength=%lu (0x%lx)\n\n", length, length);
    }
}

