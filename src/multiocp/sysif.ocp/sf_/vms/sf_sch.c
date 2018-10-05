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
static char sccs[] = "%W%    Released: %G%";
#endif /*}*/

#include "estd.h"
#include "sf.h"
#include "eh.h"

/* 
 * Scheduler Information.
 */
typedef struct SchInfo {
    struct SchInfo *next;
    struct SchInfo *prev;
    Int (*func)();	/* Function to Call */
    Ptr arg;	
} SchInfo;

typedef struct SchInfoSeq {
    SchInfo *first;
    SchInfo *last;
} SchInfoSeq;

STATIC SchInfoSeq availSchInfo;
STATIC SchInfoSeq activeSchInfo;

STATIC SchInfo *schInfoBuf;	
STATIC SchInfo *schInfoBufEnd;


Void SCH_init(maxSchInfo)
Int maxSchInfo;
{
    SchInfo *schInfo;

    /*
     * Create a Pool
     */
    schInfoBuf = (SchInfo *)
		    SF_memObtain(maxSchInfo * sizeof(*schInfoBuf));
    if (!schInfoBuf) {
	EH_fatal("SF_memObtain");
    }
    schInfoBufEnd = &schInfoBuf[maxSchInfo -1];
    QU_init(&availSchInfo);
    QU_init(&activeSchInfo);
    for (schInfo = schInfoBuf; schInfo <= schInfoBufEnd; ++schInfo){
	SF_quInsert(&availSchInfo, schInfo);	
    }
}

Void SCH_term()
{
    SF_memRelease(schInfoBuf);
}

Void SCH_submit(func, arg)
Int (*func)();
Ptr arg;
{
    SchInfo *schInfo;
    Int status;

    /*
     * Queue Up the function and argument for synchronus processing.
     * Notice The Qu insertion must be protected.
     */
    if ((status = SF_quRemove(&availSchInfo, &schInfo)) != 0) {
	EH_fatal("No SchInfo");
	return ;
    }

    schInfo->func = func;
    schInfo->arg = arg;
    
    if (SF_quInsert(&activeSchInfo, schInfo)) {
	/* PORTATION SPECIFIC
	 * Use environment specific facilities to wake up.
         */
	 sys$wake(0, 0);
    }
}   

Void SCH_block()
{
    /*
     * Should map into environment specific facilities
     * that puts this process into sleep.
     */
    if (activeSchInfo.first == &activeSchInfo) {
	/* PORTATION SPECIFIC
	 * Use Environment Specific facilities to Block
	 */
	sys$hiber();
    }
}

Void SCH_run()
{
    SchInfo *schInfo;
    Int status;

    while (SF_quRemove(&activeSchInfo, &schInfo) == 0) {
	(*schInfo->func)(schInfo->arg);
	SF_quInsert(&availSchInfo, schInfo);
    }
}
