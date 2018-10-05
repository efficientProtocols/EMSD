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
 * File name: ps_prim.c
 *
 * Description: Provider Shell Primitives
 *
 * Functions:
 *   lops_getUserData(unsigned char *buffer, int length)
 *   lops_userIn(LOPQ_Primitive *prim, Int len)
 *   LOPS_invokeInd( EROP_InvokeDesc	EROP_invokeDesc,
 *   		ESRO_SapSel 	locESROSap,
 *   		ESRO_SapSel 	remESROSap,
 *   		T_SapSel		*remTsap,
 *   		N_SapAddr		*remNsap,
 *   		ESRO_OperationValue	operationValue,
 *   		ESRO_EncodingType	encodingType,
 *   		DU_View		parameter)
 *   lops_resultInd(Bool isResultNotError,
 *   	       	EROP_InvokeDesc	invokeDesc,
 *		ESRO_UserInvokeRef      userInvokeRef,
 *   	       	EROP_EncodingType	encodingType,
 *   	       	DU_View 	parameter)
 *   LOPS_resultInd( EROP_InvokeDesc	invokeDesc,
 *		ESRO_UserInvokeRef      userInvokeRef,
 *   		EROP_EncodingType	encodingType,
 *   		DU_View 	parameter)
 *   LOPS_errorInd( EROP_InvokeDesc	invokeDesc,
 *		ESRO_UserInvokeRef      userInvokeRef,
 *   	       	EROP_EncodingType	encodingType,
 *   	       	DU_View 	parameter)
 *   lops_resultCnf(Bool isResultNotError,
 *   	       	EROP_InvokeDesc	invokeDesc,
 *		ESRO_UserInvokeRef      userInvokeRef)
 *   LOPS_resultCnf( EROP_InvokeDesc	invokeDesc,
 *		ESRO_UserInvokeRef      userInvokeRef)
 *   LOPS_errorCnf( EROP_InvokeDesc	invokeDesc,
 *		ESRO_UserInvokeRef      userInvokeRef)
 *   LOPS_failureInd(EROP_InvokeDesc	EROP_invokeDesc,
 *		ESRO_UserInvokeRef      userInvokeRef,
 *   		EROP_FailureValue	failureValue)
 *   LOPS_invokeReqStat(EROP_InvokeDesc EROP_invokeDesc, 
                        LOPQ_InvokeReq *invokeReq)
 *   LOPS_activateStat(PSQ_PrimQuInfo *primQu, Int sapDesc, int status)
 *   LOPS_sapBind(EROP_SapDesc sapDesc, ESRO_SapSel sapSel)
 *   LOPS_sapUnbind(ESRO_SapSel sapSel)
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: ps_prim.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
#endif /*}*/

#include "estd.h"
#include "eh.h"
#include "tm.h"
#include "du.h"
#include "target.h"
#include "sf.h"
#include "imq.h"
#include "esro.h"
#include "esro_sh.h"
#include "sch.h"

#include "local.h"
#include "extfuncs.h"

extern SuccFail lops_isLopsInvokeCBFull();


/*<
 * Function:    lops_getUserData
 *
 * Description: Get data and its length. Allocate Data Unit memory and copy
 *              the data.
 *
 * Arguments:   Data, length.
 *
 * Returns:     Pointer to Data Unit View.
 * 
>*/

STATIC  DU_View
lops_getUserData(unsigned char *buffer, int length)
{
    DU_View du;
    extern DU_Pool *G_duMainPool;

    if (length == 0) {
	return((DU_View) 0);
    }

    if ( (du = DU_alloc(G_duMainPool, length)) == 0 ) {
	EH_problem("lops_getUserData: DU_alloc failed\n");	
	return (DU_View)NULL;
    }

    TM_TRACE((DU_modCB, DU_MALLOC, 
	     "lops_getUserData: DU_alloc: du=0x%lx\n", du));

    OS_copy(DU_data(du), buffer, length);

    return(du);
}



/*<
 * Function:    lops_userIn
 *
 * Description: Get primitive and process it.
 *
 * Arguments:   Primitive.
 *
 * Returns:     0 if successful, a negative error number if unsuccessful.
 * 
>*/

