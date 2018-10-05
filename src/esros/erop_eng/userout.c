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
 * File:        userout.c
 *
 * Description: This file contains the event primitive interface between
 *              the ESROS layer and the user of the ESROS services.
 *
 * Functions:
 *   erop_invokeInd()
 *   erop_resultInd()
 *   erop_errorInd()
 *   erop_resultCnf()
 *   erop_errorCnf()
 *   erop_failureInd()
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: userout.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
#endif /*}*/

#include "estd.h"
#include "invoke.h"
#include "erop_sap.h"

#include "eropfsm.h"
#include "nm.h"
#include "sf.h"
#include "eh.h"
#include "sch.h"
#include "layernm.h"

#include "local.h"
#include "extfuncs.h"

typedef struct InvokeIndArg
{
    InvokeInfo 		*invoke;
    ESRO_SapSel		locESROSap;
    ESRO_SapSel 	remESROSap;
    T_SapSel		*remTsap;
    N_SapAddr		*remNsap;
    EROP_OperationValue	operationValue;
    EROP_EncodingType	encodingType;
    DU_View		parameter;
} InvokeIndArg;

typedef struct ResultIndArg
{
    InvokeInfo 	  	*invoke;
    EROP_UserInvokeRef 	userInvokeRef;
    EROP_EncodingType  	encodingType;
    DU_View		parameter;
} ResultIndArg;

typedef struct ErrorIndArg
{
    InvokeInfo 		*invoke;
    EROP_UserInvokeRef 	userInvokeRef;
    EROP_EncodingType   encodingType;
    EROP_ErrorValue	errorValue;
    DU_View 		parameter;
} ErrorIndArg;

typedef struct ResultConfArg
{
    InvokeInfo *invoke;
    EROP_UserInvokeRef 	userInvokeRef;
} ResultConfArg;

typedef struct ErrorConfArg
{
    InvokeInfo *invoke;
    EROP_UserInvokeRef 	userInvokeRef;
} ErrorConfArg;

typedef struct FailureIndArg
{
    InvokeInfo        *invoke;
    EROP_UserInvokeRef 	userInvokeRef;
    EROP_FailureValue failureValue;
} FailureIndArg;

Int erop_scheduledInvokeInd(InvokeIndArg *arg);
Int erop_scheduledResultInd(ResultIndArg *arg);
Int erop_scheduledErrorInd(ErrorIndArg *arg);
Int erop_scheduledResultCnf(ResultConfArg *arg);
Int erop_scheduledErrorCnf(ErrorConfArg *arg);
Int erop_scheduledFailureInd(FailureIndArg *arg);


/*<
 * Function:    erop_invokeInd
 *
 * Description: ESRO-INVOKE.indication
 *
 * Arguments:   Invoke info, SAP selector, Transport SAP selector, Network
 *              SAP selector, operation value, encoding type, parameter.
 *
 * Returns:     
 *
>*/

LOCAL Int 
erop_invokeInd(InvokeInfo 	   *invoke,
	       ESRO_SapSel 	   locESROSap,
	       ESRO_SapSel 	   remESROSap,
	       T_SapSel		   *remTsap,
	       N_SapAddr	   *remNsap,
	       EROP_OperationValue operationValue,
	       EROP_EncodingType   encodingType,
	       DU_View		   parameter)
{
    InvokeIndArg *arg;

#ifdef TM_ENABLED
    char taskN[100] = "Invoke Indication: ";
#endif

#ifdef AUTHENTICATE_DOMAIN
    if (erop_authenticateIP(remNsap)) {
	EH_problem("\nProgram is talking to unauthorized network address\n");
	return (FAIL);
    }
#endif

    if ((arg = SF_memGet(sizeof(InvokeIndArg)))) {
    } else {
	EH_problem("Memory allocation failed");
	return (FAIL);
    } 

    arg->invoke         = invoke;
    arg->locESROSap     = locESROSap;
    arg->remESROSap     = remESROSap;
    arg->remTsap        = remTsap;
    arg->remNsap        = remNsap;
    arg->operationValue = operationValue;
    arg->encodingType   = encodingType;
    arg->parameter      = parameter;

    TM_TRACE((EROP_modCB, TM_ENTER, 
	     "erop_invokeInd: Invoke.indication scheduled: "
	     "refNu=%ld", 
	     arg->invoke));

#ifdef TM_ENABLED
        strcpy(taskN, "Invoke Indication: ");
    	return SCH_submit ((Void *)erop_scheduledInvokeInd, 
		           (Ptr) arg, 
		           SCH_PSEUDO_EVENT, 
		           (String) strcat(taskN, TM_here()));
#else
    	return SCH_submit ((Void *)erop_scheduledInvokeInd, 
		           (Ptr) arg, 
		           SCH_PSEUDO_EVENT);
#endif
}


