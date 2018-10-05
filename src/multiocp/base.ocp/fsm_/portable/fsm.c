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
 * File: fsm.c
 *
 * Description: Finite State Machine initialization, running, etc.
 *
 * Functions:
 *   FSM_init(void)
 *   FSM_createMachine(FSM_UserData *userData)
 *   FSM_runMachine (FSM_Machine *machine, FSM_EventId evtId)
 *   fsm_evtsMatch (FSM_EventId stateEvtPtr, FSM_EventId evtPtr)
 *   FSM_getUserData(FSM_Machine *machine)
 *   FSM_setUserData(FSM_Machine *machine, FSM_UserData *userData)
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: fsm.c,v 1.20 1997/01/16 22:33:14 kamran Exp $";
#endif /*}*/

#include "estd.h"
#include "eh.h"
#include "tm.h"
#include "sf.h"

#include "fsm.h"
#include "local.h"

PUBLIC TM_ModuleCB *FSM_modCB;


/*<
 * Function:    FSM_init
 *
 * Description: Initialize state machine.
 *
 * Arguments:   None.
 *
 * Returns:     None.
 * 
>*/

PUBLIC Int 
FSM_init(void)
{
    static Bool virgin = TRUE;

    if ( !virgin ) {
	return 0;
    }
    virgin = FALSE;

    if (TM_OPEN(FSM_modCB, "FSM_") == NULL) {
	EH_problem("FSM_init:           TM_open FSM_ failed\n");
	return ( FAIL );
    }

    return (SUCCESS);
}


/*<
 * Function:    FSM_createMachine
 *
 * Description: Create a machine
 *
 * Arguments:   User data 
 *
 * Returns:     Pointer to machine if successful, 0 otherwise.
>*/

PUBLIC FSM_Machine *
FSM_createMachine(FSM_UserData *userData)
{
    fsm_Machine *machine;

    if ( (machine = (fsm_Machine *) SF_memGet(sizeof(*machine))) )  {
	/* If any thing else is to be added do it here */
    } else {
	EH_problem("FSM_createMachine: Memory allocation failed\n");
	perror("FSM_createMachine: SF_memGet:");
	return NULL;
    }

    machine->userData  = userData;			/* FSM_setUserData   */

    TM_TRACE((FSM_modCB, FSM_TMGEN, 
	     "FSM_createMachine:  machine=%s\n", TM_prAddr((Ptr) machine)));

    return ( (FSM_Machine *)machine );
}


/*<
 * Function:    FSM_runMachine
 *
 * Description: Run state machine.
 *
 * Arguments:   Pointer finite state machine, event identifier.
 *
 * Returns:     0 if successful, -1 if unsuccessful. (?)
 * 
>*/

