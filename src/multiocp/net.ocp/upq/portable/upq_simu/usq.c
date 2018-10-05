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
 * File name: usq.c
 *
 * Description: User Shell Queue
 *
 * Functions:
 *   USQ_init()
 *   USQ_primQuCreate()
 *   USQ_primQuDelete(USQ_PrimQuInfo *quInfo)
 *   USQ_putAction(USQ_PrimQuInfo *quInfo, Ptr data, Int size)
 *   USQ_getEvent(USQ_PrimQuInfo *quInfo, Ptr data, Int size)
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: usq.c,v 1.11 1996/11/07 02:57:33 kamran Exp $";
#endif /*}*/

#include  <stdio.h>
#include  <string.h>
#ifdef MSDOS
#include  <dos.h>
#endif

#include  "eh.h"
#include  "estd.h"
#include  "tm.h"
#include  "sp_shell.h"
#include  "usq.h"
#include  "imq.h"
#include  "sch.h"
#include  "sf.h"

 
static IMQ_PrimDesc pubQuDesc;

extern char pubQuName[];
extern G_heartBeat();
Int psq_schedPubQu(Int **);
Int psq_schedActionQu();


/*<
 * Function:    USQ_init
 *
 * Description: Initialize User Shell Queue module
 *
 * Arguments:	None
 *
 * Returns:	None
 *
>*/

LOCAL Void 
USQ_init()
{
    IMQ_Key	pubQuKey;

    static Bool virgin = TRUE;

    if ( !virgin ) {
	return;
    }
    virgin = FALSE;

    IMQ_init(IMQ_ELEMENTS);      /* Inititialize the Inter Module Queue */
    
    pubQuKey  = IMQ_keyCreate(pubQuName, 0);
    pubQuDesc = IMQ_primGet(pubQuKey);
}



/*<
 * Function:   	USQ_primQuCreate
 *
 * Description: Create primitive queue
 *
 * Arguments:	None
 *
 * Returns:	Pointer to primitive queue struct
 *
>*/
LOCAL USQ_PrimQuInfo *
USQ_primQuCreate()
{
    USQ_PrimQuInfo *quInfo;
    SP_Action action;
    Int size;
    Char *chnlName;
    IMQ_Key  actionQuKey;
    IMQ_Key  eventQuKey;

#ifdef TM_ENABLED
    char taskNamePub[100] = "Pub Queue (simu)"; 
#endif

    quInfo = (USQ_PrimQuInfo *)SF_memGet(sizeof(*quInfo));
    if (! quInfo) {
	EH_problem("USQ_primQuCreate: SF_memGet failed for queue creation\n");
	return (USQ_PrimQuInfo *)0;
    }

    chnlName = IMQ_nameCreate();
    strcpy(quInfo->chnlName, chnlName);
    chnlName = quInfo->chnlName;

    actionQuKey = IMQ_keyCreate(chnlName, 0);
    quInfo->actionQu = IMQ_primGet(actionQuKey);

    eventQuKey = IMQ_keyCreate(chnlName, 1);
    quInfo->eventQu = IMQ_primGet(eventQuKey);

    action.type = SP_QUADDREQ;
    strcpy(action.un.quAddReq.chnlName, chnlName);

    IMQ_primSnd(pubQuDesc, (IMQ_PrimMsg *) &action, SP_getSize(SP_QUADDREQ));

#ifdef TM_ENABLED
    SCH_submit((Void *) psq_schedPubQu, (Ptr) &pubQuDesc, 
		(SCH_Event) SCH_PSEUDO_EVENT,
		(String) strcat(taskNamePub, TM_here()));
#else
    SCH_submit((Void *) psq_schedPubQu, (Ptr) &pubQuDesc, 
		(SCH_Event) SCH_PSEUDO_EVENT); 
#endif

    G_heartBeat();
    if ((size = IMQ_primRcv(quInfo->eventQu,(IMQ_PrimMsg *) &action, 
				SP_getSize(SP_QUADDCNF))) == -1) { 
	EH_problem("USQ_primQuCreate (upq_simu): Public queue empty!!\n");
    }

    if (action.type != SP_QUADDCNF || action.un.quAddCnf.status != SUCCESS) {
	EH_problem("USQ_primQuCreate");
	free(quInfo);
	return (USQ_PrimQuInfo *)0;
    }

    return ( quInfo );
}


/*<
 * Function:	USQ_primQuDelete
 *
 * Description:	Delete primitive queue
 *
 * Arguments:	Queue pointer
 *
 * Returns:	None
 *
>*/
LOCAL Void
USQ_primQuDelete(USQ_PrimQuInfo *quInfo)
{
    /* Remove Message Qus */
    free(quInfo);
}



/*<
 * Function:	USQ_putAction
 *
 * Description: Put action primitive
 *
 * Arguments:	Queue pointer, data, size of data
 *
 * Returns:	0 on successful completion, -1 otherwise.
 *
>*/
PUBLIC SuccFail
USQ_putAction(USQ_PrimQuInfo *quInfo, Ptr data, Int size)
{
    int retVal;

#ifdef TM_ENABLED
    char taskNameAct[100] = "Action Queue (simu)"; 
#endif

    retVal = IMQ_primSnd(quInfo->actionQu, (IMQ_PrimMsg *)data, size);

#ifdef TM_ENABLED
    	SCH_submit((Void *) psq_schedActionQu, (Ptr) quInfo, 
		   (SCH_Event) SCH_PSEUDO_EVENT,
		   (String) strcat(taskNameAct, TM_here()));
#else
    	SCH_submit((Void *) psq_schedActionQu, (Ptr) quInfo, 
		   (SCH_Event) SCH_PSEUDO_EVENT);
#endif

    G_heartBeat();

    return retVal;
}


/*<
 * Function:	USQ_getEvent
 *
 * Description:	Get event
 *
 * Arguments:	Queue pointer, data, size of data
 *
 * Returns:	0 unsuccessful completion, a negative error number otherwise
 *
 * 
>*/
PUBLIC Int
USQ_getEvent(USQ_PrimQuInfo *quInfo, Ptr data, Int size)
{
    return IMQ_primRcv(quInfo->eventQu, (IMQ_PrimMsg *)data, size);
}