Int
lops_userIn(LOPQ_Primitive *prim, Int len)
{
    Int retVal = 0;
    char errBuf[1024];

    TM_TRACE((LOPS_modCB, TM_ENTER,
	    "lops_userIn:  prim->type=%d len=%d\n", prim->type, len));

    switch (prim->type) {

    case LOPQ_SAP_BIND:
	{
	    LOPQ_SapActivate    *activate;
	    Int sapDesc; 
	    SapCB *sapCB;
	    PSQ_PrimQuInfo *primQu;

	    activate = &(prim->un.activate);

	    if ((primQu = PSQ_getQuInfo(activate->chnlName)) 
		== (PSQ_PrimQuInfo *)0) {
		EH_problem("LOPS_userIn: PSQ_getQuInfo failed. "
			   "SAP doesn't have queue"); 
		return (FAIL);
	    }

	    if ((sapDesc = LOPS_addSap(activate->locSapSel, 
				       activate->functionalUnit)) < 0) {
	        if (LOPS_activateStat(primQu, (Int)NULL, -8) == FAIL) {
		    TM_TRACE((LOPS_modCB, TM_ENTER, 
			      "LOPS_userIn: LOPS_activateStat(FAIL) failed\n"));
	        }
		TM_TRACE((LOPS_modCB, TM_ENTER, 
		         "LOPS_userIn: LOPS_addSap failed for sapSel=%d\n",
			 activate->locSapSel));
		return ( FAIL );
	    }

	    if ((sapCB = LOPS_getSap(sapDesc)) == (SapCB *) 0 ) {   /* redun. */
	        if (LOPS_activateStat(primQu, (Int)NULL, -9) == FAIL) {
		    TM_TRACE((LOPS_modCB, TM_ENTER, 
			      "LOPS_userIn: LOPS_activateStat(FAIL) failed\n"));
	        }
		TM_TRACE((LOPS_modCB, TM_ENTER, 
		         "LOPS_userIn: LOPS_getSap failed\n"));
		return ( FAIL );
	    }

	    sapCB->primQu = primQu;

	    if (LOPS_activateStat(primQu, sapDesc, 0) == FAIL) {
		TM_TRACE((LOPS_modCB, TM_ENTER, 
			 "LOPS_userIn: LOPS_activateStat failed\n"));
		return (FAIL);
	    }
	}
	return (SUCCESS);

    case LOPQ_SAP_UNBIND:
	{
	    LOPQ_SapDeActivate  *deActivate;

	    deActivate = &(prim->un.deActivate);
	    if ( LOPS_remSap(deActivate->locSapDesc) ) {
		TM_TRACE((LOPS_modCB, TM_ENTER, 
			 "LOPS_userIn: SAP not active: %d\n",
			 deActivate->locSapDesc));
		return (FAIL);
	    }
	    return (SUCCESS); 
	}

    case LOPQ_INVOKE_REQ:
    {
	LOPQ_InvokeReq 	*invokeReq;
	SapCB 	       	*sapCB;
	EROP_InvokeDesc EROP_invokeDesc;
	ESRO_SapSel 	locESROSap;
	DU_View 	parameter;

	TM_TRACE((LOPS_modCB, TM_ENTER, "LOPS_userin: InvokeRequest\n"));

	invokeReq = &(prim->un.invokeReq);

	if (lops_isLopsInvokeCBFull()) {
	    LOPS_invokeReqStat(0, invokeReq);
	    EH_problem("LOPS_userIn: InvokeReq: failed: "
		       "invoke Control Block full\n");
	    return (FAIL);
        }

	EROP_invokeDesc = (EROP_InvokeDesc )0;
	if ((sapCB = LOPS_getSap(invokeReq->locSapDesc))) {

	    locESROSap = sapCB->sapSel;
	    parameter = lops_getUserData(invokeReq->parameter,
			                 invokeReq->parameterLen);

	    if (EROP_invokeReq(&EROP_invokeDesc,
				invokeReq->userInvokeRef,
				locESROSap,
		       	        invokeReq->remESROSap,
		     	        &(invokeReq->remTsap),
		     	        &(invokeReq->remNsap),
		     	        invokeReq->opValue, 
		     	        invokeReq->encodingType,
		     	        parameter) == FAIL) {

		TM_TRACE((LOPS_modCB, TM_ENTER, 
	                 "lops_userIn: EROP_invokeReq failed\n")); 
	    	if (parameter) {
            	    TM_TRACE((DU_modCB, DU_MALLOC, 
	                     "lops_userIn: DU_free: parameter=0x%lx\n", 
			     parameter));
	    	    DU_free(parameter);
	    	}
		return (FAIL);
	    } 
	    TM_TRACE((LOPS_modCB, TM_ENTER, 
		     "LOPS_userin: InvokeRequest: invokeDesc=0x%lx\n", 
		     EROP_invokeDesc));

	} else {
	    TM_TRACE((LOPS_modCB, TM_ENTER, "LOPS_getSap() failed.\n"));
	    return (FAIL);
	}
	/* LOPS_invokeReqStat takes care of LOPS_addInvoke */
	if (LOPS_invokeReqStat(EROP_invokeDesc, invokeReq) == FAIL) {

	    TM_TRACE((LOPS_modCB, TM_ENTER, "LOPS_invokeReqStat failed.\n"));
	    if (parameter) {
            	TM_TRACE((DU_modCB, DU_MALLOC, 
	                 "lops_userIn: DU_free: parameter=0x%lx\n", parameter));
	    	DU_free(parameter);
	    }
	    return (FAIL);
	}

	if (parameter) {
            TM_TRACE((DU_modCB, DU_MALLOC, 
	             "lops_userIn: DU_free: parameter=0x%lx\n", parameter));
	    DU_free(parameter);
	}
	return (SUCCESS);
    }

    case LOPQ_RESULT_REQ:
    {
	LOPQ_ResultReq *resultReq;
	InvokeCB *invokeCB;
	EROP_InvokeDesc EROP_invokeDesc;
	DU_View du;

    	TM_TRACE((LOPS_modCB, TM_ENTER, "LOPS_userin: ResultRequest\n"));

	resultReq = &(prim->un.resultReq);

	if ((invokeCB = LOPS_getInvoke(resultReq->invokeId)) == (InvokeCB *)0) {
	    sprintf(errBuf, "lops_userIn: ResultReq: no entry in invoke "
			    "control block. invokId=%ld\n", 
			    (long int)resultReq->invokeId);
	    EH_problem(errBuf);
	    return (FAIL);
	}
	    
	if ( (EROP_invokeDesc = invokeCB->EROP_invokeDesc) 	
	     != (EROP_InvokeDesc ) 0 ) {
	    /* So the connection is good */

	    du = lops_getUserData(resultReq->parameter, 
				  resultReq->parameterLen); 

    	    TM_TRACE((LOPS_modCB, TM_ENTER, 
		     "LOPS_userin: ResultRequest: du=0x%lx\n", du));

	    if (EROP_resultReq(EROP_invokeDesc,
			       resultReq->userInvokeRef,
			       resultReq->encodingType,
			       du) == FAIL) {

                if (du) {
              	    TM_TRACE((DU_modCB, DU_MALLOC, 
			       "lops_userIn: DU_free: du=0x%lx\n", du));
    		    DU_free(du);
		}
     		sprintf( errBuf, "lops_userIn: ResultReq: erop_resultReq "
				 "failed. invokeId=%ld\n", 
				 (long int)resultReq->invokeId);
     		EH_problem(errBuf);
		return (FAIL);
	    }

  	    if (du) {
                TM_TRACE((DU_modCB, DU_MALLOC, 
 			 "lops_userIn: DU_free: du=0x%lx\n", du));
                DU_free(du);
            }
	}
	return (SUCCESS);
    }

    case LOPQ_ERROR_REQ:
    {
	LOPQ_ErrorReq 	*errorReq;
	InvokeCB 	*invokeCB;
	EROP_InvokeDesc EROP_invokeDesc;
	DU_View 	du;

    	TM_TRACE((LOPS_modCB, TM_ENTER, "LOPS_userin: ErrorRequest\n"));

	errorReq = &(prim->un.errorReq);

	if ( (invokeCB = LOPS_getInvoke(errorReq->invokeId)) 
	     == (InvokeCB *) 0 ) {
	    sprintf(errBuf, "lops_userIn: ErrorReq: no entry in invoke "
			    "control block. invokeId=%ld\n", 
			    (long int)errorReq->invokeId);
	    EH_problem(errBuf);
	    return (FAIL);
	}
	    
	if ( (EROP_invokeDesc = invokeCB->EROP_invokeDesc) != 
	     (EROP_InvokeDesc ) 0 ) {
	    /* So the connection is good */

	    du = lops_getUserData(errorReq->parameter, errorReq->parameterLen);

	    if (EROP_errorReq(EROP_invokeDesc,
			      errorReq->userInvokeRef,
			      errorReq->encodingType,
			      errorReq->errorValue,
			      du) == (FAIL)) {
	    	sprintf(errBuf, "lops_userIn: EROP_errorReq failed. "
			    "invokeId=%ld\n", (long int)errorReq->invokeId);
	    	EH_problem(errBuf);

                if (du) {
	            TM_TRACE((DU_modCB, DU_MALLOC, 
			     "lops_userIn: DU_free: du=0x%lx\n", du));
    		    DU_free(du);
	  	}
		return (FAIL);
	    }
	    if (du) {
                TM_TRACE((DU_modCB, DU_MALLOC, 
 			 "lops_userIn: DU_free: du=0x%lx\n", du));
                DU_free(du);
	    }
	}
	return (SUCCESS);
    }

    default:

        TM_TRACE((LOPS_modCB, TM_ENTER, 
		 "LOPS_userin: bad event %u\n", prim->type));

	EH_problem("lops_userIn: invalid primitive type");
	retVal = -1;
    }
    return ( retVal );

} /* lops_userIn() */


 