/*<
 * Function:    erop_scheduledInvokeInd
 *
 * Description: Call scheduled invoke indication (call-back function)
 *
 * Arguments:	Invoke inducation info.
 *
 * Returns: 
 *
>*/

LOCAL Int 
erop_scheduledInvokeInd(InvokeIndArg *arg)
{
    TM_TRACE((EROP_modCB, TM_ENTER, 
	     "erop_scheduledInvokeInd: Invoke.indication call-back function "
	     "called: refNu=%ld\n", arg->invoke));

    (*(arg->invoke->locSap->invokeInd)) (
				    arg->locESROSap,
				    arg->remESROSap,
				    arg->remTsap,
				    arg->remNsap,
				    (EROP_InvokeDesc) arg->invoke,
				    arg->operationValue,
				    arg->encodingType,
				    arg->parameter);
    DU_free(arg->parameter);
    SF_memRelease(arg);
    return 0;
}


/*<
 * Function:    erop_resultInd
 *
 * Description: ESROS-RESULT.indication.
 *
 * Arguments:   Invoke info, encoding type, parameter.
 *
 * Returns:     
 *
>*/

LOCAL Int 
erop_resultInd(InvokeInfo 	  *invoke,
	       EROP_UserInvokeRef userInvokeRef,
	       EROP_EncodingType  encodingType,
	       DU_View		  parameter)
{
    ResultIndArg *arg;

#ifdef TM_ENABLED
    char taskN[100] = "Result Indication: ";
#endif

    if ((arg = SF_memGet(sizeof(ResultIndArg)))) {
    } else {
	EH_problem("Memory allocation failed");
	return (FAIL);
    } 

    arg->invoke        = invoke;
    arg->userInvokeRef = userInvokeRef;
    arg->encodingType  = encodingType;
    arg->parameter     = parameter;

    TM_TRACE((EROP_modCB, TM_ENTER, 
	     "erop_resultInd: Result.indication scheduled: "
	     "refNu=%ld, userInvokeRef=%ld", 
	     arg->invoke, arg->userInvokeRef));

#ifdef TM_ENABLED
        strcpy(taskN, "Result Indication: ");
    	return SCH_submit ((Void *)erop_scheduledResultInd, 
		           (Ptr) arg, 
		           SCH_PSEUDO_EVENT, 
		           (String) strcat(taskN, TM_here()));
#else
    	return SCH_submit ((Void *)erop_scheduledResultInd, 
		           (Ptr) arg, 
		           SCH_PSEUDO_EVENT);
#endif

}


/*<
 * Function:    erop_scheduledResultInd
 *
 * Description: Call scheduled result indication function.
 *
 * Arguments:	Result indication parameters.
 *
 * Returns: 
 *
>*/

Int
erop_scheduledResultInd(ResultIndArg *arg)
{
    TM_TRACE((EROP_modCB, TM_ENTER, 
	     "erop_scheduledResultInd: Result.indication call-back function "
	     "called: refNu=%ld, userInvokeRef=%ld", 
	     arg->invoke, arg->userInvokeRef));

    (*arg->invoke->locSap->resultInd) ((EROP_InvokeDesc) arg->invoke, 
	       				arg->userInvokeRef,
				        arg->encodingType, 
					arg->parameter);
    DU_free(arg->parameter);
    SF_memRelease(arg);
    return 0;
}


/*<
 * Function:    erop_errorInd
 *
 * Description: ESROS-ERROR.indication event.
 *
 * Arguments:   Invoke info, encoding type, parameter.
 *
 * Returns:     
 *
>*/

