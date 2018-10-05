/*
 * 
 * Copyright (C) 1997-2002  Neda Communications, Inc.
 * 
 * This file is part of ESRO. An implementation of RFC-2188.
 * 
 * ESRO is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (GPL) as 
 * published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 * 
 * ESRO is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with ESRO; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.  
 * 
 */

/*
 *  
 *  Available Other Licenses -- Usage Of This Software In Non Free Environments:
 * 
 *  License for uses of this software with GPL-incompatible software
 *  (e.g., proprietary, non Free) can be obtained from Neda Communications, Inc.
 *  Visit http://www.neda.com/ for more information.
 * 
 */

/*+
 * File name: clinvtd2.c (Connectionless Invoker Transition Diagram 2-Way HS)
 *
 * Description: Connectionless Invoker Transition Diagram (2-Way handshake)
 *
 * Functions:
 *   erop_2CLInvokerTransDiag (void)
 *   tr_2badEventIgnore(Void *machine, Void *userData, FSM_EventId evtId)
 *
 *   lsfsm_e   Invoked when ENTERING a state.
 *   lsfsm_x   Invoked when EXITING a state.
 *   lsfsm_a   Invoked as ACTION of a transition.
 *   lsfsm_t   TRANSITION specificatin.
 *   lsfsm_s   STATE specification.
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: clinvtd2.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
#endif /*}*/

#include  "estd.h"
#include  "eh.h"
#include  "queue.h"
#include  "eropfsm.h"
#include  "fsm.h"
#include  "fsmtrans.h"

#include "target.h"
#include "local.h"
#include "extfuncs.h"

extern FSM_State lsfsm_s2CLInvokerStart;
extern FSM_State lsfsm_s2InvokePduSent;
extern FSM_State lsfsm_s2InvokerRefWait;

extern Int lsfsm_ePass(Void *machine, Void *userData, FSM_EventId evtId);
extern Int lsfsm_xPass(Void *machine,Void *userData, FSM_EventId evtId);

#ifdef FUTURE
extern FSM_State lsfsm_s2Error;

LOCAL FSM_Trans	lsfsm_tError[]=
{
    {
	FSM_EvtDefault,
	(Bool(*)(FSM_Machine *, FSM_UserData *, FSM_EventId))0,
	(Int(*)())0,
	&FSM_sError
    }
};

PUBLIC FSM_State lsfsm_s2Error = {
	lsfsm_eError,
	(Int(*)()) 0,
	lsfsm_tError,
        "Error-State"};
#endif

STATIC FSM_Trans	lsfsm_tCLInvokerStart[]=
{ 
    {
	lsfsm_EvtInvokeReq,
	(Bool(*)(FSM_Machine *, FSM_UserData *, FSM_EventId))0,
	tr_2clInvoker01,
	&lsfsm_s2InvokePduSent,
	"U:Invoke.Req"
    }, 
    {
	FSM_EvtDefault,
	(Bool(*)(FSM_Machine *, FSM_UserData *, FSM_EventId))0,
	tr_2badEventIgnore,
	&lsfsm_s2CLInvokerStart,
	"I:BadEventIgnored"
    }
};

PUBLIC FSM_State lsfsm_s2CLInvokerStart = {
    lsfsm_ePass,
    lsfsm_xPass,
    lsfsm_tCLInvokerStart,
    "CL2-InvokerStart-State"};

STATIC FSM_Trans	lsfsm_tInvokePduSent[]=
{ 
    {
	lsfsm_EvtInvokePduRetranTimer,
	(Bool(*)(FSM_Machine *, FSM_UserData *, FSM_EventId))0,
	tr_2clInvoker02,
	&lsfsm_s2InvokePduSent,
	"T:RetranInvokePdu"
	},
    {
	lsfsm_EvtLastTimer,
	(Bool(*)(FSM_Machine *, FSM_UserData *, FSM_EventId))0,
	tr_2clInvoker03,
	&lsfsm_s2InvokerRefWait,
	"T:LastRetran"
    },
    {
	lsfsm_EvtPduResult,
	(Bool(*)(FSM_Machine *, FSM_UserData *, FSM_EventId))0,
	tr_2clInvoker04,
	&lsfsm_s2InvokerRefWait,
	"P:ResultPdu"
    },
    {
	lsfsm_EvtPduFailure,
	(Bool(*)(FSM_Machine *, FSM_UserData *, FSM_EventId))0,
	tr_2clInvoker05,
	&lsfsm_s2InvokerRefWait,
	"P:FailurePdu"
    },
    {
	FSM_EvtDefault,
	(Bool(*)(FSM_Machine *, FSM_UserData *, FSM_EventId))0,
	tr_2badEventIgnore,
	&lsfsm_s2InvokePduSent,
	"I:BadEventIgnored"
    }
};