/*<
 * Function:    LOPS_invokeInd
 *
 * Description: Process INVOKE.indication.
 *
 * Arguments:   Invoke descriptor, local SAP selector, remote SAP selector, 
 *              transport, SAP selector, Network SAP selector, operation 
 *              value, encoding type, parameter.
 *
 * Returns:     0 if successful, a negative error value if unsuccessful.
 * 
>*/

LOCAL Int
LOPS_invokeInd( ESRO_SapSel 		locESROSap,
		ESRO_SapSel 		remESROSap,
		T_SapSel		*remTsap,
		N_SapAddr		*remNsap,
		EROP_InvokeDesc		EROP_invokeDesc,
		ESRO_OperationValue	operationValue,
		ESRO_EncodingType	encodingType,
		DU_View			parameter)
{
    LOPQ_Event *event;
    LOPQ_InvokeInd *invokeInd;
    Int sapDesc;

    if ((event = LOPS_allocEvent(LOPQ_INVOKE_IND)) == (LOPQ_Event *)NULL) {
	EH_problem("LOPS_invokeInd: LOPS_allocEvent failed.\n");
	return (FAIL);
    }

    invokeInd = &(event->un.invokeInd);

    if ((sapDesc   = LOPS_lookSap(locESROSap)) < 0) {
	EH_problem("LOPS_invokeInd: SAP not active\n");
	return (FAIL);
    }

    if ((invokeInd->invokeId = LOPS_addInvoke(EROP_invokeDesc, sapDesc)) < 0) {
	erop_outOfInvokeId(EROP_invokeDesc, remTsap, remNsap, 1);
	EH_problem("LOPS_invokeInd: add invoke failed\n");
	return (FAIL);
    }

    TM_TRACE((LOPS_modCB, TM_ENTER, 
	     "LOPS_invokeInd: *****invokeDesc=0x%lx  invokeId=%d\n", 
	     EROP_invokeDesc, invokeInd->invokeId));

    invokeInd->locSapDesc = sapDesc;
    /* We need to decide about what to do
     * with locSapDesc field, look it up in tsapMap and forward it
     * only if that sap is active.
     */

    invokeInd->remESROSap = remESROSap;
    invokeInd->remTsap    = *remTsap;
    invokeInd->remNsap    = *remNsap; 

    invokeInd->operationValue = operationValue;
    invokeInd->encodingType   = encodingType;

    if ( parameter != (DU_View)0 ) {
	invokeInd->len = DU_size(parameter);

	OS_copy(invokeInd->data, DU_data(parameter), DU_size(parameter));

        TM_TRACE((DU_modCB, DU_MALLOC, 
		 "LOPS_invokeInd: DU_free: 0x%lx\n", parameter));

	DU_free(parameter);
    } else {
	invokeInd->len = 0;
    }

    if (LOPS_putEvent(sapDesc, event) == FAIL) {
	EH_problem("LOPS_invokeInd: LOPS_putEvent failed.\n");
	return (FAIL);
    }

    return (SUCCESS);

} /* LOPS_invokeInd() */