PUBLIC Int
FSM_runMachine (FSM_Machine *machine, FSM_EventId evtId)
{
	FSM_Trans *transPtr;
	FSM_State *nextStatePtr = NULL;
	FSM_State *curStatePtr;
	Bool	  foundNext;
	FSM_UserData	  *userData;

#ifndef FAST
	if (machine == NULL) {
	    EH_problem("FSM_runMachine: machine pointer is NULL.\n");
	    return FAIL;
	}
#endif

	if ((curStatePtr = ((fsm_Machine *)machine)->curState) == NULL) {
	    EH_problem("FSM_runMachine: Current state is NULL.\n");
	    return FAIL;
	}

	TM_TRACE((FSM_modCB, FSM_TMGEN, 
	 	 "FSM_runMachine:     machine=%s, evtId=%d\n", 
		  TM_prAddr((Ptr) machine), evtId));

	foundNext   = FALSE;

 	userData = (FSM_UserData *)((fsm_Machine *)machine)->userData;

	for (transPtr = curStatePtr->trans;
	     transPtr->nextStatePtr != (FSM_State *)0;
	     ++transPtr) {

	    TM_TRACE((FSM_modCB, FSM_TMFUNC,
		      "\tTrying event %d...\n", evtId));

	    if ( fsm_evtsMatch(transPtr->evtId, evtId) ) {
	    	TM_TRACE((FSM_modCB, FSM_TMFUNC, "\t\tAccepted.\n"));
		
	    	TM_TRACE((FSM_modCB, FSM_TMFUNC,
		          "\tTrying predicate %s...\n", transPtr->name));

	    	if (transPtr->predicate == NULL) {
	    	    TM_TRACE((FSM_modCB, FSM_TMFUNC, "\t\tNo predicate.\n"));
		    foundNext    = TRUE;
		    nextStatePtr = transPtr->nextStatePtr;
		    break;
		} 

		if ((*transPtr->predicate)(machine, userData, evtId) == TRUE) {
	    	    TM_TRACE((FSM_modCB, FSM_TMFUNC, "\tPredicate %s: Accepted.",
			      transPtr->name));
		    foundNext    = TRUE;
		    nextStatePtr = transPtr->nextStatePtr;
	            break;
		} else {
	    	    TM_TRACE((FSM_modCB, FSM_TMFUNC,
			      "\tPredicate %s: Not accepted, failed\n", transPtr->name));
		}
	    } else {
	        TM_TRACE((FSM_modCB, FSM_TMFUNC, "\t\tNot accepted, event mismatch\n"));
	    }

	} /* for */

	/* Check for the end of list specification. */
	if (  ! transPtr->nextStatePtr ) {
	    TM_TRACE((FSM_modCB, FSM_TMGEN, 
		     "FSM_runMachine:  machine=%s, Next state not found", 
		     TM_prAddr((Ptr) machine)));
	    EH_problem("FSM_runMachine: Next state not found\n");
	    return ( FAIL );
	}

	TM_TRACE((FSM_modCB, FSM_TMFUNC,
		 "FSM_runMachine: transition=%s, nextState=%s\n",
		 transPtr->name, transPtr->nextStatePtr->name));

	if ( foundNext ) {
	    int (*funcPtr)();

	    if (  (funcPtr = curStatePtr->exit) ) {
		(*funcPtr)(machine, 
			   (FSM_UserData *)((fsm_Machine *)machine)->userData, 
			   evtId);
	    }

	    if (  (funcPtr = transPtr->action) ) {
		if ((*funcPtr)(machine, 
			       (FSM_UserData *)((fsm_Machine *)machine)->userData, 
			       evtId) < 0) {

		    EH_problem("FSM_runMachine: transition action function failed\n");
		  /*  Recover here 
	      	    return (SUCCESS); */
		}
	    }

	    ((fsm_Machine *)machine)->curState = nextStatePtr;

	    if (  (funcPtr = nextStatePtr->entry) ) {
		(*funcPtr)(machine, 
			  (FSM_UserData *)((fsm_Machine *)machine)->userData, 
			   evtId);
	    }
	    
	    /* FSM_ Re-Run/Creation of Internal events 
	     * should be done in the entry function
	     */
	} else {
	    TM_TRACE((FSM_modCB, FSM_TMGEN, 
		     "FSM_runMachine:  machine=%s, Next state not found", 
		     TM_prAddr((Ptr) machine)));
	    EH_problem("FSM_runMachine: Next state not found\n");
	    return ( FAIL );
	}
	return ( SUCCESS );
}


/*<
 * Function:    fsm_evtsMatch
 *
 * Description: Compare two events 
 *
 * Arguments:   Two event identifiers.
 *
 * Returns:     0 if successful, -1 if unsuccessful.
 *
 * 
>*/

Bool
fsm_evtsMatch (FSM_EventId stateEvtPtr, FSM_EventId evtPtr)
{
    Bool retVal= FALSE;

	/* check FSM_EvtDefault case */
    if ( ((stateEvtPtr == FSM_EvtDefault) || (stateEvtPtr == evtPtr)) ) {
	retVal = TRUE;
    }

    TM_TRACE((FSM_modCB, FSM_TMEXEC,
	    "fsm_evtsMatch:      State(evt=%d) Event(evt=%d), match=%d\n",
	     stateEvtPtr, evtPtr, retVal));

    return retVal;
}



/*<
 * Function:    FSM_getUserData
 *
 * Description: Get user data address 
 *
 * Arguments:   Machine 
 *
 * Returns:     Address of user data if successful, otherwise NULL.
 * 
>*/

FSM_UserData *
FSM_getUserData(FSM_Machine *machine)
{
#ifndef FAST
   if (machine == NULL) {
      EH_problem("FSM_getUserData: machine pointer is NULL.\n");
      return (NULL);
   }
#endif
    return ((fsm_Machine *)machine)->userData;
}


/*<
 * Function:    FSM_setUserData
 *
 * Description: Set user data address 
 *
 * Arguments:   Machine and user data address
 *
 * Returns:     
 * 
>*/

Void
FSM_setUserData(FSM_Machine *machine, FSM_UserData *userData)
{
#ifndef FAST
   if (machine == NULL) {
      EH_problem("FSM_setUserData: machine pointer is NULL.\n");
      return;
   }
#endif

    ((fsm_Machine *)machine)->userData = userData;
}
