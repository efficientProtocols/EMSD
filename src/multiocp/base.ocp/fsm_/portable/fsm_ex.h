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
 * File: fsm_ex.h
 *
 * Description:
 *	This File contains the definititions related to the Finite State 
 *      Machine of FSM example. Represented by the Module Identifer  FSM_.
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

/*
 * RCS Revision: $Id: fsm_ex.h,v 1.4 1996/12/06 21:40:44 kamran Exp $
 */

#ifndef _FSMEX_H
#define _FSMEX_H

#include "extfuncs.h"

    /* User Action Primitives */
#define fsmex_EvtIdleTo1		2
#define fsmex_Evt1ToIdle		3
#define fsmex_Evt1To2			4
#define fsmex_Evt2ToIdle		5

extern FSM_State fsmex_sIdle;
extern FSM_State fsmex_s1;
extern FSM_State fsmex_s2;

Int fsm_ePass(Void *machine, Void *userData, FSM_EventId evtId);
Int fsm_xPass(Void *machine, Void *userData, FSM_EventId evtId);

#ifdef FUTURE
extern FSM_State fsmex_sError;

LOCAL FSM_Trans	fsmex_tError[]= {
    {FSM_EvtDefault, (Bool(*)())0, (Int(*)())0,  &FSM_sError, (String)""} };

PUBLIC FSM_State fsmex_sError = {
	fsm_eError,
	(Int(*)()) 0,
	fsmex_tError,
        "Error-State"};
#endif

STATIC FSM_Trans	fsmex_tIdle[]= { 
    {fsmex_EvtIdleTo1, (Bool(*)())0, tr_idleTo1,    &fsmex_s1,  "F:IdleTo1"},
    {FSM_EvtDefault,   (Bool(*)())0, tr_badEventIgnore, &fsmex_sIdle, "I:BadEventIgnored"} };

PUBLIC FSM_State fsmex_sIdle = {
    fsm_ePass,
    fsm_xPass,
    fsmex_tIdle,
    "Idle-State"};

STATIC FSM_Trans	fsmex_t1[]= { 
    {fsmex_Evt1ToIdle, (Bool(*)())0, tr_1ToIdle,  &fsmex_sIdle,  "F:1ToIdle"},
    {fsmex_Evt1To2,    (Bool(*)())0, tr_1To2,     &fsmex_s2,     "F:1To2"},
    {FSM_EvtDefault,   (Bool(*)())0, tr_badEventIgnore, &fsmex_s1, "I:BadEventIgnored"} };

PUBLIC FSM_State fsmex_s1 = {
    fsm_ePass,
    fsm_xPass,
    fsmex_t1,
    "State-1"};

STATIC FSM_Trans	fsmex_t2[]= { 
    {fsmex_Evt2ToIdle, (Bool(*)())0, tr_2ToIdle, &fsmex_sIdle, "F:2ToIdle"},
    {FSM_EvtDefault,   (Bool(*)())0, tr_badEventIgnore, &fsmex_s2, "I:BadEventIgnored"} };

PUBLIC FSM_State fsmex_s2 = {
    fsm_ePass,
    fsm_xPass,
    fsmex_t2,
    "State-2"};

typedef union fsmex_EventInfo {  

    struct evt1 {
	Int field1;
	Int field2;
    } evt1;

    struct evt2 {
	Int field1;
	Int field2;
    } evt2;

} fsmex_EventInfo;

fsmex_EventInfo fsmex_evtInfo;     /* Global data for all FSM_ handlers */

#endif