/*<
 * Function:    lops_resultInd
 *
 * Description: Process RESULT.indication and ERROR.indication.
 *
 * Arguments:   Result/error switch, invoke descriptor, encoding type, 
 *              parameter.
 *
 * Returns:     0 if successful, a negative error value if unsuccessful.
 * 
>*/

LOCAL Int
lops_resultInd(Bool 		  isResultNotError,
	       EROP_InvokeDesc	  invokeDesc,
	       EROP_UserInvokeRef userInvokeRef,
	       EROP_EncodingType  encodingType,
	       DU_View 	 	  parameter)
{
    LOPQ_Event 	*resultEvent;
    LOPQ_Event 	*errorEvent;
    LOPQ_ResultInd *resultInd;
    Int 	invokeId;
    Int 	retVal = (SUCCESS);

    if ((resultEvent = LOPS_allocEvent(LOPQ_RESULT_IND)) == 
        (LOPQ_Event *)NULL) {
	EH_problem("LOPS_resultInd: LOPS_allocEvent failed for resultEvent.\n");
	return (FAIL);
    }
    if ((errorEvent = LOPS_allocEvent(LOPQ_ERROR_IND)) == (LOPQ_Event *)NULL) {
    	SF_memRelease(resultEvent);
	EH_problem("LOPS_resultInd: LOPS_allocEvent failed for errorEvent.\n");
	return (FAIL);
    }

    if ( isResultNotError ) {
	resultInd = &(resultEvent->un.resultInd);
    } else {
	resultInd = (LOPQ_ResultInd *) &(errorEvent->un.errorInd);
    }

    invokeId = resultInd->invokeId = 
	       LOPS_getInvokeIdForEROP_InvokeDesc(invokeDesc);

    if (invokeId < 0) {
	EH_problem("LOPS_resultInd: Invalid Invoke Descriptor\n");
    	SF_memRelease(errorEvent);
    	SF_memRelease(resultEvent);
	return (FAIL);
    }

    resultInd->userInvokeRef = userInvokeRef;
    resultInd->encodingType  = encodingType;

    if ( parameter != (DU_View)0 ) {
	resultInd->len = DU_size(parameter);

	OS_copy(resultInd->data, DU_data(parameter), DU_size(parameter));

        TM_TRACE((DU_modCB, DU_MALLOC, 
		 "LOPS_resultInd: DU_free: 0x%lx\n", parameter));

	DU_free(parameter);
    } else {
	resultInd->len = 0;
    }

    if ( isResultNotError ) {
	if (LOPS_putEvent(LOPS_sapForInvokeId(invokeId), resultEvent) == FAIL) {
	    retVal = FAIL;
	}
    	SF_memRelease(errorEvent);
    } else {
	if (LOPS_putEvent(LOPS_sapForInvokeId(invokeId), errorEvent) == FAIL) {
	    retVal = FAIL;
	}
    	SF_memRelease(resultEvent);
    }

    return retVal;

} /* lops_resultInd() */

 
/*<
 * Function:    LOPS_resultInd
 *
 * Description: Process RESULT.indication.
 *
 * Arguments:   Invoke descriptor, encoding type, parameter.
 *
 * Returns:     0 if successful, a negative error value if unsuccessful.
 * 
>*/