PUBLIC FSM_State lsfsm_s2InvokePduSent = {
    lsfsm_ePass,
    lsfsm_xPass,
    lsfsm_tInvokePduSent,
    "CL2-InvokePduSent-State"};

STATIC FSM_Trans	lsfsm_tInvokerRefWait[]=
{ 
    {
	lsfsm_EvtRefNuTimer,
	(Bool(*)(FSM_Machine *, FSM_UserData *, FSM_EventId))0,
	tr_2clInvoker07,
	&lsfsm_s2CLInvokerStart,
	"T:InvokerRefWait"
    },
    {
	lsfsm_EvtPduResult,
	(Bool(*)(FSM_Machine *, FSM_UserData *, FSM_EventId))0,
	tr_2clInvoker06,
	&lsfsm_s2InvokerRefWait,
	"P:DuplicateResultPdu"
    },
    {
	FSM_EvtDefault,
	(Bool(*)(FSM_Machine *, FSM_UserData *, FSM_EventId))0,
	tr_2badEventIgnore,
	&lsfsm_s2InvokerRefWait,
	"I:BadEventIgnored"
    }
};


PUBLIC FSM_State lsfsm_s2InvokerRefWait = {
    lsfsm_ePass,
    lsfsm_xPass,
    lsfsm_tInvokerRefWait,
    "CL2-InvokerRefWait-State"};


/*<
 * Function:    erop_2CLInvokerTransDiag
 *
 * Description: Protcol Engine: Create Connectionless Invoker Transition 
 *              Diagram.
 *
 * Arguments:   None.
 *
 * Returns:     Pointer to state machine diagram if successful, otherwise 0.
 *
 * 
>*/

PUBLIC FSM_TransDiagram *
erop_2CLInvokerTransDiag (void)
{
    static FSM_TransDiagram * app;
    static Bool virgin = TRUE;

    if (!virgin) {
        return app;
    }

    virgin = FALSE;

    if ( ! (app = (FSM_TransDiagram*)FSM_TRANSDIAG_create("2CLInvokerTransDiag", &lsfsm_s2CLInvokerStart)) ) {
	EH_problem("erop_2CLInvokerTransDiag: FSM_TRANSDIAG_create failed");
	app = (FSM_TransDiagram *) 0;
    }
    return app;
}


/*<
 * Function:    tr_2badEventIgnore
 *
 * Description: Bad event is ignored.
 *
 * Arguments:   State machine, event id.
 *
 * Returns:     0 if completed successfule, -1 otherwise.
 * 
>*/

Int
tr_2badEventIgnore(Void *machine, Void *userData, FSM_EventId evtId)
{
    EH_problem("tr_2badEventIgnore: Bad Event Ignored\n");
    return (SUCCESS); 
}

#if 0

/*<
 * Function:    lsfsm_ePass
 *
 * Description: Invoked when entering a state.
 *
 * Arguments:   State machine, event id.
 *
 * Returns:     0 if completed successfule, -1 otherwise.
 * 
>*/

Int
lsfsm_ePass(Void *machine, Void *userData, FSM_EventId evtId)
{
    TM_TRACE((EROP_modCB, TM_ENTER,
	     "lsfsm_ePass:         machine=0x%lx  evtId=0x%x\n", machine, evtId));

    return (SUCCESS); 
}



/*<
 * Function:    lsfsm_xPass
 *
 * Description: Invoked when exiting a state.
 *
 * Arguments:   State machine, event id.
 *
 * Returns:     0 if completed successfule, -1 otherwise.
 * 
>*/

Int
lsfsm_xPass(Void *machine,Void *userData, FSM_EventId evtId)
{
    TM_TRACE((EROP_modCB, TM_ENTER,
	     "lsfsm_xPass:         machine=0x%lx  evtId=0x%x\n", machine, evtId));

    return (SUCCESS); 
}
#endif
