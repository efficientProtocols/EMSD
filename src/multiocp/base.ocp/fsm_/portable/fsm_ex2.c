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
 * File name: fsm_ex2.c  (FSM Example)
 *
 * Description: (Example program for FSM module)
 *
 *   In this example, the following state machine follows the following path:
 *      Idle -> S1 -> S2 -> Idle -> S1 -> S3 -> Idle
 *
 *   Selection of either S1->S2 or S1->S3 when in state 1, is based on an
 *   internal event generated in transition from Idle to S1.
 *
 *                   ------
 *            ----->/ Idle \<-----
 *           /      \______/      \
 *          /        /\  |         \
 *         /         |   |          \
 *        /          |   |           \
 *       /           |   \/           \
 *   -------         -------        -------
 *  /State 2\<----- /State 1\----->/State 3\
 *  \_______/       \_______/      \_______/
 *
 *
 * Functions:
 *   fsmex_transDiag (void)
 *   fsm_eError (Void *machine, Void *userData, FSM_EventId evtId)
 *   tr_badEventIgnore(Void *machine, Void *userData, FSM_EventId evtId)
 *
 *   fsm_ePass(Void *machine, Void *userData, FSM_EventId evtId)
 *   fsm_xPass(Void *machine, Void *userData, FSM_EventId evtId)
 *   Int tr_idleTo1(Void *machine, Void *userData, FSM_EventId)
 *   Int tr_1ToIdle(Void *machine, Void *userData, FSM_EventId)
 *   Int tr_1To2   (Void *machine, Void *userData, FSM_EventId)
 *   Int tr_2ToIdle(Void *machine, Void *userData, FSM_EventId)
 *   Int tr_1To3   (Void *machine, Void *userData, FSM_EventId)
 *   Int tr_3ToIdle(Void *machine, Void *userData, FSM_EventId)
 *
 *   fsm_e   Invoked when ENTERING a state.
 *   fsm_x   Invoked when EXITING a state.
 *   fsm_a   Invoked as ACTION of a transition.
 *   fsm_t   TRANSITION specificatin.
 *   fsm_s   STATE specification.
 *   G_heartBeat(void)
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: fsm_ex2.c,v 1.5 1996/12/06 21:40:45 kamran Exp $";
#endif /*}*/

#include  "estd.h"
#include  "eh.h"
#include  "queue.h"
#include  "sch.h"

#include  "fsm.h"
#include  "fsmtrans.h"
#include  "fsm_ex2.h"
#include  "extfuncs.h"

extern Int      G_heartBeat(void);