LOCAL Int
LOPS_resultInd( EROP_InvokeDesc	   invokeDesc,
		EROP_UserInvokeRef userInvokeRef,
		EROP_EncodingType  encodingType,
		DU_View 	   parameter)
{
    return lops_resultInd(TRUE, invokeDesc, 
			  userInvokeRef, 
			  encodingType, parameter);
}
 

/*<
 * Function:    LOPS_errorInd
 *
 * Description: Process ERROR.indication.
 *
 * Arguments:   Invoke descriptor, encoding type, parameter.
 *
 * Returns:     0 if successful, a negative error value if unsuccessful.
 * 
>*/

LOCAL Int
LOPS_errorInd( EROP_InvokeDesc	  invokeDesc,
	       EROP_UserInvokeRef userInvokeRef,
	       EROP_EncodingType  encodingType,
	       EROP_ErrorValue	  errorValue,
	       DU_View 	  	  parameter)
{
    LOPQ_Event 	  *errorEvent;
    LOPQ_ErrorInd *errorInd;
    Int 	  invokeId;
    Int 	  retVal = (SUCCESS);

    if ((errorEvent = LOPS_allocEvent(LOPQ_ERROR_IND)) == (LOPQ_Event *)NULL) {
	EH_problem("LOPS_resultInd: LOPS_allocEvent failed for errorEvent\n");
	return (FAIL);
    }

    errorInd = (LOPQ_ErrorInd *) &(errorEvent->un.errorInd);

    invokeId = errorInd->invokeId = 
	       LOPS_getInvokeIdForEROP_InvokeDesc(invokeDesc);

    if (invokeId < 0) {
	EH_problem("LOPS_resultInd: Invalid Invoke Descriptor\n");
    	SF_memRelease(errorEvent);
	return (FAIL);
    }

    errorInd->userInvokeRef = userInvokeRef;
    errorInd->encodingType  = encodingType;
    errorInd->errorValue    = errorValue;

    if ( parameter != (DU_View)0 ) {
	errorInd->len = DU_size(parameter);

	OS_copy(errorInd->data, DU_data(parameter), DU_size(parameter));

        TM_TRACE((DU_modCB, DU_MALLOC, 
		 "LOPS_errorInd: DU_free: 0x%lx\n", parameter));

	DU_free(parameter);
    } else {
	errorInd->len = 0;
    }

   if (LOPS_putEvent(LOPS_sapForInvokeId(invokeId), errorEvent) == FAIL) {
	retVal = FAIL;
    	SF_memRelease(errorEvent);
   }
   return retVal;

} /* LOPS_errorInd() */


