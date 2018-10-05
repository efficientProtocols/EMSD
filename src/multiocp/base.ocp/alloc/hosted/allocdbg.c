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

#ifdef OS_MALLOC_DEBUG

static FILE *	hMemDebug = NULL;

enum
{
    Flags_PrintEach		= (1 << 0),
    Flags_PrintOutstanding	= (1 << 1)
};

static OS_Uint8	flags = Flags_PrintOutstanding;


typedef struct AllocDebug
{
    QU_ELEMENT;

    OS_Uint32	magic1;
    OS_Uint16	len;
    char	fileAndLine[1024];
    OS_Uint32	magic2;
} AllocDebug;

static QU_Head		allocatedMemory = QU_INITIALIZE(allocatedMemory);


void
OS_allocDebugInit(char * pDebugFileName)
{
    if (pDebugFileName != NULL)
    {
	if ((hMemDebug = fopen(pDebugFileName, "w")) == NULL)
	{
	    fprintf(stderr,
		    "OS_allocDebugInit: could not open file %s\n",
		    pDebugFileName);
	    exit(1);
	}

	setbuf(hMemDebug, NULL);
    }

    QU_INIT(&allocatedMemory);

    /* Set up to call print allocated memory */
    signal(SIGUSR1, (void (*)(int)) OS_allocPrintOutstanding);

    /* Set up to clear the printed list of allocated memory */
    signal(SIGUSR2, (void (*)(int)) OS_allocResetPrint);
}


void *
OS_allocDebug(OS_Uint16 numBytes,
	      char * pFileName,
	      int lineNum)
{
    unsigned char * p;
    AllocDebug *    pDebug;
    size_t	    len;

    len = (numBytes + (4 - (numBytes % 4)) +
	   sizeof(AllocDebug) + sizeof(AllocDebug));

    if ((p = malloc(len)) == NULL)
    {
	return NULL;
    }

    /* Fill all of the allocated memory with a known null quantity */
    OS_memSet(p, 0xa5, len);

    pDebug = (AllocDebug *) p;

    QU_INIT(pDebug);
    QU_INSERT(pDebug, &allocatedMemory);

    pDebug->magic1 = pDebug->magic2 = 0x23422342;
    pDebug->len = numBytes;
    sprintf(pDebug->fileAndLine,
	    "%s(%d): %d", pFileName, lineNum, numBytes);

    pDebug = (AllocDebug *) (p + sizeof(AllocDebug) +
			     numBytes + (4 - (numBytes % 4)));

    pDebug->magic1 = pDebug->magic2 = 0x23422342;
    pDebug->len = numBytes;
    sprintf(pDebug->fileAndLine,
	    "%s(%d): %d", pFileName, lineNum, numBytes);

    p += sizeof(AllocDebug);

    if (hMemDebug != NULL && (flags & Flags_PrintEach))
    {
	fprintf(hMemDebug, "0x%08lx (0x%08lx) + %s(%d) numBytes=%d\n",
		(unsigned long) (p - sizeof(AllocDebug)),
		(unsigned long) p,
		pFileName, lineNum,
		(int) numBytes);
    }
    return p;
}