LOCAL Int 
erop_errorInd(InvokeInfo *invoke,
	      EROP_UserInvokeRef userInvokeRef,
	      EROP_EncodingType  encodingType,
	      EROP_ErrorValue	 errorValue,
	      DU_View 		 parameter)
{
    ErrorIndArg *arg;

#ifdef TM_ENABLED
    char taskN[100] = "Error Indication: ";
#endif

    if ((arg = SF_memGet(sizeof(ErrorIndArg)))) {
    } else {
	EH_problem("Memory allocation failed");
	return (FAIL);
    } 

    arg->invoke        = invoke;
    arg->userInvokeRef = userInvokeRef;
    arg->encodingType  = encodingType;
    arg->errorValue    = errorValue;
    arg->parameter     = parameter;

    TM_TRACE((EROP_modCB, TM_ENTER, 
	     "erop_errorInd: Error.indication scheduled: "
	     "refNu=%ld, userInvokeRef=%ld", 
	     arg->invoke, arg->userInvokeRef));

#ifdef TM_ENABLED
        strcpy(taskN, "Error Indication: ");
    	return SCH_submit ((Void *)erop_scheduledErrorInd, 
		           (Ptr) arg, 
		           SCH_PSEUDO_EVENT, 
		           (String) strcat(taskN, TM_here()));
#else
    	return SCH_submit ((Void *)erop_scheduledErrorInd, 
		           (Ptr) arg, 
		           SCH_PSEUDO_EVENT);
#endif

}


/*<
 * Function:    erop_scheduledErrorInd
 *
 * Description: Call scheduled error indication function.
 *
 * Arguments:	Error indication parameters.
 *
 * Returns:
 *
>*/

Int
erop_scheduledErrorInd(ErrorIndArg *arg)
{
    TM_TRACE((EROP_modCB, TM_ENTER, 
	     "erop_scheduledErrorInd: Error.indication call-back function "
	     "called: refNu=%ld, userInvokeRef=%ld", 
	     arg->invoke, arg->userInvokeRef));

    (*arg->invoke->locSap->errorInd) ((EROP_InvokeDesc) arg->invoke, 
				      arg->userInvokeRef,
				      arg->errorValue, 
				      arg->encodingType, 
				      arg->parameter);

    DU_free(arg->parameter);
    SF_memRelease(arg);
    return 0;
}


/*<
 * Function:    erop_resultCnf
 *
 * Description: ESROS-RESULT.confirm event.
 *
 * Arguments:   Invoke info.
 *
 * Returns:
 *
>*/

LOCAL Int 
erop_resultCnf(InvokeInfo *invoke,
	       EROP_UserInvokeRef userInvokeRef)
{
    ResultConfArg *arg;

#ifdef TM_ENABLED
    char taskN[100] = "Result Confirm: ";
#endif

    if ((arg = SF_memGet(sizeof(ResultConfArg)))) {
    } else {
	EH_problem("Memory allocation failed");
	return (FAIL);
    } 

    arg->invoke        = invoke;
    arg->userInvokeRef = userInvokeRef;

    TM_TRACE((EROP_modCB, TM_ENTER, 
	     "erop_resultCnf: Result.confirmation scheduled: "
	     "refNu=%ld, userInvokeRef=%ld", 
	     arg->invoke, arg->userInvokeRef));

#ifdef TM_ENABLED
        strcpy(taskN, "Result Confirmation: ");
    	return SCH_submit ((Void *)erop_scheduledResultCnf, 
		           (Ptr) arg, 
		           SCH_PSEUDO_EVENT, 
		           (String) strcat(taskN, TM_here()));
#else
    	return SCH_submit ((Void *)erop_scheduledResultCnf, 
		           (Ptr) arg, 
		           SCH_PSEUDO_EVENT);
#endif
}


/*<
 * Function:    erop_scheduledResultCnf
 *
 * Description: Call scheduled result confirm
 *
 * Arguments:	Result confirm parameters.
 *
 * Returns: 
 *
>*/

Int
erop_scheduledResultCnf(ResultConfArg *arg)
{
    TM_TRACE((EROP_modCB, TM_ENTER, 
	     "erop_scheduledResultCnf: Result.confirmation call-back function "
	     "called: refNu=%ld, userInvokeRef=%ld", 
	     arg->invoke, arg->userInvokeRef));

    (*arg->invoke->locSap->resultCnf) ((EROP_InvokeDesc) arg->invoke,
				        arg->userInvokeRef);

    SF_memRelease(arg);
    return 0;
}


/*<
 * Function:    erop_errorCnf
 *
 * Description: ESROS-ERROR.indication event.
 *
 * Arguments:   Invoke info.
 *
 * Returns:
 *
>*/