/*<
 * Function:    lops_resultCnf
 *
 * Description: Process RESULT.confirm and ERROR.confirm.
 *
 * Arguments:   Result/Error switch, invoke descriptor.
 *
 * Returns:     0 if successful, a negative error value if unsuccessful.
 * 
>*/

LOCAL Int
lops_resultCnf(	Bool isResultNotError,
	       	EROP_InvokeDesc	invokeDesc,
		EROP_UserInvokeRef userInvokeRef)
{
    LOPQ_Event 	*resultEvent;
    LOPQ_Event 	*errorEvent;
    LOPQ_ResultCnf *resultCnf;
    Int 	invokeId;
    Int 	retVal = (SUCCESS);

    if ((resultEvent = LOPS_allocEvent(LOPQ_RESULT_CNF)) 
	== (LOPQ_Event *)NULL) {
	EH_problem("LOPS_resultCnf: LOPS_allocEvent failed for resultEvent\n");
	return (FAIL);
    }
    if ((errorEvent = LOPS_allocEvent(LOPQ_ERROR_CNF)) == (LOPQ_Event *)NULL) {
	EH_problem("LOPS_resultCnf: LOPS_allocEvent failed for errorEvent\n");
    	SF_memRelease(resultEvent);
	return (FAIL);
    }

    if ( isResultNotError ) {
	resultCnf = &(resultEvent->un.resultCnf);
    } else {
	resultCnf = (LOPQ_ResultCnf *) &(errorEvent->un.errorCnf);
    }

    invokeId = resultCnf->invokeId = 
	       LOPS_getInvokeIdForEROP_InvokeDesc(invokeDesc);
    resultCnf->userInvokeRef = userInvokeRef;

    if (invokeId < 0) {
	EH_problem("LOPS_resultCnf: Invalid Invoke Descriptor\n");
    	SF_memRelease(resultEvent);
    	SF_memRelease(errorEvent);
	return (FAIL);
    }

    if ( isResultNotError ) {
	if (LOPS_putEvent(LOPS_sapForInvokeId(invokeId), resultEvent) == FAIL) {
	    retVal = FAIL;
	}
    	SF_memRelease(errorEvent);
    } else {
	if (LOPS_putEvent(LOPS_sapForInvokeId(invokeId), errorEvent) == FAIL) {
	    retVal = FAIL;
	}
    	SF_memRelease(resultEvent);
    }
    return retVal;

} /* lops_resultCnf() */


/*<
 * Function:    LOPS_resultCnf
 *
 * Description: Process RESULT.confirm.
 *
 * Arguments:   Invoke descriptor.
 *
 * Returns:     0 if successful, a negative error value if unsuccessful.
 * 
>*/


LOCAL Int
LOPS_resultCnf( EROP_InvokeDesc	invokeDesc, 
		EROP_UserInvokeRef userInvokeRef)
{
    return lops_resultCnf(TRUE, invokeDesc, userInvokeRef);
}
 

/*<
 * Function:    LOPS_errorCnf
 *
 * Description: Process ERROR.confirm.
 *
 * Arguments:   Invoke descriptor.
 *
 * Returns:     0 if successful, a negative error value if unsuccessful.
 * 
>*/

LOCAL Int
LOPS_errorCnf( EROP_InvokeDesc	invokeDesc, 
		EROP_UserInvokeRef userInvokeRef)
{
    return lops_resultCnf(FALSE, invokeDesc, userInvokeRef);
}

 

/*<
 * Function:    LOPS_failureInd
 *
 * Description: Process FAILURE.indication.
 *
 * Arguments:   Invoke descriptor, failure reason.
 *
 * Returns:     0 if successful, a negative error value if unsuccessful.
 * 
>*/

