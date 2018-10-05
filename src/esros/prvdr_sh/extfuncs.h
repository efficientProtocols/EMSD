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
 *  Available Other Licenses -- Usage Of This Software In Non Free Environments:
 * 
 *  License for uses of this software with GPL-incompatible software
 *  (e.g., proprietary, non Free) can be obtained from Neda Communications, Inc.
 *  Visit http://www.neda.com/ for more information.
 * 
 */

/*+
 * File name: extfuncs.h
-*/

extern LOPQ_Event *LOPS_allocEvent (Int eventType); 
extern SuccFail LOPS_putEvent (Int sapDesc, LOPQ_Event *event);
extern SuccFail LOPS_init (void);
extern ESRO_InvokeId LOPS_addInvoke (EROP_InvokeDesc EROP_invokeDesc, 
				     Int sapDesc);
extern void LOPS_remInvoke (Int invokeId);
extern InvokeCB *LOPS_getInvoke (Int invokeId);
extern Int LOPS_getInvokeIdForEROP_InvokeDesc (EROP_InvokeDesc EROP_invokeDesc);
extern Int LOPS_addSap (ESRO_SapSel inSap, ESRO_FunctionalUnit functionalUnit);
extern SuccFail LOPS_remSap (Int sapDesc);
extern SapCB *LOPS_getSap (Int sapDesc); 
extern Int LOPS_lookSap (ESRO_SapSel sapSel);
extern Int LOPS_sapForInvokeId (Int invokeId);
extern Int LOPS_userIn (LOPQ_Primitive *prim, Int len);
extern SuccFail LOPS_remSap (Int);
extern SuccFail LOPS_remSap (Int);
extern SapCB *LOPS_getSap (Int);
extern InvokeCB *LOPS_getInvoke (Int);
extern InvokeCB *LOPS_getInvoke (Int);
extern InvokeCB *LOPS_getInvoke (Int);
extern InvokeCB *LOPS_getInvoke (Int);
extern Int LOPS_invokeInd (ESRO_SapSel locESROSap, ESRO_SapSel remESROSap, 
			   T_SapSel *remTsap, N_SapAddr *remNsap, 
			   EROP_InvokeDesc EROP_invokeDesc,
			   ESRO_OperationValue operationValue, 
			   ESRO_EncodingType encodingType, DU_View parameter); 
extern ESRO_InvokeId LOPS_addInvoke (EROP_InvokeDesc, Int);
extern Int lops_resultInd (Bool isResultNotError, EROP_InvokeDesc invokeDesc, 
			   EROP_UserInvokeRef userInvokeRef,
                           EROP_EncodingType encodingType, DU_View parameter); 
extern Int LOPS_resultInd (EROP_InvokeDesc invokeDesc, 
			   EROP_UserInvokeRef userInvokeRef,
                           EROP_EncodingType encodingType, DU_View parameter);
extern Int LOPS_errorInd (EROP_InvokeDesc invokeDesc, 
			  EROP_UserInvokeRef userInvokeRef,
                          EROP_EncodingType encodingType, 
			  EROP_ErrorValue errorValue, DU_View parameter);
extern Int lops_resultCnf (Bool isResultNotError, EROP_InvokeDesc invokeDesc,
			   EROP_UserInvokeRef userInvokeRef);
extern Int LOPS_resultCnf (EROP_InvokeDesc invokeDesc,
			   EROP_UserInvokeRef userInvokeRef);
extern Int LOPS_errorCnf (EROP_InvokeDesc invokeDesc,
			   EROP_UserInvokeRef userInvokeRef);

extern Int LOPS_failureInd (EROP_InvokeDesc EROP_invokeDesc, 
			    EROP_UserInvokeRef userInvokeRef,
			    EROP_FailureValue failureValue);
extern Int LOPS_invokeReqStat  (EROP_InvokeDesc EROP_invokeDesc,
				LOPQ_InvokeReq *invokeReq);
extern ESRO_InvokeId LOPS_addInvoke (EROP_InvokeDesc, Int);
extern ESRO_InvokeId LOPS_addInvoke (EROP_InvokeDesc, Int);
extern Int LOPS_activateStat (PSQ_PrimQuInfo *primQu, Int sapDesc, int status);
extern Int LOPS_sapBind (ESRO_SapSel sapSel, ESRO_FunctionalUnit functionalUnit);
extern SuccFail LOPS_sapUnbind (ESRO_SapSel sapSel);

extern SuccFail erop_outOfInvokeId();
extern Void EROP_freeLopsInvoke(Void (*lops_freeInvoke)());
extern Int LOPS_getEventSize(Int eventType);

