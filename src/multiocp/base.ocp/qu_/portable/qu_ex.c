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

#ifdef SCCS_VER	/*{*/
static char sccs[] = "%W%	Released: %G%";
#endif /*}*/

#include "estd.h"
#include "queue.h"
#include "seq.h"

typedef struct SomeInfo {
    struct SomeInfo *next;
    struct SomeInfo *prev;
#define DATASIZE 16	/* No Special Significance */
    Char data[DATASIZE];
    Int len;
} SomeInfo;

typedef struct SomeInfoSeq {
    SomeInfo *first;
    SomeInfo *last;
} SomeInfoSeq;

SEQ_PoolDesc someInfoPool;
SomeInfoSeq someInfoSeq;

Void seqInsert(), seqProcess(), process();


/*<
 * Function:    
 *
 * Description: 
 *
 * Arguments:
 *
 * Returns: 
 *
>*/

main()
{
    static Char *someData = "Some Data";

#define POOLSIZE 22  /* 22 is one of my favorite numbers */
    someInfoPool = SEQ_poolCreate(sizeof(*someInfoSeq.first),
				 POOLSIZE);
    QU_init(&someInfoSeq);

    seqInsert(someData, strlen(someData)+1);
    seqProcess();
}


/*<
 * Function:    seqInsert
 *
 * Description: Insert An Element into someInfoSeq
 *
 * Arguments:
 *
 * Returns: 
 *
>*/

Void seqInsert(Char *data, Int len)
{
    SomeInfo *someInfo;

    someInfo = (SomeInfo *) SEQ_elemObtain(someInfoPool);
    BS_memCopy(data, someInfo->data, len);
    someInfo->len = len;
    QU_insert(&someInfoSeq, someInfo);
}


/*<
 * Function:    
 *
 * Description: 
 *
 * Arguments:
 *
 * Returns: 
 *
>*/

Void seqProcess()
{
    SomeInfo *someInfo;
    Char *p;

    while ((someInfo = someInfoSeq.first) != 
	    (SomeInfo *) &someInfoSeq) {
        QU_remove(someInfo);
	process(&someInfo->data[0], someInfo->len);
	SEQ_elemRelease(someInfoPool, someInfo);
    }
}


/*<
 * Function:    
 *
 * Description: 
 *
 * Arguments:
 *
 * Returns: 
 *
>*/

Void process(Char *data, Int len)
{
    printf("Processing %s\n", data);
}	     