LOCAL Int
LOPS_failureInd(EROP_InvokeDesc	   EROP_invokeDesc,
		EROP_UserInvokeRef userInvokeRef,
		EROP_FailureValue  failureValue)

{
    LOPQ_Event 	*event;
    LOPQ_FailureInd *failureInd;
    Int 	invokeId;

    TM_TRACE((LOPS_modCB, TM_ENTER, 
	     "LOPS_failureInd: *****invokeDesc=0x%lx\n", EROP_invokeDesc));

    if ((event = LOPS_allocEvent(LOPQ_FAILURE_IND)) == (LOPQ_Event *)NULL) {
	EH_problem("LOPS_failureInd: LOPS_allocEvent failed.\n");
	return (FAIL);
    }

    failureInd = &(event->un.failureInd);

    invokeId = failureInd->invokeId = 
	       LOPS_getInvokeIdForEROP_InvokeDesc(EROP_invokeDesc);
    failureInd->userInvokeRef = userInvokeRef;

    if (invokeId < 0) {
	EH_problem("LOPS_failureInd: Invalid Invoke Descriptor\n");
        TM_TRACE((LOPS_modCB, TM_ENTER, 
		 "                invokeDesc=0x%lx", 
		 EROP_invokeDesc));
    	SF_memRelease(event);
	return (FAIL);
    }

    LOPS_remInvoke(failureInd->invokeId);

    failureInd->failureValue = failureValue;

    if (LOPS_putEvent(LOPS_sapForInvokeId(invokeId), event) == FAIL) {
	char errBuf[256];
	sprintf(errBuf, "LOPS_failureInd: LOPS_putEvent failed. invokeId=%ld\n",
		   (long int)failureInd->invokeId);
	EH_problem(errBuf);
	return (FAIL);
    }

    return (SUCCESS);

} /* LOPS_failureInd() */


/*<
 * Function:    LOPS_invokeReqStat
 *
 * Description: Send Invoke Request status
 *
 * Arguments:   Invoke descriptor, SAP descriptor.
 *
 * Returns:     0 if successful, a negative error value if unsuccessful.
 * 
>*/

LOCAL Int
LOPS_invokeReqStat(EROP_InvokeDesc EROP_invokeDesc, LOPQ_InvokeReq *invokeReq)
{
    LOPQ_PStatus *pStatus;
    LOPQ_InvokeReqStat *invokeReqStat;
    Int sapDesc;

    sapDesc = invokeReq->locSapDesc;

    if ((pStatus =(LOPQ_PStatus *) LOPS_allocEvent(LOPQ_INVOKE_REQ_STAT))
        == (LOPQ_PStatus *)NULL) {
	EH_problem("LOPS_invokeReqStat: LOPS_allocEvent failed.\n");
	return (FAIL);
    }

    invokeReqStat = &(pStatus->un.invokeReqStat);

    invokeReqStat->locSapDesc = sapDesc;
    if ( EROP_invokeDesc != (EROP_InvokeDesc ) 0 ) {
	if ( (invokeReqStat->invokeId =  
	      LOPS_addInvoke(EROP_invokeDesc, sapDesc)) < 0 ) {
	    EH_problem("LOPS_invokeReqStat: add invoke failed\n");
	    pStatus->status = FAIL;
	} else {
	    pStatus->status = SUCCESS;
	}
    } else {
	/* Define the Status Codes */
	pStatus->status = FAIL;
    }
    invokeReqStat->userShellInvokeId = invokeReq->userShellInvokeId;

    if (LOPS_putEvent(sapDesc, (LOPQ_Event *)pStatus) == FAIL) {
	EH_problem("LOPS_invokeReqStat: LOPS_putEvent failed.\n");
	return (FAIL);
    }

    return (SUCCESS);

} /* LOPS_invokeReqStat() */


/*<
 * Function:    LOPS_activateStat
 *
 * Description: Send SAP activation status
 *
 * Arguments:   SAP descriptor.
 *
 * Returns:     0 if successful, a negative error value if unsuccessful.
 * 
>*/

