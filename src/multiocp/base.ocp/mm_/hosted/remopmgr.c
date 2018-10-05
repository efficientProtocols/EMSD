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

/*
 * Author: Derrell Lipman
 * History:
 *
 */

/*
 * Module Management Agent functionality
 */


#include "estd.h"
#include "rc.h"
#include "queue.h"
#include "tmr.h"
#include "buf.h"
#include "mm.h"
#include "mmlocal.h"


static ReturnCode
formatPdu(MM_RemoteOperationType operationType,
	  MM_RemoteOperationValue operationValue,
	  MM_Protocol * p,
	  void ** phBuf);


/*
 * MM_remoteOperationInit()
 *
 * Initialize the Module Management Remote Operation subsystem.
 */
ReturnCode
MM_remoteOperationInit(void)
{
    return mm_asnInit();
}



/*
 * MM_remoteOperationRequest()
 *
 * Create a protocol request structure, and format it into a PDU.
 *
 * Parameters
 *
 *     pRequest --
 *       Pointer to a structure containing the parameters for this request.
 *
 *     phBuf --
 *       Pointer to a location in which the handle of the buffer containing
 *       the formatted PDU will be placed.
 */
ReturnCode
MM_remoteOperationRequest(MM_Request * pRequest,
			  void ** phBuf)
{
    ReturnCode	    rc;
    MM_Protocol	    protocol;

    /* Zero out the protocol struture to make it easy to clean up later */
    OS_memSet((void *) &protocol, '\0', sizeof(MM_Protocol));

    switch(pRequest->operationValue)
    {
    case MM_RemOp_GetValue:
	/*
	 * Get the current value of a managable object.
	 */

	/* Get the application entity instance name */
	if ((rc =
	     STR_attachZString(pRequest->
			       un.getValue.pApplicationEntityInstanceName,
			       FALSE,
			       &protocol.hApplicationEntityInstanceName)) !=
	    Success)
	{
	    goto cleanUp;
	}
    
	/* Get the module name */
	if ((rc = STR_attachZString(pRequest->un.getValue.pModuleName, FALSE,
				    &protocol.hModuleName)) != Success)
	{
	    goto cleanUp;
	}
    
	/* Get the managable object name */
	if ((rc = STR_attachZString(pRequest->un.getValue.pManagableObjectName,
				    FALSE,
				    &protocol.hManagableObjectName)) !=
	    Success)
	{
	    goto cleanUp;
	}

	break;

    case MM_RemOp_SetValue:
	/*
	 * Get the current value of a managable object.
	 */

	/* Get the application entity instance name */
	if ((rc =
	     STR_attachZString(pRequest->
			       un.setValue.pApplicationEntityInstanceName,
			       FALSE,
			       &protocol.hApplicationEntityInstanceName)) !=
	    Success)
	{
	    goto cleanUp;
	}
    
	/* Get the module name */
	if ((rc = STR_attachZString(pRequest->un.setValue.pModuleName,
				    FALSE,
				    &protocol.hModuleName)) != Success)
	{
	    goto cleanUp;
	}
    
	/* Get the managable object name */
	if ((rc =
	     STR_attachZString(pRequest->un.setValue.pManagableObjectName,
			       FALSE,
			       &protocol.hManagableObjectName)) != Success)
	{
	    goto cleanUp;
	}

	/* Get the value */
	switch(pRequest->un.setValue.value.type)
	{
	case MM_ValueType_SignedInt:
	    protocol.value.choice = MM_VALUE_CHOICE_SIGNEDINT;
	    protocol.value.un.signedInt =
		pRequest->un.setValue.value.un.signedInt;
	    break;

	case MM_ValueType_UnsignedInt:
	    protocol.value.choice = MM_VALUE_CHOICE_UNSIGNEDINT;
	    protocol.value.un.unsignedInt =
		pRequest->un.setValue.value.un.unsignedInt;
	    break;

	case MM_ValueType_String:
	    protocol.value.choice = MM_VALUE_CHOICE_STRING;

	    if ((rc = STR_attachZString(pRequest->un.setValue.value.un.string,
					FALSE,
					&protocol.value.un.string)) != Success)
	    {
		goto cleanUp;
	    }

	    break;
	}
    
	break;

    case MM_RemOp_GetThreshold:
	/*
	 * Get the current threshold of a managable object.
	 */

	/* Get the application entity instance name */
	if ((rc =
	     STR_attachZString(pRequest->un.getThreshold.
			       pApplicationEntityInstanceName,
			       FALSE,
			       &protocol.hApplicationEntityInstanceName)) !=
	    Success)
	{
	    goto cleanUp;
	}
    
	/* Get the module name */
	if ((rc = STR_attachZString(pRequest->un.getThreshold.pModuleName,
				    FALSE,
				    &protocol.hModuleName)) != Success)
	{
	    goto cleanUp;
	}
    
	/* Get the managable object name */
	if ((rc =
	     STR_attachZString(pRequest->un.getThreshold.pManagableObjectName,
			       FALSE,
			       &protocol.hManagableObjectName)) != Success)
	{
	    goto cleanUp;
	}

	/* Get the threshold type */
	protocol.thresholdType = pRequest->un.getThreshold.thresholdType;

	break;

    case MM_RemOp_SetThreshold:
	/*
	 * Get the current threshold of a managable object.
	 */

	/* Get the application entity instance name */
	if ((rc =
	     STR_attachZString(pRequest->
			       un.setThreshold.pApplicationEntityInstanceName,
			       FALSE,
			       &protocol.hApplicationEntityInstanceName)) !=
	    Success)
	{
	    goto cleanUp;
	}
    
	/* Get the module name */
	if ((rc = STR_attachZString(pRequest->un.setThreshold.pModuleName,
				    FALSE,
				    &protocol.hModuleName)) != Success)
	{
	    goto cleanUp;
	}
    
	/* Get the managable object name */
	if ((rc =
	     STR_attachZString(pRequest->un.setThreshold.pManagableObjectName,
			       FALSE,
			       &protocol.hManagableObjectName)) != Success)
	{
	    goto cleanUp;
	}

	/* Get the threshold */
	switch(pRequest->un.setThreshold.value.type)
	{
	case MM_ValueType_SignedInt:
	    protocol.value.choice = MM_VALUE_CHOICE_SIGNEDINT;
	    protocol.value.un.signedInt =
		pRequest->un.setThreshold.value.un.signedInt;
	    break;

	case MM_ValueType_UnsignedInt:
	    protocol.value.choice = MM_VALUE_CHOICE_UNSIGNEDINT;
	    protocol.value.un.unsignedInt =
		pRequest->un.setThreshold.value.un.unsignedInt;
	    break;

	case MM_ValueType_String:
	    /* String type not legal for thresholdolds */
	    goto cleanUp;
	    break;
	}

	break;

    case MM_RemOp_StartTimer:
	/*
	 * Start a timer.  When it expires, an event will be raised.
	 */

	/* Get the application entity instance name */
	if ((rc =
	     STR_attachZString(pRequest->
			       un.startTimer.pApplicationEntityInstanceName,
			       FALSE,
			       &protocol.hApplicationEntityInstanceName)) !=
	    Success)
	{
	    goto cleanUp;
	}
    
	/* Get the module name */
	if ((rc = STR_attachZString(pRequest->un.startTimer.pModuleName,
				    FALSE,
				    &protocol.hModuleName)) != Success)
	{
	    goto cleanUp;
	}
    
	/* Get the managable object name */
	if ((rc =
	     STR_attachZString(pRequest->un.startTimer.pManagableObjectName,
			       FALSE,
			       &protocol.hManagableObjectName)) != Success)
	{
	    goto cleanUp;
	}

	/* Get the number of milliseconds */
	protocol.milliseconds = pRequest->un.startTimer.milliseconds;

	break;

    case MM_RemOp_StopTimer:
	/*
	 * Stop a timer.
	 */

	/* Get the application entity instance name */
	if ((rc =
	     STR_attachZString(pRequest->
			       un.stopTimer.pApplicationEntityInstanceName,
			       FALSE,
			       &protocol.hApplicationEntityInstanceName)) !=
	    Success)
	{
	    goto cleanUp;
	}
    
	/* Get the module name */
	if ((rc = STR_attachZString(pRequest->un.stopTimer.pModuleName,
				    FALSE,
				    &protocol.hModuleName)) != Success)
	{
	    goto cleanUp;
	}
    
	/* Get the managable object name */
	if ((rc =
	     STR_attachZString(pRequest->un.stopTimer.pManagableObjectName,
			       FALSE,
			       &protocol.hManagableObjectName)) != Success)
	{
	    goto cleanUp;
	}
	break;

    case MM_RemOp_GetApplicationEntityList:
	/*
	 * Get the names of the application entity instances which have been
	 * registered.
	 */
	break;

    case MM_RemOp_GetModuleList:
	/*
	 * Get the names of the modules which have been registered with the
	 * specified application entity instance name.
	 */
	
	/* Get the application entity instance name */
	if ((rc =
	     STR_attachZString(pRequest->
			       un.getModuleList.pApplicationEntityInstanceName,
			       FALSE,
			       &protocol.hApplicationEntityInstanceName)) !=
	    Success)
	{
	    goto cleanUp;
	}
    
	break;

    case MM_RemOp_GetManagableObjectList:
	/*
	 * Get the names of the managable objects which have been
	 * registered with the specified module name.
	 */

	/* Get the application entity instance name */
	if ((rc =
	     STR_attachZString(pRequest->un.getManagableObjectList.
			       pApplicationEntityInstanceName, FALSE,
			       &protocol.hApplicationEntityInstanceName)) !=
	    Success)
	{
	    goto cleanUp;
	}
    
	/* Get the module name */
	if ((rc =
	     STR_attachZString(pRequest->un.getManagableObjectList.pModuleName,
			       FALSE,
			       &protocol.hModuleName)) != Success)
	{
	    goto cleanUp;
	}
    
	break;

    case MM_RemOp_GetManagableObjectInfo:
	/*
	 * Get the identification message associated with the
	 * specified managable object, and the managable object type.
	 * The identification message describes the managable object
	 * to a user of network manager software.
	 */

	/* Get the application entity instance name */
	if ((rc =
	     STR_attachZString(pRequest->un.getManagableObjectInfo.
			       pApplicationEntityInstanceName,
			       FALSE,
			       &protocol.hApplicationEntityInstanceName)) !=
	    Success)
	{
	    goto cleanUp;
	}
    
	/* Get the module name */
	if ((rc =
	     STR_attachZString(pRequest->un.getManagableObjectInfo.pModuleName,
			       FALSE,
			       &protocol.hModuleName)) != Success)
	{
	    goto cleanUp;
	}
    
	/* Get the managable object name */
	if ((rc =
	     STR_attachZString(pRequest->un.getManagableObjectInfo.
			       pManagableObjectName, FALSE,
			       &protocol.hManagableObjectName)) != Success)
	{
	    goto cleanUp;
	}

	break;

    case MM_RemOp_GetManagableObjectNotifyMask:
	/*
	 * Get the notification mask for the specified managable
	 * object, which indicates the classification and urgency of
	 * events raised on that object.
	 */

	/* Get the application entity instance name */
	if ((rc =
	     STR_attachZString(pRequest->un.getManagableObjectNotifyMask.
			       pApplicationEntityInstanceName, FALSE,
			       &protocol.hApplicationEntityInstanceName)) !=
	    Success)
	{
	    goto cleanUp;
	}
    
	/* Get the module name */
	if ((rc =
	     STR_attachZString(pRequest->un.getManagableObjectNotifyMask.
			       pModuleName,
			       FALSE,
			       &protocol.hModuleName)) != Success)
	{
	    goto cleanUp;
	}
    
	/* Get the managable object name */
	if ((rc =
	     STR_attachZString(pRequest->un.getManagableObjectNotifyMask.
			       pManagableObjectName, FALSE,
			       &protocol.hManagableObjectName)) != Success)
	{
	    goto cleanUp;
	}

	break;

    case MM_RemOp_SetManagableObjectNotifyMask:
	/*
	 * Set the notification mask for the specified managable
	 * object, which indicates the classification and urgency of
	 * events raised on that object.
	 */

	/* Get the application entity instance name */
	if ((rc =
	     STR_attachZString(pRequest->un.setManagableObjectNotifyMask.
			       pApplicationEntityInstanceName, FALSE,
			       &protocol.hApplicationEntityInstanceName)) !=
	    Success)
	{
	    goto cleanUp;
	}
    
	/* Get the module name */
	if ((rc =
	     STR_attachZString(pRequest->un.setManagableObjectNotifyMask.
			       pModuleName,
			       FALSE,
			       &protocol.hModuleName)) != Success)
	{
	    goto cleanUp;
	}
    
	/* Get the managable object name */
	if ((rc =
	     STR_attachZString(pRequest->un.setManagableObjectNotifyMask.
			       pManagableObjectName, FALSE,
			       &protocol.hManagableObjectName)) != Success)
	{
	    goto cleanUp;
	}

	/* Get the notify mask */
	protocol.notifyMask =
	    pRequest->un.setManagableObjectNotifyMask.notifyMask;

	break;

    case MM_RemOp_GetDestinationList:
	/*
	 * Get the names of the destinations which have been registered.
	 */

	/* Get the application entity instance name */
	if ((rc =
	     STR_attachZString(pRequest->un.getDestinationList.
			       pApplicationEntityInstanceName,
			       FALSE,
			       &protocol.hApplicationEntityInstanceName)) !=
	    Success)
	{
	    goto cleanUp;
	}
    
	break;

    case MM_RemOp_GetDestinationMasks:
	/*
	 * Obtain the set of notification and event types which should be sent
	 * to this destination.
	 */

	/* Get the application entity instance name */
	if ((rc =
	     STR_attachZString(pRequest->un.getDestinationMasks.
			       pApplicationEntityInstanceName,
			       FALSE,
			       &protocol.hApplicationEntityInstanceName)) !=
	    Success)
	{
	    goto cleanUp;
	}
    
	/* Get the destination  name */
	if ((rc =
	     STR_attachZString(pRequest->un.getDestinationMasks.
			       pDestinationName,
			       FALSE,
			       &protocol.hDestinationName)) != Success)
	{
	    goto cleanUp;
	}
    
	break;
    
    case MM_RemOp_SetDestinationMasks:
	/*
	 * Modify the set of notification types which should be sent
	 * to this destination.
	 */

	/* Get the application entity instance name */
	if ((rc =
	     STR_attachZString(pRequest->un.setDestinationMasks.
			       pApplicationEntityInstanceName, FALSE,
			       &protocol.hApplicationEntityInstanceName)) !=
	    Success)
	{
	    goto cleanUp;
	}
    
	/* Get the destination  name */
	if ((rc =
	     STR_attachZString(pRequest->un.setDestinationMasks.
			       pDestinationName,
			       FALSE,
			       &protocol.hDestinationName)) != Success)
	{
	    goto cleanUp;
	}
    
	/* Get the notify mask */
	protocol.notifyMask =
	    pRequest->un.setDestinationMasks.notifyMask;

	/* Get the event mask */
	protocol.eventMask =
	    pRequest->un.setDestinationMasks.eventMask;

	break;

    default:
	/* Avoid compiler warnings.  The remainder aren't handled here.  */
	break;
    }

    /* Format the PDU */
    if ((rc =
	 formatPdu(MM_RemOpType_Request,
		   pRequest->operationValue, &protocol, phBuf)) != Success)
    {
	goto cleanUp;
    }

    rc = Success;

    /* Clean up */
  cleanUp:
    mm_cleanUp(&protocol);

    return rc;
}


