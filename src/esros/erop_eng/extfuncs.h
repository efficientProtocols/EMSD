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

#include "eropfsm.h"
#include "fsm.h"
#include "fsmtrans.h"
#include "erop.h"

#ifdef LINT_ARGS	/* Arg. checking enabled */

extern SuccFail 
erop_relPdu(InvokeInfo *invokeInfo, PduSeq *pduSeq);
extern SuccFail 
erop_relAllPdu(InvokeInfo *invokeInfo);
extern DU_View 
erop_pduRetrieve(InvokeInfo *invokeInfo, PduSeq *pduSeq);
extern Void 
erop_pduKeep(InvokeInfo *invokeInfo, PduSeq *pduSeq, DU_View dt);
extern DU_View 
erop_invokePdu (InvokeInfo *invoke, ESRO_SapSel remEsroSapSel, 
	        Int invokeRefNu, EROP_OperationValue operationValue, 
		EROP_EncodingType encodingType, DU_View data);
extern DU_View erop_resultPdu (InvokeInfo *invoke, Int invokeRefNu, ESRO_SapSel locEsroSapSel, ESRO_SapSel remEsroSapSel, EROP_EncodingType encodingType, DU_View data);
extern DU_View erop_errorPdu (InvokeInfo *invoke, Int invokeRefNu, ESRO_SapSel locEsroSapSel, ESRO_SapSel remEsroSapSel, EROP_EncodingType encodingType, EROP_ErrorValue errorValue, DU_View data);
extern DU_View erop_ackPdu (InvokeInfo *invoke, Int invokeRefNu);
extern Void erop_freeInvoke (InvokeInfo *);
extern Void erop_relInvoke(InvokeInfo *invoke);

extern Int erop_resultInd (InvokeInfo *invoke, 
			   EROP_UserInvokeRef userInvokeRef,
			   EROP_EncodingType encodingType, 
			   DU_View parameter);
extern Int erop_errorInd (InvokeInfo *invoke, 
			  EROP_UserInvokeRef userInvokeRef,
			  EROP_EncodingType encodingType, 
			  EROP_ErrorValue errorValue, DU_View parameter);
extern Int erop_failureInd (InvokeInfo *invoke, 
			    EROP_UserInvokeRef userInvokeRef,
			    EROP_FailureValue failureValue);

extern SuccFail erop_pduSeqInit(PduSeq *p);

extern PUBLIC FSM_TransDiagram *erop_CLInvokerTransDiag (void);
extern PUBLIC FSM_TransDiagram *erop_CLPerformerTransDiag (void);
extern PUBLIC FSM_TransDiagram *erop_2CLInvokerTransDiag (void);
extern PUBLIC FSM_TransDiagram *erop_2CLPerformerTransDiag (void);
extern LOCAL Void tm_pduPr(tm_ModInfo *modInfo, TM_Mask mask, char *str, DU_View du, int maxLog);

extern Int erop_invokeInd (InvokeInfo *invoke, ESRO_SapSel locESROSap, 
			   ESRO_SapSel remESROSap, T_SapSel *remTsap, 
			   N_SapAddr *remNsap, 
			   EROP_OperationValue operationValue, 
			   EROP_EncodingType encodingType, DU_View parameter);
extern Int 
erop_resultCnf(InvokeInfo *invoke, EROP_UserInvokeRef userInvokeRef);
extern Int 
erop_errorCnf(InvokeInfo *invoke, EROP_UserInvokeRef userInvokeRef);

extern Int tr_badEventIgnore(Void *machine, Void *userData, FSM_EventId evtId);
extern Int tr_clInvoker01 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_clInvoker02 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_clInvoker03 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_clInvoker04 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_clInvoker05 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_clInvoker06 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_clInvoker07 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_clInvoker08 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_clInvoker09 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_clInvoker10 (Void *invoke, Void *userData, FSM_EventId evtId);

extern Int tr_clPerformer01 (Void *invoke, Void *userData, FSM_EventId evtId); 
extern Int tr_clPerformer02 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_clPerformer03 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_clPerformer04 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_clPerformer05 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_clPerformer06 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_clPerformer07 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_clPerformer08 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_clPerformer09 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_clPerformer10 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_clPerformer11 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_clPerformer12 (Void *invoke, Void *userData, FSM_EventId evtId);

extern Int tr_2badEventIgnore(Void *machine, Void *userData, FSM_EventId evtId);
extern Int tr_2clInvoker01 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_2clInvoker02 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_2clInvoker03 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_2clInvoker04 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_2clInvoker05 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_2clInvoker06 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_2clInvoker07 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_2clInvoker08 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_2clInvoker09 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_2clInvoker10 (Void *invoke, Void *userData, FSM_EventId evtId);