Int
LOPS_activateStat(PSQ_PrimQuInfo *primQu, Int sapDesc, int status)
{
    LOPQ_PStatus *pStatus;
    LOPQ_SapActivateStat *activateStat;

    if ((pStatus =(LOPQ_PStatus *) LOPS_allocEvent(LOPQ_SAP_BIND_STAT))
	== (LOPQ_PStatus *)NULL) {
	EH_problem("LOPS_activateStat: LOPS_allocEvent failed\n");
	return (FAIL);
    }

    activateStat = &(pStatus->un.activateStat);
    if (status < 0) {
	pStatus->status = status;		/* failure status */
	activateStat->locSapDesc = 0;		/* not necessary, not used */
        TM_TRACE((LOPS_modCB, TM_ENTER, 
		 "LOPS_activateStat: sapBind failed: "
		 "sapDesc=%ld, FailureValue=%d", 
		 (long int)sapDesc, status));
	if (PSQ_putEvent(primQu, (Ptr) pStatus, 
		         LOPS_getEventSize(LOPQ_SAP_BIND_STAT))  == FAIL) {
	    return (FAIL);

	}
	return (SUCCESS);

    } else {
	pStatus->status 	 = SUCCESS;
	activateStat->locSapDesc = sapDesc;
        TM_TRACE((LOPS_modCB, TM_ENTER, 
	         "LOPS_activateStat: sapDesc=%ld, status=%d\n", 
		 (long int)sapDesc, status));

        if (LOPS_putEvent(sapDesc, (LOPQ_Event *)pStatus) == (FAIL)) {
	    EH_problem("LOPS_activateStat: LOPS_putEvent failed");
	    return (FAIL);
       	}
        return (SUCCESS);
    }

} /* LOPS_activateStat() */


/*<
 * Function: LOPS_sapBind
 *
 * Description: Bind a SAP
 *
 * Arguments: SAP selector.
 *
 * Returns:   Status of EROP_sapBind
 *
 * 
>*/
LOCAL Int
LOPS_sapBind(ESRO_SapSel sapSel, ESRO_FunctionalUnit functionalUnit)
{
    EROP_SapDesc sapDesc;
    
    TM_TRACE((LOPS_modCB, TM_ENTER, "LOPS_BindSap: sapSel=%d  funcUnit=%d\n", 
	     sapSel, functionalUnit));

    return ( EROP_sapBind(&sapDesc,
			  sapSel, 
			  functionalUnit,	
			  LOPS_invokeInd,
			  LOPS_resultInd,
			  LOPS_errorInd,
			  LOPS_resultCnf,
			  LOPS_errorCnf,
			  LOPS_failureInd) );
}

/*<
 * Function:    LOPS_sapUnbind
 *
 * Description: Unbind a SAP.
 *
 * Arguments:   SAP selector.
 *
 * Returns:     Status of EROP_sapUnbind
 *
 * 
>*/

LOCAL SuccFail
LOPS_sapUnbind(ESRO_SapSel sapSel)
{
    Int statVal;
    char errBuf[256];

    if ( (statVal = EROP_sapUnbind(sapSel)) ) {
	sprintf(errBuf, "LOPS_sapUnbind: SAP unbind failed. sapSel=%d", sapSel);
	EH_problem(errBuf);

	TM_TRACE((LOPS_modCB, TM_ENTER,
		 "LOPS_SapUnbind: sapSel=%d\n", sapSel));

    }
    return ( statVal );
}

/*<
 * Function:    lops_getSapDesc
 *
 * Description: Get SAP description
 *
 * Arguments:   Queue info
 *
 * Returns:     SAP descriptor
 *
 * 
>*/
    
Int 
lops_getSapDesc(PSQ_PrimQuInfo *quInfo)
{
    Int sapDesc;

    for (sapDesc = 0; sapDesc < DIMOF(psSapCBMap); sapDesc++) {
	if (psSapCBMap[sapDesc].primQu == quInfo) {
    	    return sapDesc;
	}
    }
    return -1;
}

/*<
 * Function:    lops_cleanSap
 *
 * Description: Clean SAP
 *
 * Arguments:   Queue info
 *
 * Returns:     0 on successful cleaning, -1 otherwise.
 *
 * 
>*/

Int
lops_cleanSap(PSQ_PrimQuInfo *quInfo)
{
    Int sapDesc;

#ifndef MSDOS     /*     SCH_AllTaskDelete(quInfo->actionQu, NULL); NOTYET */
    SCH_allTaskDelete(quInfo->actionQu, NULL, NULL); 
#endif

    if ( (sapDesc = lops_getSapDesc(quInfo)) < 0 ) {
	EH_problem("lops_cleanSap: lops_getSapDesc failed.\n");
	return (FAIL);
    }

    if ( LOPS_remSap(sapDesc) < 0 ) {
	EH_problem("lops_cleanSap: lops_remSap failed.\n");
	return (FAIL);
    }

    return (SUCCESS);
}