/*
 * MM_remoteOperationResult()
 *
 * Parse a result PDU, and convert it into a result structure.
 *
 * Parameters
 *
 *     hBuf --
 *       Buffer containing the result PDU to be parsed.
 *
 *     operationValue --
 *       Operation Value specifying the type of operation to be parsed.
 *
 *     ppResult --
 *       Pointer to a location in which a pointer to a result structure will
 *       be placed.  Upon successful completion of this function, the result
 *       structure will be filled in with the result data.
 *
 *       IT IS THE CALLER'S RESPONSIBILITY TO FREE THE RESULT STRUCTURE!
 */
ReturnCode
MM_remoteOperationResult(void * hBuf,
			 MM_RemoteOperationValue operationValue,
			 MM_Result ** ppResult)
{
    ReturnCode	    rc;
    MM_Protocol	    protocol;
    MM_Result *	    pResult = NULL;
    OS_Uint32	    pduLen;

    if (hBuf != NULL)
    {
	/* Zero out the event struture to make it easy to clean up later */
	OS_memSet((void *) &protocol, '\0', sizeof(MM_Protocol));

	/* Specify the maximum parse length. */
	pduLen = BUF_getRemainingBufferLength(hBuf);

	/* Parse the MM Protocol PDU */
	if ((rc = ASN_parse(ASN_EncodingRules_Basic,
			    &mm_protocols[operationValue].resultQ,
			    hBuf, &protocol,
			    &pduLen)) != Success)
	{
	    goto cleanUp;
	}
    }

    /*
     * Put the parameters into the result structure
     */

    switch(operationValue)
    {
    case MM_RemOp_GetValue:
	switch(protocol.value.choice)
	{
	case MM_VALUE_CHOICE_SIGNEDINT:
	    /* Allocate a result structure */
	    if ((pResult = OS_alloc(sizeof(MM_Result))) == NULL)
	    {
		rc = ResourceError;
		goto cleanUp;
	    }

	    /* Assign the parameters */
	    pResult->un.getValue.value.type = MM_ValueType_SignedInt;
	    pResult->un.getValue.value.un.signedInt =
		protocol.value.un.signedInt;
	    break;

	case MM_VALUE_CHOICE_UNSIGNEDINT:
	    /* Allocate a result structure */
	    if ((pResult = OS_alloc(sizeof(MM_Result))) == NULL)
	    {
		rc = ResourceError;
		goto cleanUp;
	    }

	    /* Assign the parameters */
	    pResult->un.getValue.value.type = MM_ValueType_UnsignedInt;
	    pResult->un.getValue.value.un.unsignedInt =
		protocol.value.un.unsignedInt;
	    break;

	case MM_VALUE_CHOICE_STRING:
	    /* Allocate a result structure */
	    if ((pResult = OS_alloc(sizeof(MM_Result))) == NULL)
	    {
		rc = ResourceError;
		goto cleanUp;
	    }
	
	    /* Assign the parameters. */
	    pResult->un.getValue.value.type = MM_ValueType_String;

	    /* Allocate space to copy the list */
	    if ((pResult->un.getValue.value.un.string =
		 OS_alloc(STR_getStringLength(protocol.value.un.string))) ==
		NULL)
	    {
		OS_free(pResult);
		pResult = NULL;
		rc = ResourceError;
		goto cleanUp;
	    }
	
	    /* Copy the string */
	    strcpy(pResult->un.getValue.value.un.string,
		   STR_stringStart(protocol.value.un.string));
	    break;
	}

	break;
	
    case MM_RemOp_SetValue:
	/* No result parameters */
	pResult = NULL;
	break;
	
    case MM_RemOp_GetThreshold:
	switch(protocol.value.choice)
	{
	case MM_VALUE_CHOICE_SIGNEDINT:
	    /* Allocate a result structure */
	    if ((pResult = OS_alloc(sizeof(MM_Result))) == NULL)
	    {
		rc = ResourceError;
		goto cleanUp;
	    }

	    /* Assign the parameters */
	    pResult->un.getThreshold.value.type = MM_ValueType_SignedInt;
	    pResult->un.getThreshold.value.un.signedInt =
		protocol.value.un.signedInt;
	    break;

	case MM_VALUE_CHOICE_UNSIGNEDINT:
	    /* Allocate a result structure */
	    if ((pResult = OS_alloc(sizeof(MM_Result))) == NULL)
	    {
		rc = ResourceError;
		goto cleanUp;
	    }

	    /* Assign the parameters */
	    pResult->un.getThreshold.value.type = MM_ValueType_UnsignedInt;
	    pResult->un.getThreshold.value.un.unsignedInt =
		protocol.value.un.unsignedInt;
	    break;

	default:
	    break;
	}

	break;
	
    case MM_RemOp_SetThreshold:
	/* No result parameters */
	pResult = NULL;
	break;
	
    case MM_RemOp_StartTimer:
	/* No result parameters */
	pResult = NULL;
	break;
	
    case MM_RemOp_StopTimer:
	/* No result parameters */
	pResult = NULL;
	break;
	
    case MM_RemOp_GetApplicationEntityList:
	/* Allocate a result structure */
	if ((pResult = OS_alloc(sizeof(MM_Result))) == NULL)
	{
	    rc = ResourceError;
	    goto cleanUp;
	}
	
	/* Allocate space to copy the list */
	if ((pResult->un.getApplicationEntityList.list =
	     OS_alloc(STR_getStringLength(protocol.hString))) == NULL)
	{
	    OS_free(pResult);
	    pResult = NULL;
	    rc = ResourceError;
	    goto cleanUp;
	}
	
	/* Copy the string */
	OS_copy(pResult->un.getApplicationEntityList.list,
		STR_stringStart(protocol.hString),
		STR_getStringLength(protocol.hString));
	break;
	
    case MM_RemOp_GetModuleList:
	/* Allocate a result structure */
	if ((pResult = OS_alloc(sizeof(MM_Result))) == NULL)
	{
	    rc = ResourceError;
	    goto cleanUp;
	}
	
	/* Allocate space to copy the list */
	if ((pResult->un.getModuleList.list =
	     OS_alloc(STR_getStringLength(protocol.hString))) == NULL)
	{
	    OS_free(pResult);
	    pResult = NULL;
	    rc = ResourceError;
	    goto cleanUp;
	}
	
	/* Copy the string */
	OS_copy(pResult->un.getModuleList.list,
		STR_stringStart(protocol.hString),
		STR_getStringLength(protocol.hString));
	break;
	
    case MM_RemOp_GetManagableObjectList:
	/* Allocate a result structure */
	if ((pResult = OS_alloc(sizeof(MM_Result))) == NULL)
	{
	    rc = ResourceError;
	    goto cleanUp;
	}
	
	/* Allocate space to copy the list */
	if ((pResult->un.getManagableObjectList.list =
	     OS_alloc(STR_getStringLength(protocol.hString))) == NULL)
	{
	    OS_free(pResult);
	    pResult = NULL;
	    rc = ResourceError;
	    goto cleanUp;
	}
	
	/* Copy the string */
	OS_copy(pResult->un.getManagableObjectList.list,
		STR_stringStart(protocol.hString),
		STR_getStringLength(protocol.hString));
	break;
	
    case MM_RemOp_GetManagableObjectInfo:
	/* Allocate a result structure */
	if ((pResult = OS_alloc(sizeof(MM_Result))) == NULL)
	{
	    rc = ResourceError;
	    goto cleanUp;
	}
	
	/* Assign the parameters */
	pResult->un.getManagableObjectInfo.objectType =
	    protocol.objectType;
	
	/* Allocate space to copy the list */
	if ((pResult->un.getManagableObjectInfo.description =
	     OS_alloc(STR_getStringLength(protocol.hString))) == NULL)
	{
	    OS_free(pResult);
	    pResult = NULL;
	    rc = ResourceError;
	    goto cleanUp;
	}
	
	/* Copy the string */
	strcpy(pResult->un.getManagableObjectInfo.description,
	       STR_stringStart(protocol.hString));
	break;
	
    case MM_RemOp_GetManagableObjectNotifyMask:
	/* Allocate a result structure */
	if ((pResult = OS_alloc(sizeof(MM_Result))) == NULL)
	{
	    rc = ResourceError;
	    goto cleanUp;
	}
	
	/* Assign the parameters */
	pResult->un.getManagableObjectNotifyMask.notifyMask =
	    protocol.notifyMask;
	
	break;
	
    case MM_RemOp_SetManagableObjectNotifyMask:
	/* No result parameters */
	pResult = NULL;
	break;
	
    case MM_RemOp_GetDestinationList:
	/* Allocate a result structure */
	if ((pResult = OS_alloc(sizeof(MM_Result))) == NULL)
	{
	    rc = ResourceError;
	    goto cleanUp;
	}
	
	/* Allocate space to copy the list */
	if ((pResult->un.getDestinationList.list =
	     OS_alloc(STR_getStringLength(protocol.hString))) == NULL)
	{
	    OS_free(pResult);
	    pResult = NULL;
	    rc = ResourceError;
	    goto cleanUp;
	}
	
	/* Copy the string */
	OS_copy(pResult->un.getDestinationList.list,
		STR_stringStart(protocol.hString),
		STR_getStringLength(protocol.hString));
	break;
	
    case MM_RemOp_GetDestinationMasks:
	/* Allocate a result structure */
	if ((pResult = OS_alloc(sizeof(MM_Result))) == NULL)
	{
	    rc = ResourceError;
	    goto cleanUp;
	}
	
	/* Assign the parameters */
	pResult->un.getDestinationMasks.notifyMask =
	    protocol.notifyMask;
	pResult->un.getDestinationMasks.eventMask =
	    protocol.eventMask;
	
	break;

    case MM_RemOp_SetDestinationMasks:
	/* No result parameters */
	pResult = NULL;
	break;

    default:
	/* Avoid compiler warnings.  The remainder aren't handled here. */
	break;
    }

    /* Now that we've (possibly) allocated a result structure... */
    if (pResult != NULL)
    {
	/* ... specify the operation value in it. */
	pResult->operationValue = operationValue;
    }

    /* Give 'em what they came for */
    *ppResult = pResult;

    /* We've completed successfully. */
    rc = Success;

  cleanUp:
    mm_cleanUp(&protocol);

    return rc;
}


