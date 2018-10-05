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
 * File: userin.c
 *
 * Description:
 *   This file contains the action primitive interface between
 *   the ESROS layer and the user of the ESROS services.
 *
 * Functions:
 *   EROP_invokeReq
 *   EROP_errorReq
 *   EROP_resultReq
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: userin.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
#endif /*}*/

#include "eh.h"
#include "estd.h"
#include "erop_sap.h"
#include "eropfsm.h"
#include "fsm.h"
#include "erop_cfg.h"

#include "esro.h"
#include "erop.h"

#include "local.h" 
#include "extfuncs.h" 


/*<
 * Function:    EROP_invokeReq
 *
 * Description: ESROS-INVOKE.request
 *
 * Arguments:   Invoke descriptor, Local SAP selector, remote SAP selector, 
 *		Transport SAP selector, Network SAP selector, operation value, 
 *		encoding type, parameter.
 *
 * Returns:     0 if successful, -1 otherwise
 *
>*/

PUBLIC Int
EROP_invokeReq(EROP_InvokeDesc 	   *invokeDesc,
	       EROP_UserInvokeRef  userInvokeRef,
	       EROP_SapSel 	   locESROSap,
	       EROP_SapSel 	   remESROSap,
	       T_SapSel 	   *remTsap,
	       N_SapAddr 	   *remNsap,
	       EROP_OperationValue opValue,
	       EROP_EncodingType   encodingType,
	       DU_View 	 	   parameter)
{
    InvokeInfo *invoke;
    Int gotVal;
    SapInfo *sapInfo;
    DU_View eropView;

    TM_TRACE((EROP_modCB, TM_ENTER, "EROP_invokeReq entered\n"));

#ifdef AUTHENTICATE_DOMAIN
    if (erop_authenticateIP(remNsap)) {
	EH_problem("\nProgram is talking to unauthorized network address\n");
	return (FAIL);
    }
#endif

    if ((sapInfo = erop_getSapInfo(locESROSap)) == (SapInfo *) NULL) {
	EH_problem("EROP_invokeReq: erop_getSapInfo failed");
	return (FAIL);
    }

    if ((invoke = erop_invokeInfoCreate(sapInfo)) == (InvokeInfo *) NULL) {
	EH_problem("EROP_invokeReq: erop_invokeInfoCreate failed");
	return (FAIL);
    }

    TM_TRACE((EROP_modCB, TM_ENTER, "EROP_invokeReq: invokeInfo=0x%lx\n", 
	     invoke));

    if (parameter) {
        if (DU_size(parameter) > invokePduSize) {
            EH_problem("EROP_invokeReq: We shouldn't be here. Large data not supported by connectionless engine\n");
	    return (FAIL);
	}
    }

    FSM_evtInfo.invokeReq.userInvokeRef= userInvokeRef;
    FSM_evtInfo.invokeReq.remESROSap   = remESROSap;
    FSM_evtInfo.invokeReq.remTsap      = remTsap;
    FSM_evtInfo.invokeReq.remNsap      = remNsap;
    FSM_evtInfo.invokeReq.opValue      = opValue;
    FSM_evtInfo.invokeReq.encodingType = encodingType;
    FSM_evtInfo.invokeReq.parameter    = eropView = DU_link(parameter);

    /* Select CL-Invoker Transition-Diagram for this invoke (machine) */

    if (sapInfo->functionalUnit == ESRO_3Way) {
    	invoke->transDiag = erop_CLInvokerTransDiag();

    } else if (sapInfo->functionalUnit == ESRO_2Way) {
    	invoke->transDiag = erop_2CLInvokerTransDiag();

    } else {
        EH_problem("EROP_invokeReq: Invalid functional unit assigned to sap\n");
/*        DU_free(eropView); */
	return (FAIL);
    }

    invoke->curState  = invoke->transDiag->state;
    invoke->performingNotInvoking = FALSE;

    if ((gotVal = FSM_runMachine((Void *)invoke, lsfsm_EvtInvokeReq)) < 0) {
	EH_problem("EROP_invokeReq: FSM_runMachine failed");
/*        DU_free(eropView); */
	return (FAIL);
    }

    *invokeDesc = (EROP_InvokeDesc *)invoke;

    TM_TRACE((EROP_modCB, TM_ENTER, 
	      "EROP_invokeReq: Before exit: invokeDesc=0x%lx\n",
	      (unsigned long)invokeDesc));

/*    DU_free(eropView); */

    return (SUCCESS);
}


/*<
 * Function:    EROP_resultReq
 *
 * Description: ESROS RESULT or ERROR request
 *
 * Arguments:   Type (result or error), invoke descriptor, encoding type, 
 *              parameter.
 *
 * Returns:     0 if successful, -1 if unsuccessful.  (?)
 *
>*/

PUBLIC  Int
EROP_resultReq(EROP_InvokeDesc invokeDesc,
	       EROP_UserInvokeRef userInvokeRef,
	       EROP_EncodingType encodingType, 
	       DU_View parameter)

{
    int gotVal;
    DU_View duLink = NULL;

    TM_TRACE((EROP_modCB, TM_ENTER, "EROP_resultReq\n"));

    if (parameter) {
    	duLink = DU_link(parameter);
    }

    FSM_evtInfo.resultReq.isResultNotError = 1;
    FSM_evtInfo.resultReq.userInvokeRef    = userInvokeRef;
    FSM_evtInfo.resultReq.encodingType     = encodingType;
    FSM_evtInfo.resultReq.parameter        = duLink;

    if ((gotVal = FSM_runMachine((Void *)invokeDesc, lsfsm_EvtResultReq)) < 0) {
	if (duLink != (DU_View)0) {

	    TM_TRACE((DU_modCB, DU_MALLOC, 
                     "erop_resultReq: DU_free: parameter=0x%x\n", duLink));

	    DU_free(duLink);
	}
    }
    return (gotVal);
}


/*<
 * Function:    EROP_errorReq
 *
 * Description: ESROS-ERROR.request.
 *
 * Arguments:   Invoke descriptor, encoding type, parameter.
 *
 * Returns:     0 if successful, -1 if unsuccessful.
 *
>*/

PUBLIC  Int
EROP_errorReq(EROP_InvokeDesc invokeDesc,
	      EROP_UserInvokeRef userInvokeRef,
	      EROP_EncodingType encodingType, 
	      EROP_ErrorValue errorValue, 
	      DU_View parameter)

{
    int gotVal;
    DU_View duLink = NULL;

    TM_TRACE((EROP_modCB, TM_ENTER, "EROP_errorReq\n"));

    if (parameter) {
    	duLink = DU_link(parameter);
    }

    FSM_evtInfo.resultReq.isResultNotError = 0;
    FSM_evtInfo.resultReq.userInvokeRef    = userInvokeRef;
    FSM_evtInfo.errorReq.encodingType      = encodingType;
    FSM_evtInfo.errorReq.errorValue        = errorValue;
    FSM_evtInfo.errorReq.parameter         = duLink;

    if ((gotVal = FSM_runMachine((Void *)invokeDesc, lsfsm_EvtResultReq)) < 0) {
	if (duLink != (DU_View)0) {

	    TM_TRACE((DU_modCB, DU_MALLOC, 
                     "erop_errorReq: DU_free: parameter=0x%x\n",
		     parameter));

	    DU_free(parameter);
	}
    }
    return (gotVal);
}


