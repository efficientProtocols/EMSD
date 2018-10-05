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

/*+
 * Description:
 *   Out-Stream Module (OUTSTRM_)
 *   Primarily designed for use by EH_, LOG_, and TM_ modules.
 *
 *   Output devices that are currently supported are:
 *        Files
 *        Network
 *        Com Ports
 *        Console
 *
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: outstrm.c,v 1.1.1.1 1998/02/01 00:51:49 mohsen Exp $";
#endif /*}*/

/* #includes */
#include  "estd.h"
#include  "oe.h"
#include  "tm.h"
#include  "outstrm.h"
#include  "eh.h"
#include  "queue.h"
#include  "seq.h"
#include  "pf.h"
 

typedef struct outstrm_ModInfo {
    struct outstrm_ModInfo *next;
    struct outstrm_ModInfo *prev;
    Char outstrmName[16];
    OUTSTRM_Type  outstrmType;
    Int openCount;		/* close only when this goes to 0 */
    FILE *outstrmFile;
} outstrm_ModInfo;

typedef struct OutstrmInfoSeq {
    QU_HEAD;
} OutstrmInfoSeq;

/* There has been a OUTSTRM_open */
STATIC SEQ_PoolDesc activePool;
STATIC OutstrmInfoSeq activeSeq;		

STATIC SEQ_PoolDesc setUpPool;
STATIC OutstrmInfoSeq setUpSeq;	   


/*<
 * Function:
 * Description:
 *
 * Arguments:
 *
 * Returns:
 *
 * 
>*/
PUBLIC Void
OUTSTRM_init (void)
{
    static Bool virgin = TRUE;

    if ( virgin ) {
	virgin = FALSE;
	activePool = SEQ_poolCreate(sizeof(outstrm_ModInfo), 0);
	setUpPool = SEQ_poolCreate(sizeof(outstrm_ModInfo), 0);
	QU_INIT(&activeSeq);
	QU_INIT(&setUpSeq);

    }
}

#if 0
QU_Head *
OUTSTRM_getHead(void)
{
    return ((QU_Head *)&activeSeq);
}
#endif



/*<
 * Function:
 * Description:
 *
 * Arguments:
 *
 * Returns:
 *
 * 
>*/
PUBLIC OUTSTRM_ModDesc
OUTSTRM_open(Char *outstrmName, OUTSTRM_Type outstrmType)
{
    outstrm_ModInfo *quIndex;
    outstrm_ModInfo *newOutstrm;

    newOutstrm = (outstrm_ModInfo *)0;

    if ( ! (strlen(outstrmName) < sizeof(newOutstrm->outstrmName) ) ) {
	EH_problem("Outstrm Name Too Long:");
	EH_problem(outstrmName);
	return (newOutstrm);
    }	

    /* Check to make sure it is not duplicate */	
    for ( quIndex = QU_FIRST(&activeSeq);
	  ! QU_EQUAL(quIndex, &activeSeq);
	  quIndex = QU_NEXT(quIndex)) {
	if ( ! strcmp(quIndex->outstrmName, outstrmName) ) {
	    /* For each outstrm we should really not have more
	     * than one OUTSTRM_open.
	     */
	    /* EH_problem("OUTSTRM_open: Duplicate outstrm"); */
	    quIndex->openCount++;
	    newOutstrm =  (outstrm_ModInfo *) quIndex;
	    break;
	}
    }

    if ( newOutstrm == (outstrm_ModInfo *)0 ) {
	/* So It needs to be allocated and added to the Queue */
	if ( ! (newOutstrm = (outstrm_ModInfo *) SEQ_elemObtain(activePool)) ) {
	    EH_problem("OUTSTRM_open: Not enough memory for new outstrm");
	    return ( (OUTSTRM_ModDesc) 0 );
	}
	(Void) strcpy(newOutstrm->outstrmName, outstrmName);
	QU_INIT(newOutstrm);
	QU_INSERT(newOutstrm, &activeSeq);
    }

    /* Now Let's do the real open */
    switch ( outstrmType ) {
    case OUTSTRM_K_file:
	{
	    FILE *fp;
	    if ( (fp = fopen(newOutstrm->outstrmName, "w")) ) {
		newOutstrm->outstrmFile = fp;
		setbuf(newOutstrm->outstrmFile, (char *)0);
	    } else {
		EH_problem("");
		return ( (OUTSTRM_ModDesc) 0 );
	    }	
	}	
	break;
    case OUTSTRM_K_network:
        /* newOutstrm ... */
	break;
    case OUTSTRM_K_comPort:
        /* newOutstrm ... */
	break;
    case OUTSTRM_K_console:
	/* outstrmFileName is ignored */
	newOutstrm->outstrmFile = stdout;
	setbuf(newOutstrm->outstrmFile, (char *)0);
	break;

    default:
	EH_problem("");
	return ( (outstrm_ModInfo *) 0 );
    }
    return ( (OUTSTRM_ModDesc) newOutstrm );
}

PUBLIC SuccFail
OUTSTRM_puts(OUTSTRM_ModDesc outstrmDesc, String outStr)
{
    outstrm_ModInfo *outstrm = (outstrm_ModInfo *) outstrmDesc;
    fputs (outStr, outstrm->outstrmFile);
    return SUCCESS;
}

PUBLIC SuccFail
OUTSTRM_close(OUTSTRM_ModDesc outstrmDesc)
{
    outstrm_ModInfo *outstrm = (outstrm_ModInfo *) outstrmDesc;
    /* NOTYET */
    return Fail;
}
