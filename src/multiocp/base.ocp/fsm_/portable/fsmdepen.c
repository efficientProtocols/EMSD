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
 * File: fsmdepen.c
 *
 * Description: Finite State Machine: generate event and delete machine .
 *
 * Functions:
 *   FSM_generateEvent (FSM_Machine *machine, FSM_EventId evtId)
 *   fsm_scheduledRunMachine(fsm_RunMachineArgs *runMachineArgs)
 *   FSM_deleteMachine(FSM_Machine *machine)
 *   fsm_scheduledDeleteMachine(FSM_Machine *machine)
 *
 -*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: fsmdepen.c,v 1.6 1996/07/14 19:45:15 kamran Exp $"
#endif /*}*/

#include "estd.h"
#include "eh.h"
#include "sf.h"
#include "sch.h"

#include "fsm.h"

typedef struct fsm_RunMachineArgs {
    FSM_Machine *machine;
    FSM_EventId eventId;
} fsm_RunMachineArgs;

Void fsm_scheduledRunMachine(fsm_RunMachineArgs *runMachineArgs);
Void fsm_scheduledDeleteMachine(Void *machine);


/*<
 * Function:    FSM_generateEvent
 *
 * Description: Generate internal event.
 *
 * Arguments:   Pointer to machine and event id.
 *
 * Returns:     None.
 * 
>*/

PUBLIC Int 
FSM_generateEvent(FSM_Machine *machine, FSM_EventId evtId)
{
    static Bool virgin = TRUE;
    fsm_RunMachineArgs *runMachineArgs;

#ifdef TM_ENABLED
    static char taskName[100] = "FSM internal event:";
#endif

    if (virgin) {
	virgin = FALSE;
	SCH_init(100);
    }

    if ( (runMachineArgs = (fsm_RunMachineArgs *) SF_memGet(sizeof(*runMachineArgs))) )  {
	/* If any thing else is to be added do it here */
    } else {
	EH_problem("FSM_generateEvent: Memory allocation failed");
	perror("FSM_generateEvent: SF_memGet:");
	return (FAIL);
    }
    runMachineArgs->machine = machine;
    runMachineArgs->eventId = evtId;

#ifdef TM_ENABLED
    strcpy (taskName, "FSM internal event:");
    SCH_submit ((Void *)fsm_scheduledRunMachine, (Ptr) runMachineArgs, 
		(SCH_Event) SCH_PSEUDO_EVENT, 
		(String) strcat(taskName, TM_here()));
#else
    SCH_submit ((Void *)fsm_scheduledRunMachine, (Ptr) runMachineArgs, 
		(SCH_Event) SCH_PSEUDO_EVENT);
#endif

    return (SUCCESS);
}


/*<
 * Function:    fsm_scheduledRunMachine
 *
 * Description: Run the machine which is scheduled to be run
 *
 * Arguments:   Pointer to structure containing machine and event
 *
 * Returns:     None
 *
>*/
Void 
fsm_scheduledRunMachine(fsm_RunMachineArgs *runMachineArgs)
{
    FSM_runMachine(runMachineArgs->machine, runMachineArgs->eventId);

    SF_memRelease(runMachineArgs);
}


/*<
 * Function:    FSM_deleteMachine
 *
 * Description: Delete a machine
 *
 * Arguments:   Machine pointer 
 *
 * Returns:     0 if succesful, otherwise -1.
 *
>*/

PUBLIC SuccFail
FSM_deleteMachine(FSM_Machine *machine)
{
    static Bool virgin = TRUE;

#ifdef TM_ENABLED
    static char taskName[100] = "FSM delete machine, ";
#endif

#ifndef FAST
    if (machine == NULL) {
	return (FAIL);
    }
#endif

    if (virgin) {
	virgin = FALSE;
	SCH_init(100);
    }

#ifdef TM_ENABLED
    TM_TRACE((FSM_modCB, FSM_TMGEN, 
	     "FSM_deleteMachine:  machine=%s, deletion scheduled", 
	     TM_prAddr((Ptr) machine)));
    strcpy (taskName, "FSM delete machine, ");
    SCH_submit ((Void *)fsm_scheduledDeleteMachine, (Ptr) machine, 
		(SCH_Event) SCH_PSEUDO_EVENT, 
		(String) strcat(taskName, TM_here()));
#else
    SCH_submit ((Void *)fsm_scheduledDeleteMachine, (Ptr) machine, 
		(SCH_Event) SCH_PSEUDO_EVENT);
#endif

    return (SUCCESS);
}


/*<
 * Function:    fsm_scheduledDeleteMachine
 *
 * Description: Delete the machine which is scheduled to be deleted
 *
 * Arguments:   Machine pointer 
 *
 * Returns:     0 if succesful, otherwise -1.
 *
>*/

Void
fsm_scheduledDeleteMachine(Void *machine)
{
    TM_TRACE((FSM_modCB, FSM_TMFUNC, "FSM_deleteMachine:     machine=%s\n", 
             TM_prAddr((Ptr) machine)));

    SF_memRelease(machine);
}


