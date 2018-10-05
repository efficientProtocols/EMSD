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

#define TM_ENABLED

#include <stdio.h>
#include "estd.h"
#include "queue.h"
#include "seq.h"
#include "sf.h"
#include "tm.h"

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
main()
{
    static Char *someData = "Some Data";
    static Char *someData2 = "some Data";

#define POOLSIZE 22  /* 22 is one of my favorit numbers */
    someInfoPool = SEQ_poolCreate(sizeof(*someInfoSeq.first), POOLSIZE);
    QU_init(&someInfoSeq);

    seqInsert(someData, strlen(someData)+1);
    seqInsert(someData2, strlen(someData2)+1);
    seqProcess();
}

/*
 * Insert An Element into someInfoSeq
 */
Void seqInsert(data, len)
Char *data;
Int len;
{
    SomeInfo *someInfo;
    Int retVal;

    someInfo = (SomeInfo *) SEQ_elemObtain(someInfoPool);
    BS_memCopy(data, someInfo->data, len);
    someInfo->len = len;
    retVal = SF_quInsert(&someInfoSeq, someInfo);
    printf("%s someInfoSeq.first=%x, someInfo=%x\n", TM_here(), 
	    someInfoSeq.first, someInfo);
    printf("SF_quInsert returned %x\n", retVal);
}

Void seqProcess()
{
    SomeInfo *someInfo;
    Char *p;
    Int retVal; 

    printf("%s someInfoSeq.first=%x, someInfoSeq=%x\n", TM_here(), 
	    someInfoSeq.first, &someInfoSeq);
    
    while ((retVal = SF_quRemove(&someInfoSeq, &someInfo)) == SUCCESS) {
	printf("%s someInfoSeq.first=%x, someInfo=%x\n", TM_here(), 
	    someInfoSeq.first, someInfo);
	printf("SF_quRemove returned %x\n", retVal);
	process(&someInfo->data[0], someInfo->len);
	SEQ_elemRelease(someInfoPool, someInfo);
    }
}

Void process(data, len)
Char *data;
Int len;
{
    printf("Processing %s\n", data);
}	     