/*
 * MM_remoteOperationError()
 *
 * Parse an error PDU, and provide the error code.
 *
 * Parameters
 *
 *     hBuf --
 *       Buffer containing the error PDU to be parsed.
 *
 *     pRc --
 *       Pointer to a location in which the error (return) code will
 *       be placed.
 */
void
MM_remoteOperationError(void * hBuf,
			ReturnCode * pRc)
{
    int 	    i;
    OS_Uint8	    octet;
    OS_Uint16	    errorVal = 0;

    /* Make sure we got exactly 2 bytes, containing the error code */
    if (BUF_getBufferLength(hBuf) != 2)
    {
	/* It's not a properly formed error pdu */
	*pRc = Fail;
    }

    /* Parse the error code */
    for (i=0; i<2; i++)
    {
	/* Get the next octet */
	if ((*pRc = BUF_getOctet(hBuf, &octet)) != Success)
	{
	    return;
	}

	/* Shift it into our return value */
	errorVal = (errorVal << 8) | octet;
    }

    /* Give 'em what they came for */
    *pRc = (ReturnCode) errorVal;
}


static ReturnCode
formatPdu(MM_RemoteOperationType operationType,
	     MM_RemoteOperationValue operationValue,
	     MM_Protocol * p,
	     void ** phBuf)
{
    ReturnCode	    rc;
    void *	    hBuf = *phBuf;
    OS_Uint32	    pduLen;

    /* If no buffer was provided, allocate one */
    if (hBuf == NULL)
    {
	if ((rc = BUF_alloc(0, &hBuf)) != Success)
	{
	    return FAIL_RC(rc, ("formatPdu(): could not allocate buffer"));
	}

	*phBuf = hBuf;
    }

    /* Format the PDU. */
    if ((rc = ASN_format(ASN_EncodingRules_Basic,
			 &mm_protocols[operationValue].requestQ,
			 hBuf,
			 p,
			 &pduLen)) != Success)
    {
	OS_free(hBuf);
	return FAIL_RC(rc, ("formatPdu(): ASN_format\n"));
    }

    return Success;
}


void
mm_cleanUp(MM_Protocol * p)
{
    if (p->hApplicationEntityInstanceName != NULL)
    {
	STR_free(p->hApplicationEntityInstanceName);
    }

    if (p->hModuleName != NULL)
    {
	STR_free(p->hModuleName);
    }

    if (p->hManagableObjectName != NULL)
    {
	STR_free(p->hManagableObjectName);
    }

    if (p->hDestinationName != NULL)
    {
	STR_free(p->hDestinationName);
    }

    if (p->hString != NULL)
    {
	STR_free(p->hString);
    }
}