extern Int tr_2clPerformer01 (Void *invoke, Void *userData, FSM_EventId evtId); 
extern Int tr_2clPerformer02 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_2clPerformer03 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_2clPerformer04 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_2clPerformer05 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_2clPerformer06 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_2clPerformer07 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_2clPerformer08 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_2clPerformer09 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_2clPerformer10 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_2clPerformer11 (Void *invoke, Void *userData, FSM_EventId evtId);
extern Int tr_2clPerformer12 (Void *invoke, Void *userData, FSM_EventId evtId);

extern LOCAL Int lower_dataInd(T_SapSel *remTsapSel,
	      		       N_SapAddr *remNsapAddr,
	      		       T_SapSel *locTsapSel,
	      		       N_SapAddr *locNsapAddr,
	      		       DU_View data);

extern SuccFail erop_sendFailurePdu(T_SapSel *tSapSel, N_SapAddr *naddr, 
			     	    short unsigned int invokeRefNu, 
			     	    unsigned char reason);
extern int getConf();
extern SuccFail erop_authenticateIP(N_SapAddr *remNsap);

#else

extern SuccFail erop_relPdu();
extern SuccFail erop_relAllPdu();
extern DU_View erop_pduRetrieve();
extern Void erop_pduKeep();
extern DU_View erop_invokePdu ();
extern DU_View erop_resultPdu ();
extern DU_View erop_ackPdu ();
extern Int erop_resultInd ();
extern Int erop_errorInd ();
extern Int erop_failureInd ();
extern Void EROP_freeInvoke ();
extern Void erop_relInvoke();

extern SuccFail erop_pduSeqInit();

extern PUBLIC FSM_TransDiagram *erop_CLInvokerTransDiag ();
extern PUBLIC FSM_TransDiagram *erop_CLPerformerTransDiag ();
extern PUBLIC FSM_TransDiagram *erop_2CLInvokerTransDiag ();
extern PUBLIC FSM_TransDiagram *erop_2CLPerformerTransDiag ();
extern void tm_pduPr ();

extern Int erop_invokeInd ();
extern Int erop_resultCnf ();
extern Int erop_errorCnf (); 

extern Int tr_badEventIgnore();
extern Int tr_clInvoker01 ();
extern Int tr_clInvoker02 ();
extern Int tr_clInvoker03 ();
extern Int tr_clInvoker04 ();
extern Int tr_clInvoker05 ();
extern Int tr_clInvoker06 ();
extern Int tr_clInvoker07 ();
extern Int tr_clInvoker08 ();
extern Int tr_clInvoker09 ();
extern Int tr_clInvoker10 ();

extern Int tr_badEventIgnore();
extern Int tr_clPerformer01 (); 
extern Int tr_clPerformer02 ();
extern Int tr_clPerformer03 ();
extern Int tr_clPerformer04 ();
extern Int tr_clPerformer05 ();
extern Int tr_clPerformer06 ();
extern Int tr_clPerformer07 ();
extern Int tr_clPerformer08 ();
extern Int tr_clPerformer09 ();
extern Int tr_clPerformer10 ();
extern Int tr_clPerformer11 ();
extern Int tr_clPerformer12 ();

extern Int tr_2badEventIgnore();
extern Int tr_2clInvoker01 ();
extern Int tr_2clInvoker02 ();
extern Int tr_2clInvoker03 ();
extern Int tr_2clInvoker04 ();
extern Int tr_2clInvoker05 ();
extern Int tr_2clInvoker06 ();
extern Int tr_2clInvoker07 ();
extern Int tr_2clInvoker08 ();
extern Int tr_2clInvoker09 ();
extern Int tr_2clInvoker10 ();

extern Int tr_2clPerformer01 ();
extern Int tr_2clPerformer02 ();
extern Int tr_2clPerformer03 ();
extern Int tr_2clPerformer04 ();
extern Int tr_2clPerformer05 ();
extern Int tr_2clPerformer06 ();
extern Int tr_2clPerformer07 ();
extern Int tr_2clPerformer08 ();
extern Int tr_2clPerformer09 ();
extern Int tr_2clPerformer10 ();
extern Int tr_2clPerformer11 ();
extern Int tr_2clPerformer12 ();

extern LOCAL Int lower_dataInd();
extern SuccFail erop_sendFailurePdu();
extern int getConf();
extern SuccFail erop_authenticateIP();

#endif
