/*
 *
 *Copyright (C) 1997-2002  Neda Communications, Inc.
 *
 *This file is part of ESRO. An implementation of RFC-2188.
 *
 *ESRO is free software; you can redistribute it and/or modify it
 *under the terms of the GNU General Public License (GPL) as 
 *published by the Free Software Foundation; either version 2,
 *or (at your option) any later version.
 *
 *ESRO is distributed in the hope that it will be useful, but WITHOUT
 *ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with ESRO; see the file COPYING.  If not, write to
 *the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *Boston, MA 02111-1307, USA.  
 *
*/
/*
 * 
 * Available Other Licenses -- Usage Of This Software In Non Free Environments:
 *
 * License for uses of this software with GPL-incompatible software
 * (e.g., proprietary, non Free) can be obtained from Neda Communications, Inc.
 * Visit http://www.neda.com/ for more information.
 *
*/
/*+
 * File: eropfsm.h
 *
 * Description:
 *	This File contains the definititions related to the Finite State 
 *      Machine of ESROS engine. Represented by the Module Identifer  FSM_.
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

/*
 * RCS Revision: $Id: eropfsm.h,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $
 */

#ifndef _EROPFSM_H
#define _EROPFSM_H

#include "tm.h"
#include "addr.h"

#include "invoke.h"
#include "erop.h"

    /* User Action Primitives */
#define lsfsm_EvtInvokeReq		2
#define lsfsm_EvtErrorReq		3
#define lsfsm_EvtResultReq		4

#define lsfsm_EvtPduInputInd		5
    /* In PDU Indications */
#define    lsfsm_EvtPduInvoke		6
#define    lsfsm_EvtPduResult		7
#define    lsfsm_EvtPduAck		8
#define    lsfsm_EvtPduFailure		9
    
#define    lsfsm_EvtTimerInd		10
    /* Timer Indications */
#define    lsfsm_EvtInvokePduRetranTimer	11
#define    lsfsm_EvtResultPduRetranTimer	12
#define    lsfsm_EvtRefNuTimer		13
#define    lsfsm_EvtInactivityTimer	14
#define    lsfsm_EvtLastTimer		15
#define    lsfsm_EvtPerfNoResponseTimer	16

typedef union FSM_EventInfo {  
    struct InvokeReq {
	EROP_UserInvokeRef userInvokeRef;
	ESRO_SapSel remESROSap;
	T_SapSel *remTsap;
	N_SapAddr *remNsap;
        EROP_OperationValue opValue;
	EROP_EncodingType encodingType;
	DU_View  parameter;
    } invokeReq;

    struct ResultReq {
	Bool	isResultNotError;
	EROP_UserInvokeRef userInvokeRef;
	EROP_EncodingType encodingType;
	DU_View    	  parameter;
    } resultReq;

    struct ErrorReq {
	Bool	isResultNotError;
	EROP_UserInvokeRef userInvokeRef;
	EROP_EncodingType encodingType;
	EROP_ErrorValue   errorValue;
	DU_View  	  parameter;
    } errorReq;

    struct InternalInfo {
	Int	expiredTimerName;
    } internalInfo;
    struct TmrInfo {
	Int	name;
	long 	subscript;
	Int 	datum;
    } tmrInfo;
} FSM_EventInfo;

/* LCL_XTRN FSM_EventInfo FSM_evtInfo; */     /* Global data for all FSM_ handlers */
FSM_EventInfo FSM_evtInfo;     /* Global data for all FSM_ handlers */

/*
typedef InvokeInfo FSM_Machine;
*/
#endif
