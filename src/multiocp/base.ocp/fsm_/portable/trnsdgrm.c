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
 * File: trnsdgrm.c
 *
 * Description: Create and initialize the transition diagram (of state machine)
 *
 * Functions:
 *   FSM_TRANSDIAG_create (Char *appName)
 *   FSM_TRANSDIAG_init (void)
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: trnsdgrm.c,v 1.8 1996/07/14 19:45:28 kamran Exp $";
#endif /*}*/

#include  "estd.h"
#include  "eh.h"
#include  "sf.h"
#include  "tm.h"
#include  "queue.h"
#include  "fsm.h"
#include  "fsmtrans.h"
#include  "local.h"
 
STATIC QU_Head transDiagQuHead;

PUBLIC TM_ModuleCB *FSM_TRANSDIAG_modCB;
 

/*<
 * Function:    FSM_TRANSDIAG_create
 *
 * Description: Create a transition diagram.
 *
 * Arguments:   Name of the transition diagram.
 *
 * Returns:     Pointer to transition diagram.
 *
 * 
>*/

PUBLIC Void *
FSM_TRANSDIAG_create (String transDiagName, FSM_State *initialState)
{
    FSM_TransDiagram * transDiag;

    if ( (transDiag = (FSM_TransDiagram *) SF_memGet(sizeof(*transDiag))) )  {
	/* If any thing else is to be added do it here */
	transDiag->name  =  transDiagName;
	transDiag->state = initialState;
	QU_INIT(transDiag);
	QU_INSERT(transDiag, &transDiagQuHead);

    } else {
	EH_problem("FSM_TRANSDIAG_create: Memory allocation failed");
	perror("FSM_TRANSDIAG_create: SF_memGet:");
	transDiag = (FSM_TransDiagram *) NULL;
    }
    return (Void *)transDiag;
}


/*<
 * Function:    FSM_TRANSDIAG_init
 *
 * Description: Initialize transition diagrams.
 *
 * Arguments:   None.
 *
 * Returns:     None.
 *
>*/

PUBLIC Void
FSM_TRANSDIAG_init (void)
{
    static Bool virgin = TRUE;

    if ( !virgin ) {
	return;
    }
    virgin = FALSE;

    QU_INIT(&transDiagQuHead);
}



/*<
 * Function:    FSM_TRANSDIAG_load
 *
 * Description: Load transition diagram
 *
 * Arguments:   Machine and transition diagram function
 *
 * Returns:     0 if successful, -1 if unsuccessful.
 *
 * 
>*/

SuccFail
FSM_TRANSDIAG_load (FSM_Machine *machine, FSM_TransDiagram *transDiag)
{
#ifndef FAST
   if (machine == NULL  ||  transDiag == NULL) {
      return (FAIL);
   }
#endif
   ((fsm_Machine *)machine)->transDiag = transDiag;
 
   return (SUCCESS);
} 


/*<
 * Function:    FSM_TRANSDIAG_resetMachine
 *
 * Description: Reset Finite State Machine
 *
 * Arguments:   Pointer to machine
 *
 * Returns:     0 if successful, -1 if unsuccessful.
 * 
>*/

SuccFail
FSM_TRANSDIAG_resetMachine (FSM_Machine *machine)
{
   FSM_TransDiagram *transDiag;

#ifndef FAST
   if (machine == NULL) {
      return (FAIL);
   }
#endif

   transDiag = (FSM_TransDiagram *)((fsm_Machine *)machine)->transDiag;
   ((fsm_Machine *)machine)->curState = transDiag->state;

   return (SUCCESS);
} 

/*<
 * Function:    FSM_deleteTransDiag
 *
 * Description: Delete transition diagram
 *
 * Arguments:   Pointer to transition diagram
 *
 * Returns:     0 if succesful, otherwise -1.
 *
>*/

Void
FSM_deleteTransDiag(Void *transDiag)
{
    TM_TRACE((FSM_modCB, FSM_TMFUNC, 
	     "FSM_deleteTransDiag:     TransDiag=0x%lx\n", transDiag));

    SF_memRelease(transDiag);
}
