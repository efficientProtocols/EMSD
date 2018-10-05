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
 * File: fsm.h
 *
 * Description:
 *	This File contains the definititions related to a general Finite State 
 *      Machine, represented by the Module Identifer  FSM_.
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

/*
 * RCS Revision: $Id: fsm.h,v 1.7 1996/12/06 21:40:43 kamran Exp $
 */

#ifndef _FSM_H
#define _FSM_H

#include "tm.h"

typedef Void FSM_Machine;
typedef Void FSM_UserData;

EXTERN TM_ModuleCB *FSM_modCB;

#define FSM_TMEXEC	TM_BIT2
#define FSM_TMGEN	TM_BIT3
#define FSM_TMFUNC	TM_BIT4

typedef int FSM_EventId;

#define    FSM_EvtDefault	0	/* Catch all, any Event */
#define    FSM_EvtInternal	1 	/* Re-Run of FSM_ from within */

#if 0

/* To maximize efficiency and in order not to do an additional
 * dereferencing of machine->userData, the user can define a
 * structure containing both FSM_Machine fields and user's specific
 * fields. The FSM_MACHINE macro is used in the beginning of such
 * structure to define the FSM_ specific fields which are then 
 * followed by user specific fields.
 * The FSM_getUserData(machine) should be used by FSM_ users
 * when accessing their userData.
 */

#define FSM_MACHINE 	struct FSM_Machine *next; \
    			struct FSM_Machine *prev; \
    			Void *transDiag; \
    			struct FSM_State *curState
#endif

typedef struct FSM_State {
    Int (*entry)(FSM_Machine  *machine, 
	    	 FSM_UserData *userData, 
		 FSM_EventId  evtId);
    Int (*exit)(FSM_Machine  *machine, 
		FSM_UserData *userData,
		FSM_EventId  evtId);
    struct FSM_Trans  *trans; 
    String name;
} FSM_State;

typedef struct FSM_Trans {
    FSM_EventId	evtId;
    Bool (*predicate)(FSM_Machine  *machine, 
		      FSM_UserData *userData, 
                      FSM_EventId  evtId);
    Int (*action)(FSM_Machine  *machine, 
		  FSM_UserData *userData, 
		  FSM_EventId  evtId);
    FSM_State  *nextStatePtr;
    String name;
} FSM_Trans;

#ifdef LINT_ARGS /*{*/

extern Int FSM_init (void); 
extern Int FSM_runMachine (FSM_Machine *machine, FSM_EventId evtId); 
extern Bool fsm_evtsMatch (FSM_EventId stateEvtPtr, FSM_EventId evtPtr); 
PUBLIC FSM_Machine * FSM_createMachine(FSM_UserData *userData);
PUBLIC SuccFail FSM_deleteMachine(FSM_Machine *machine);
PUBLIC Int FSM_generateEvent(FSM_Machine *machine, FSM_EventId evtId);
Void FSM_setUserData(FSM_Machine *machine, FSM_UserData *userData);
FSM_UserData *FSM_getUserData(FSM_Machine *machine);

#else

extern Int FSM_init (); 
extern Int FSM_runMachine (); 
extern Bool fsm_evtsMatch (); 
PUBLIC FSM_Machine * FSM_createMachine();
PUBLIC SuccFail FSM_deleteMachine();
PUBLIC Int FSM_generateEvent();
Void FSM_setUserData();
FSM_UserData *FSM_getUserData();

#endif

#endif