LOCAL Int 
erop_errorCnf(InvokeInfo *invoke, EROP_UserInvokeRef userInvokeRef)
{
    ErrorConfArg *arg;

#ifdef TM_ENABLED
    char taskN[100] = "Error Confirm: ";
#endif

    if ((arg = SF_memGet(sizeof(ErrorConfArg)))) {
    } else {
	EH_problem("Memory allocation failed");
	return (FAIL);
    } 

    arg->invoke        = invoke;
    arg->userInvokeRef = userInvokeRef;

    TM_TRACE((EROP_modCB, TM_ENTER, 
	     "erop_errorCnf: Error.confirmation scheduled: "
	     "refNu=%ld, userInvokeRef=%ld", 
	     arg->invoke, arg->userInvokeRef));

#ifdef TM_ENABLED
        strcpy(taskN, "Error Confirmation: ");
    	return SCH_submit ((Void *)erop_scheduledErrorCnf, 
		           (Ptr) arg, 
		           SCH_PSEUDO_EVENT, 
		           (String) strcat(taskN, TM_here()));
#else
    	return SCH_submit ((Void *)erop_scheduledErrorCnf, 
		           (Ptr) arg, 
		           SCH_PSEUDO_EVENT);
#endif
}


/*<
 * Function:    erop_scheduledErrorCnf
 *
 * Description: Call scheduled error confirm call-back function
 *
 * Arguments:	Error confirm parameters.
 *
 * Returns: 
 *
>*/

Int
erop_scheduledErrorCnf(ErrorConfArg *arg)
{
    TM_TRACE((EROP_modCB, TM_ENTER, 
	     "erop_scheduledErrorCnf: Error.confirmation call-back function "
	     "called: refNu=%ld, userInvokeRef=%ld", 
	     arg->invoke, arg->userInvokeRef));

    (*arg->invoke->locSap->errorCnf) ((EROP_InvokeDesc) arg->invoke,
				      arg->userInvokeRef);
    SF_memRelease(arg);
    return 0;
}


/*<
 * Function:    erop_failureInd
 *
 * Description: ESROS-FAILURE.indication event.
 *
 * Arguments:   Invoke info, failure reason.
 *
 * Returns:     
 *
>*/
LOCAL Int 
erop_failureInd(InvokeInfo        *invoke,
	        EROP_UserInvokeRef userInvokeRef,
		EROP_FailureValue failureValue)
{
    FailureIndArg *arg;

#ifdef TM_ENABLED
    char taskN[100] = "Failure Indication: ";
#endif

    if ((arg = SF_memGet(sizeof(FailureIndArg)))) {
    } else {
	EH_problem("Memory allocation failed");
	return (FAIL);
    } 

    arg->invoke        = invoke;
    arg->userInvokeRef = userInvokeRef;
    arg->failureValue  = failureValue;

    TM_TRACE((EROP_modCB, TM_ENTER, 
	     "erop_failureInd: Failure.indication scheduled: "
	     "refNu=%ld, userInvokeRef=%ld", 
	     arg->invoke, arg->userInvokeRef));

#ifdef TM_ENABLED
        strcpy(taskN, "Failure Indication: ");
    	return SCH_submit ((Void *)erop_scheduledFailureInd, 
		           (Ptr) arg, 
		           SCH_PSEUDO_EVENT, 
		           (String) strcat(taskN, TM_here()));
#else
    	return SCH_submit ((Void *)erop_scheduledFailureInd, 
		           (Ptr) arg, 
		           SCH_PSEUDO_EVENT);
#endif
}


/*<
 * Function:    erop_scheduledFailureInd
 *
 * Description: Call scheduled failure indication call-back function
 *
 * Arguments:	Failure indication paramenters.
 *
 * Returns: 
 *
>*/

Int
erop_scheduledFailureInd(FailureIndArg *arg)
{
    TM_TRACE((EROP_modCB, TM_ENTER, 
	     "erop_scheduledFailureInd: Failure.indication call-back function "
	     "called: refNu=%ld, userInvokeRef=%ld", 
	     arg->invoke, arg->userInvokeRef));

    (*arg->invoke->locSap->failureInd) ((EROP_InvokeDesc) arg->invoke, 
				        arg->userInvokeRef,
					arg->failureValue);

    SF_memRelease(arg);
    return 0;
}
