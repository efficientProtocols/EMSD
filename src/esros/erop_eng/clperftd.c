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
 * File name: clperftd.c (ConnectionLess Performer Transition Diagram)
 *
 * Description: Connection Less Performer Transition Diagram
 *
 * Functions:
 *  PUBLIC FSM_TransDiagram *
 *  erop_CLPerformerTransDiag (void)
 *
 *  lsfsm_e   Invoked when ENTERING a state.
 *  lsfsm_x   Invoked when EXITING a state.
 *  lsfsm_a   Invoked as ACTION of a transition.
 *  lsfsm_t   TRANSITION specificatin.
 *  lsfsm_s   STATE specification.
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: clperftd.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
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

extern FSM_State FSM_sError;

extern FSM_State lsfsm_sCLPerformerStart;
extern FSM_State lsfsm_sInvokePduReceived;
extern FSM_State lsfsm_sAckPduWait;
extern FSM_State lsfsm_sPerformerRefWait;

extern Int lsfsm_ePass(Void *machine, Void *userData, FSM_EventId evtId);
extern Int lsfsm_xPass(Void *machine,Void *userData, FSM_EventId evtId);

STATIC FSM_Trans	lsfsm_tCLPerformerStart[]=
{ 
    {
	lsfsm_EvtPduInvoke,
	(Bool(*)(FSM_Machine *, FSM_UserData *, FSM_EventId))0,
	tr_clPerformer01,
	&lsfsm_sInvokePduReceived,
	"P:InvokePdu"
    },
    {
	FSM_EvtDefault,
	(Bool(*)(FSM_Machine *, FSM_UserData *, FSM_EventId))0,
	tr_badEventIgnore,
	&lsfsm_sCLPerformerStart,
	"I:BadEventIgnored"
    }
};


PUBLIC FSM_State lsfsm_sCLPerformerStart = {
    lsfsm_ePass,
    lsfsm_xPass,
    lsfsm_tCLPerformerStart,
    "CL-PerformerStart-State"};


STATIC FSM_Trans	lsfsm_tInvokePduReceived[]=
{ 
    {
	lsfsm_EvtResultReq,
	(Bool(*)(FSM_Machine *, FSM_UserData *, FSM_EventId))0,
	tr_clPerformer02,
	&lsfsm_sAckPduWait,
	"U:ResulReq"
    },
    {
	lsfsm_EvtPduInvoke,
	(Bool(*)(FSM_Machine *, FSM_UserData *, FSM_EventId))0,
	tr_clPerformer04,
	&lsfsm_sInvokePduReceived,
	"P:DuplicateInvokePdu"
    },
    {
	FSM_EvtInternal,
	(Bool(*)(FSM_Machine *, FSM_UserData *, FSM_EventId))0,
	tr_clPerformer08 /*NOTYET */,
	&lsfsm_sCLPerformerStart,
	"I:Failure"
    },
    {
	lsfsm_EvtPerfNoResponseTimer,
	(Bool(*)(FSM_Machine *, FSM_UserData *, FSM_EventId))0,
	tr_clPerformer08,
	&lsfsm_sCLPerformerStart,
	"I:Failure"
    },
    {
	FSM_EvtDefault,
	(Bool(*)(FSM_Machine *, FSM_UserData *, FSM_EventId))0,
	tr_badEventIgnore,
	&lsfsm_sInvokePduReceived,
	"I:BadEventIgnored"
    }
};


PUBLIC FSM_State lsfsm_sInvokePduReceived = {
    lsfsm_ePass,
    lsfsm_xPass,
    lsfsm_tInvokePduReceived,
    "CL-InvokePduReceived-State"};

STATIC FSM_Trans	lsfsm_tAckPduWait[]=
{ 
    {
	lsfsm_EvtPduAck,
	(Bool(*)(FSM_Machine *, FSM_UserData *, FSM_EventId))0,
	tr_clPerformer03,
	&lsfsm_sPerformerRefWait,
	"P:AckPdu"
    },
    {
	lsfsm_EvtResultPduRetranTimer,
	(Bool(*)(FSM_Machine *, FSM_UserData *, FSM_EventId))0,
	tr_clPerformer05,
	&lsfsm_sAckPduWait,
	"T:RetranResultPdu"
    },
    {
	lsfsm_EvtLastTimer,
	(Bool(*)(FSM_Machine *, FSM_UserData *, FSM_EventId))0,
	tr_clPerformer09,
	&lsfsm_sPerformerRefWait,
	"T:LastRetranResultPdu"
    },
    {
	lsfsm_EvtPduInvoke,
	(Bool(*)(FSM_Machine *, FSM_UserData *, FSM_EventId))0,
	tr_clPerformer06,
	&lsfsm_sAckPduWait,
	"P:DuplicateInvokePdu"
    },
    {
	FSM_EvtDefault,
	(Bool(*)(FSM_Machine *, FSM_UserData *, FSM_EventId))0,
	tr_badEventIgnore,
	&lsfsm_sAckPduWait,
	"I:BadEventIgnored"
    }
};


PUBLIC FSM_State lsfsm_sAckPduWait = {
    lsfsm_ePass,
    lsfsm_xPass,
    lsfsm_tAckPduWait,
    "CL-AckPduWait-State"};

/* This is needed to prevent late and duplicate InvokePdus from causing trouble */
STATIC FSM_Trans	lsfsm_tPerformerRefWait[]=
{ 
    {
	lsfsm_EvtRefNuTimer,
	(Bool(*)(FSM_Machine *, FSM_UserData *, FSM_EventId))0,
	tr_clPerformer10,
	&lsfsm_sCLPerformerStart,
	"T:PerformerRefWait"
    },
    {
	lsfsm_EvtPduInvoke,
	(Bool(*)(FSM_Machine *, FSM_UserData *, FSM_EventId))0,
	tr_clPerformer07,
	&lsfsm_sPerformerRefWait,
	"P:DuplicateInvokePdu"
    },
    {
	lsfsm_EvtPduAck,
	(Bool(*)(FSM_Machine *, FSM_UserData *, FSM_EventId))0,
	tr_clPerformer11,
	&lsfsm_sPerformerRefWait,
	"P:DuplicateAckPdu"
    },
    {
	FSM_EvtDefault,
	(Bool(*)(FSM_Machine *, FSM_UserData *, FSM_EventId))0,
	tr_badEventIgnore,
	&lsfsm_sPerformerRefWait,
	"I:BadEventIgnored"
    }
};


PUBLIC FSM_State lsfsm_sPerformerRefWait = {
    lsfsm_ePass,
    lsfsm_xPass,
    lsfsm_tPerformerRefWait,
    "CL-PerformerRefWait-State"};



/*<
 * Function: erop_CLPerformerTransDiag (void)
 *
 * Description: Create the transition diagram of Connectionless performer 
 *              and put it in start state.
 *
 * Arguments: None.
 *
 * Returns: Pointer to Finite State Machine of the Transition Diagram.
 *
 * 
>*/

PUBLIC FSM_TransDiagram *
erop_CLPerformerTransDiag (void)
{
    static FSM_TransDiagram * app;
    static Bool virgin = TRUE;

    if (!virgin) {
        return app;
    }

    virgin = FALSE;

    if ( ! (app = (FSM_TransDiagram *)FSM_TRANSDIAG_create("CLPerformerTransDiag", &lsfsm_sCLPerformerStart)) ) {
	EH_problem("erop_CLPerformerTransDiag: FSM_TRANSDIAG_create failed");
	app = (FSM_TransDiagram *) 0;
    }
    return app;
}