Void
main(int argc, char **argv)
{
    int c;
    extern char *optarg;
    extern int optind;

    Void *transDiag;
    Void *machine;

    typedef struct UserData {
	char name[20];
    } UserData;

    UserData userData = {"Example program"};

    TM_INIT();

    while ((c = getopt(argc, argv, "T:t:")) != EOF) {
	switch ( c ) {

#ifdef TM_ENABLED
	case 'T':
	    TM_setUp(optarg);
	    break;
#endif
	}
    }

    SCH_init(100);

    FSM_init();

    FSM_TRANSDIAG_init();

    if ( ! (transDiag = FSM_TRANSDIAG_create("fsmExTransDiag", &fsmex_sIdle)) ) {
	EH_problem("main: FSM_transDiag_create failed");
	exit(13);
    }

    machine = FSM_createMachine(&userData);

    FSM_TRANSDIAG_load(machine, transDiag);

    FSM_TRANSDIAG_resetMachine(machine);


    {
  	int i;
	static FSM_EventId event[5] = {
	    fsmex_EvtIdleTo1,		
	    fsmex_Evt2ToIdle,
	    fsmex_EvtIdleTo1,		
	    fsmex_Evt3ToIdle,
	    -1
	/* fsmex_Evt1ToIdle, */
	}; 
 	
	for (i = 0; event[i] != -1; i++) {
    	    FSM_runMachine(machine, event[i]);

	    if (G_heartBeat()) {
    	        exit(13);
	    }
	}

        FSM_deleteMachine(machine);
	if (G_heartBeat()) {
    	    exit(13);
	}
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

Int 
tr_idleTo1(Void *machine, Void *userData, FSM_EventId eventId)
{
    static Bool firstTime = 1;

    TM_TRACE((FSM_modCB, FSM_TMFUNC, "tr_idleTo1:      Transition: %s\n", 
             FSM_getUserData(machine)));

    if (firstTime) {
	firstTime = 0;
	FSM_generateEvent(machine, fsmex_Evt1To2);
    } else {
	FSM_generateEvent(machine, fsmex_Evt1To3);
    }

    return 0;
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

Int 
tr_1ToIdle(Void *machine, Void *userData, FSM_EventId eventId)
{
    TM_TRACE((FSM_modCB, FSM_TMFUNC, "tr_1ToIdle:         Transition: %s\n", 
             FSM_getUserData(machine)));
    return 0;
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

Int 
tr_1To2(Void *machine, Void *userData, FSM_EventId eventId)
{
    TM_TRACE((FSM_modCB, FSM_TMFUNC, "tr_1To2:         Transition: %s\n", 
             FSM_getUserData(machine)));
    return 0;
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

Int 
tr_2ToIdle(Void *machine, Void *userData, FSM_EventId eventId)
{
    TM_TRACE((FSM_modCB, FSM_TMFUNC, "tr_2ToIdle:      Transition: %s\n", 
             FSM_getUserData(machine)));
    return 0;
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

Int 
tr_1To3(Void *machine, Void *userData, FSM_EventId eventId)
{
    TM_TRACE((FSM_modCB, FSM_TMFUNC, "tr_1To3:         Transition: %s\n", 
             FSM_getUserData(machine)));
    return 0;
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

Int 
tr_3ToIdle(Void *machine, Void *userData, FSM_EventId eventId)
{
    TM_TRACE((FSM_modCB, FSM_TMFUNC, "tr_3ToIdle:      Transition: %s\n", 
             FSM_getUserData(machine)));
    return 0;
}


/*<
 * Function:    fsm_eError
 *
 * Description: Error function
 *
 * Arguments:   State machine, event id.
 *
 * Returns:     0 if completed successfule, -1 otherwise.
 * 
>*/
Int
fsm_eError (Void *machine, Void *userData, FSM_EventId evtId)
{
    EH_problem("fsm_eError: FSM \"error\" fucntion is called");
    return (SUCCESS); 
}


/*<
 * Function:    tr_badEventIgnore
 *
 * Description: Bad event is ignored.
 *
 * Arguments:   State machine, event id.
 *
 * Returns:     0 if completed successfule, -1 otherwise.
 * 
>*/

Int
tr_badEventIgnore(Void *machine, Void *userData, FSM_EventId evtId)
{
    EH_problem("tr_badEventIgnore: Bad Event Ignored\n");
    return (SUCCESS); 
}


/*<
 * Function:    fsm_ePass
 *
 * Description: Invoked when entering a state.
 *
 * Arguments:   State machine, event id.
 *
 * Returns:     0 if completed successfule, -1 otherwise.
 * 
>*/

Int
fsm_ePass(Void *machine, Void *userData, FSM_EventId evtId)
{
    TM_TRACE((FSM_modCB, FSM_TMFUNC,
	"fsm_ePass:       machine=0x%lx  evtId=0x%x\n", machine, evtId));

    return (SUCCESS); 
}



/*<
 * Function:    fsm_xPass
 *
 * Description: Invoked when exiting a state.
 *
 * Arguments:   State machine, event id.
 *
 * Returns:     0 if completed successfule, -1 otherwise.
 * 
>*/

Int
fsm_xPass(Void *machine, Void *userData, FSM_EventId evtId)
{
    TM_TRACE((FSM_modCB, FSM_TMFUNC,
	"fsm_xPass:       machine=0x%lx  evtId=0x%x\n", machine, evtId));

    return (SUCCESS); 
}


/*<
 * Function:    G_heartBeat
 *
 * Description: Heart Beat for the stack.
 *
 * Arguments:   None.
 *
 * Returns:     0 on successful completion, -1 on unsuccessful completion.
 *
>*/

SuccFail
G_heartBeat(void)
{
    if (SCH_block() < 0) {
        EH_fatal("main: SCH_block returned negative value");
	return (FAIL);
    }

    SCH_run();

    return (SUCCESS);
}

