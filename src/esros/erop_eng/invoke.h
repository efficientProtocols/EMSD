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
 * File name: invoke.h
 *
 * Description:
 *
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 * 
 */

/*
 * RCS Revision: $Id: invoke.h,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $
 */

#ifndef _INVOKE_H_	/*{*/
#define _INVOKE_H_

#include "addr.h"
#include "queue.h"
#include "du.h"

#include "tmr_if.h"

#include "erop.h"
#include "target.h"

typedef struct PduSeq {
     DU_View first;
     DU_View last;
} PduSeq;

/*
 *  The  invoke structure.
 */

typedef struct InvokeInfoSeq {
    struct InvokeInfo *first;
    struct InvokeInfo *last;
} InvokeInfoSeq;

typedef struct InvokeInfo {
    struct InvokeInfo *next;		
    struct InvokeInfo *prev;
    
    struct FSM_TransDiagram *transDiag;    /* Code/Program to be run by this machine */
    struct FSM_State	*curState;
    Bool		performingNotInvoking;
		
    struct SapInfo	*locSap;	/* local  user */
    
    EROP_UserInvokeRef  userInvokeRef;  /* ESROS User Invoke Reference */
    ESRO_SapSel		remEsroSapSel;	/* Remote Address */
    T_SapSel		remTsapSel;
    N_SapAddr 		remNsapAddr;	/* remote network address */

    EROP_OperationValue	operationValue;
    EROP_EncodingType	encodingType;
    EROP_ErrorValue	errorValue;
    Int 		failureReason;
    Int 		invokeRefNu;
 
#ifndef FIXED_TIMERS
    TmrDuration		maxLocAckDelay;	/* local acknowledge time */
    TmrDuration		maxRemAckDelay;	/* remote acknowledge time */
    TmrDuration		roundTripDelay;	/* current round trip delay average */
#endif

    TmrDuration		retransTimeout;	/* retransmission timer interval */

    TmrDuration		refKeepTime;
    TmrDuration		rwait; 
    TmrDuration		inactivityDelay; 
    TmrDuration		perfNoResponse;

    PduSeq		invokePduSeq;
    PduSeq		resultPduSeq;

    short		nuOfRetrans;		/* max retransmission count */

    Bool		isResultNotError;

    TmrQuHead	tmrQu;

    QU_Element refMap;

} InvokeInfo;

#define erop_getNsapAddr(elem) ((N_SapAddr *)((unsigned long int)elem-(unsigned long int)nSapAddrOffset))

#define erop_getInvokeInfoAddr(elem) ((InvokeInfo *)((unsigned long int)elem-(unsigned long int)invokeInfoOffset))

Void (*erop_freeLopsInvoke)(InvokeInfo *);

#include "erop_sap.h"

#ifdef LINT_ARGS /*{*/

extern void erop_invokeInit (Int); 
extern InvokeInfo *erop_invokeInfoCreate (); 
extern void erop_invokeInfoFree (InvokeInfo *); 
extern void erop_invokeInfoInit (InvokeInfo *); 
extern Int erop_refInit (Int); 
extern short int erop_refNew (InvokeInfo *); 
extern SuccFail erop_refKeep (Int refNu, InvokeInfo *); 
extern InvokeInfo *erop_refToPerformer(short unsigned int refNu, N_SapAddr *nSapAddr);
extern InvokeInfo *erop_refToInvoker(short unsigned int refNu, N_SapAddr *nSapAddr);
extern void erop_refFree (InvokeInfo *);
extern void LOPS_freeInvoke (InvokeInfo *);

#else

extern void erop_invokeInit (); 
extern InvokeInfo *erop_invokeInfoCreate ();
extern void erop_invokeInfoFree (); 
extern void erop_invokeInfoInit (); 
extern void erop_refInit (); 
extern short int erop_refNew (); 
extern SuccFail erop_refKeep (); 
extern InvokeInfo *erop_refToInvoker ();
extern InvokeInfo *erop_refToPerformer ();
extern void erop_refFree ();
extern void LOPS_freeInvoke ();

#endif

#ifdef __STDC__
#else
#endif

#endif	/*}*/