void *
OS_reallocDebug(void * pMem,
		OS_Uint16 numBytes,
		char * pFileName,
		int lineNum)
{
    unsigned char * p = pMem;
    AllocDebug *    pDebug;
    size_t	    len;
    OS_Uint16	    oldLen;

    pDebug = (AllocDebug *) ((unsigned char *) p - sizeof(AllocDebug));

    QU_REMOVE(pDebug);

    if (pDebug->magic1 != 0x23422342 ||
	pDebug->magic2 != 0x23422342)
    {
	abort();
    }

    /* Save old length */
    oldLen = pDebug->len;

    pDebug = (AllocDebug *) ((unsigned char *) p +
			     pDebug->len + (4 - (pDebug->len % 4)));

    if (pDebug->magic1 != 0x23422342 ||
	pDebug->magic2 != 0x23422342)
    {
	abort();
    }

    p -= sizeof(AllocDebug);

    if (hMemDebug != NULL && (flags & Flags_PrintEach))
    {
	fprintf(hMemDebug, "0x%08lx (0x%08lx) *- %s(%d)\n",
		(unsigned long) p,
		(unsigned long) (p + sizeof(AllocDebug)),
		pFileName, lineNum);
    }
    
    len = (numBytes + (4 - (numBytes % 4)) +
	   sizeof(AllocDebug) + sizeof(AllocDebug));

    if ((p = realloc(p, len)) == NULL)
    {
	if (hMemDebug != NULL && (flags & Flags_PrintEach))
	{
	    fprintf(hMemDebug, "%s(%d): realloc FAILED!\n",
		    pFileName, lineNum);
	}
	return NULL;
    }

    /* Fill all of the new uncopied memory with a known quantity */
    if (numBytes > oldLen)
    {
	OS_memSet(p + sizeof(AllocDebug) + oldLen,
		  0x42,
		  (numBytes - oldLen) + sizeof(AllocDebug));
    }

    pDebug = (AllocDebug *) p;

    QU_INIT(pDebug);
    QU_INSERT(pDebug, &allocatedMemory);

    pDebug->magic1 = pDebug->magic2 = 0x23422342;
    pDebug->len = numBytes;
    sprintf(pDebug->fileAndLine,
	    "%s(%d): %d", pFileName, lineNum, numBytes);

    pDebug = (AllocDebug *) (p + sizeof(AllocDebug) +
			     numBytes + (4 - (numBytes % 4)));

    pDebug->magic1 = pDebug->magic2 = 0x23422342;
    pDebug->len = numBytes;
    sprintf(pDebug->fileAndLine,
	    "%s(%d): %d", pFileName, lineNum, numBytes);

    p += sizeof(AllocDebug);

    if (hMemDebug != NULL && (flags & Flags_PrintEach))
    {
	fprintf(hMemDebug, "0x%08lx (0x%08lx) *+ %s(%d) numBytes=%d\n",
		(unsigned long) (p - sizeof(AllocDebug)),
		(unsigned long) p,
		pFileName, lineNum,
		(int) numBytes);
    }

    return p;
}

void
OS_freeDebug(void * p,
	     char * pFileName,
	     int lineNum)
{
    AllocDebug *    pDebug;

    pDebug = (AllocDebug *) ((unsigned char *) p - sizeof(AllocDebug));

    QU_REMOVE(pDebug);

    if (pDebug->magic1 != 0x23422342 ||
	pDebug->magic2 != 0x23422342)
    {
	abort();
    }

    pDebug = (AllocDebug *) ((unsigned char *) p +
			     pDebug->len + (4 - (pDebug->len % 4)));

    if (pDebug->magic1 != 0x23422342 ||
	pDebug->magic2 != 0x23422342)
    {
	abort();
    }

    p -= sizeof(AllocDebug);

    if (hMemDebug != NULL && (flags & Flags_PrintEach))
    {
	fprintf(hMemDebug, "0x%08lx (0x%08lx) - %s(%d)\n",
		(unsigned long) p,
		(unsigned long) (p + sizeof(AllocDebug)),
		pFileName, lineNum);
    }
    
    pDebug = (AllocDebug *) p;
    OS_memSet(p,
	      0x5a,
	      (pDebug->len +
	       (4 - (pDebug->len % 4)) +
	       sizeof(AllocDebug) + sizeof(AllocDebug)));

    free(p);
}

void
OS_allocPrintOutstanding(void)
{
    AllocDebug *    pDebug;
    void *	    h = hMemDebug;

#if ! defined(OS_TYPE_MSDOS) || ! defined(OS_VARIANT_Dos)
    if (hMemDebug == NULL)
    {
	h = stdout;
    }
#endif

    if (! (flags & Flags_PrintOutstanding))
    {
	return;
    }

    fprintf(h, "\n\nCURRENTLY ALLOCATED MEMORY\n");

    for (pDebug = QU_FIRST(&allocatedMemory);
	 ! QU_EQUAL(pDebug, &allocatedMemory);
	 pDebug = QU_NEXT(pDebug))
    {
	fprintf(h, "\t0x%08lx (0x%08lx) %s\n",
	       (unsigned long) pDebug,
	       (unsigned long) (pDebug + 1),
	       pDebug->fileAndLine);
    }

    fprintf(h, "\n\n");
}

void
OS_allocResetPrint(void)
{
    AllocDebug *    pDebug;

    for (pDebug = QU_FIRST(&allocatedMemory);
	 ! QU_EQUAL(pDebug, &allocatedMemory);
	 pDebug = QU_FIRST(&allocatedMemory))
    {
	/* The memory is still allocated!  Just remove it from the queue. */
	QU_REMOVE(pDebug);
    }			  
}


void
OS_allocSetFlags(OS_Uint8 f)
{
    flags = f;
}

#endif /* OS_MALLOC_DEBUG */
