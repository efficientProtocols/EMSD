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
 * Module Management Manager functionality
 */


#include "estd.h"
#include "rc.h"
#include "queue.h"
#include "tmr.h"
#include "buf.h"
#include "mm.h"
#include "mmlocal.h"

/*
 * Global and Static variables
 */
TM_ModuleCB *	mm_hModCB;


/*
 * Forward Declarations
 */
static ReturnCode (* pfRemoteOperationSend)(void ** phOperation,
					    MM_RemoteOperationType opType,
					    MM_RemoteOperationValue opValue,
					    void ** phBuf) = NULL;

static ReturnCode (* pfRemoteOperationRecv)(void ** phOperation,
					    MM_RemoteOperationType * pOpType,
					    MM_RemoteOperationValue * pOpValue,
					    void ** phBuf) = NULL;

static ReturnCode
synchronousOperation(MM_Request * pRequest,
		     MM_Result ** ppResult);




/*
 * MM_managerInit()
 *
 * Initialize the Module Management manager.
 */
ReturnCode
MM_managerInit(ReturnCode (* pfSend)(void ** phOperation,
				     MM_RemoteOperationType operationType,
				     MM_RemoteOperationValue operationValue,
				     void ** phBuf),
	       ReturnCode (* pfRecv)(void ** phOperation,
				     MM_RemoteOperationType * pOperationType,
				     MM_RemoteOperationValue * pOperationValue,
				     void ** phBuf))
{
    ReturnCode			rc;

    /* Create a trace module handle */
    if (TM_OPEN(mm_hModCB, "MM") == NULL)
    {
	return FAIL_RC(ResourceError, ("TM_OPEN() failed\n"));
    }

    /* Initialize the ASN module and ASN Compile trees */
    if ((rc = mm_asnInit()) != Success)
    {
	return FAIL_RC(rc, ("MM_agentInit(): mm_asnInit()\n"));
    }

    /* Save the pointers to remote operation functions */
    pfRemoteOperationSend = pfSend;
    pfRemoteOperationRecv = pfRecv;

    return Success;
}


/*
 * MM_getValueByName()
 *
 * Get the current value of a managable object.
 *
 * Parameters:
 *
 *   pApplicationEntityInstanceName --
 *     Name of the application instance in which the managable entity
 *     resides.
 *
 *   pModuleName --
 *     Name of the module in which the manangable entity resides.
 *
 *   pManagableObjectName --
 *     Name of the managable object for which the value is desired.
 *
 *   pValue --
 *     Pointer to the location where the current value of the specified
 *     managable object is to be placed.  It is up to the caller to provide a
 *     pointer to the correct type of variable into which the value will be
 *     placed.
 *
 *
 * NOTE: The value of a managable object of type MM_ObjectType_String is a
 *       pointer to a COPY of the string maintained within the managable
 *       object.  It is the caller's responsibility to free this COPY, with
 *       OS_free(), when it is no longer required.
 */
ReturnCode
MM_getValueByName(char * pApplicationEntityInstanceName,
		  char * pModuleName,
		  char * pManagableObjectName,
		  MM_Value * pValue)
{
    ReturnCode			rc;
    MM_Request			request;
    MM_Result *			pResult;

    /* Fill in the request structure */
    request.operationValue = MM_RemOp_GetValue;

    request.un.getValue.pApplicationEntityInstanceName =
	pApplicationEntityInstanceName;
    request.un.getValue.pModuleName = pModuleName;
    request.un.getValue.pManagableObjectName = pManagableObjectName;

    /* Issue the operation request and wait for a response */
    if ((rc = synchronousOperation(&request, &pResult)) != Success)
    {
	/* We've had a local failure, or got back an error response */
	return rc;
    }

    /* Give 'em what they came for */
    *pValue = pResult->un.getValue.value;

    /* Free the result structure */
    if (pResult != NULL)
    {
	OS_free(pResult);
    }

    return Success;
}

/*
 * MM_setValueByName()
 *
 * Get the current value of a managable object.
 *
 * Parameters:
 *
 *   pApplicationEntityInstanceName --
 *     Name of the application instance in which the managable entity
 *     resides.
 *
 *   pModuleName --
 *     Name of the module in which the manangable entity resides.
 *
 *   pManagableObjectName --
 *     Name of the managable object for which the value is to be
 *     modified.
 *
 *   pValue --
 *     Pointer to the new value for this managable object.  It is up
 *     to the caller to provide a pointer to the correct type of
 *     variable into which the value will be placed.
 */
ReturnCode
MM_setValueByName(void * pApplicationEntityInstanceName,
		  char * pModuleName,
		  char * pManagableObjectName,
		  MM_Value * pValue)
{
    ReturnCode			rc;
    MM_Request			request;
    MM_Result *			pResult;

    /* Fill in the request structure */
    request.operationValue = MM_RemOp_SetValue;

    request.un.setValue.pApplicationEntityInstanceName =
	pApplicationEntityInstanceName;
    request.un.setValue.pModuleName = pModuleName;
    request.un.setValue.pManagableObjectName = pManagableObjectName;
    request.un.setValue.value = *pValue;

    /* Issue the operation request and wait for a response */
    if ((rc = synchronousOperation(&request, &pResult)) != Success)
    {
	/* We've had a local failure, or got back an error response */
	return rc;
    }

    /* Free the result structure */
    if (pResult != NULL)
    {
	OS_free(pResult);
    }

    return Success;
}



/*
 * MM_getThresholdByName()
 *
 * Get the current threshold of a managable object.
 *
 * Parameters:
 *
 *   pApplicationEntityInstanceName --
 *     Name of the application instance in which the managable entity
 *     resides.
 *
 *   pModuleName --
 *     Name of the module in which the manangable entity resides.
 *
 *   pManagableObjectName --
 *     Name of the managable object for which the threshold is desired.
 *
 *   thresholdType --
 *     Indication of whether the threshold to be set is a Maximum threshold or
 *     a Minimum threshold.
 *
 *   pValue --
 *     Pointer to the location where the current threshold of the
 *     specified managable object is to be placed.  It is up to the
 *     caller to provide a pointer to the correct type of variable
 *     into which the threshold will be placed.
 */
ReturnCode
MM_getThresholdByName(char * pApplicationEntityInstanceName,
		      char * pModuleName,
		      char * pManagableObjectName,
		      MM_ThresholdType thresholdType,
		      MM_Value * pValue)
{
    ReturnCode			rc;
    MM_Request			request;
    MM_Result *			pResult;

    /* Fill in the request structure */
    request.operationValue = MM_RemOp_GetThreshold;

    request.un.getThreshold.pApplicationEntityInstanceName =
	pApplicationEntityInstanceName;
    request.un.getThreshold.pModuleName = pModuleName;
    request.un.getThreshold.pManagableObjectName = pManagableObjectName;
    request.un.getThreshold.thresholdType = thresholdType;

    /* Issue the operation request and wait for a response */
    if ((rc = synchronousOperation(&request, &pResult)) != Success)
    {
	/* We've had a local failure, or got back an error response */
	return rc;
    }

    /* Give 'em what they came for */
    *pValue = pResult->un.getThreshold.value;

    /* Free the result structure */
    if (pResult != NULL)
    {
	OS_free(pResult);
    }

    return Success;
}

/*
 * MM_setThresholdByName()
 *
 * Get the current threshold of a managable object.
 *
 * Parameters:
 *
 *   pApplicationEntityInstanceName --
 *     Name of the application instance in which the managable entity
 *     resides.
 *
 *   pModuleName --
 *     Name of the module in which the manangable entity resides.
 *
 *   pManagableObjectName --
 *     Name of the managable object for which the threshold is to be
 *     modified.
 *
 *   thresholdType --
 *     Indication of whether the threshold to be set is a Maximum threshold or
 *     a Minimum threshold.
 *
 *   pValue --
 *     Pointer to the new threshold for this managable object.  It is up
 *     to the caller to provide a pointer to the correct type of
 *     variable into which the threshold will be placed.
 */
ReturnCode
MM_setThresholdByName(void * pApplicationEntityInstanceName,
		      char * pModuleName,
		      char * pManagableObjectName,
		      MM_ThresholdType thresholdType,
		      MM_Value * pValue)
{
    ReturnCode			rc;
    MM_Request			request;
    MM_Result *			pResult;

    /* Fill in the request structure */
    request.operationValue = MM_RemOp_SetThreshold;

    request.un.setThreshold.pApplicationEntityInstanceName =
	pApplicationEntityInstanceName;
    request.un.setThreshold.pModuleName = pModuleName;
    request.un.setThreshold.pManagableObjectName = pManagableObjectName;
    request.un.setThreshold.thresholdType = thresholdType;
    request.un.setThreshold.value = *pValue;

    /* Issue the operation request and wait for a response */
    if ((rc = synchronousOperation(&request, &pResult)) != Success)
    {
	/* We've had a local failure, or got back an error response */
	return rc;
    }

    /* Free the result structure */
    if (pResult != NULL)
    {
	OS_free(pResult);
    }

    return Success;
}



/*
 * MM_startTimerByName()
 *
 * Start a timer.  When it expires, an event will be raised.
 *
 *
 * Parameters:
 *
 *   pApplicationEntityInstanceName --
 *     Name of the application instance in which the managable entity
 *     resides.
 *
 *   pModuleName --
 *     Name of the module in which the manangable entity resides.
 *
 *   pManagableObjectName --
 *     Name of the managable object for which the value is desired.
 *
 *   milliseconds --
 *     Number of milliseconds before the timer should expire.
 */
ReturnCode
MM_startTimerByName(char * pApplicationEntityInstanceName,
		    char * pModuleName,
		    char * pManagableObjectName,
		    OS_Uint32 milliseconds)
{
    ReturnCode			rc;
    MM_Request			request;
    MM_Result *			pResult;

    /* Fill in the request structure */
    request.operationValue = MM_RemOp_StartTimer;

    request.un.startTimer.pApplicationEntityInstanceName =
	pApplicationEntityInstanceName;
    request.un.startTimer.pModuleName = pModuleName;
    request.un.startTimer.pManagableObjectName = pManagableObjectName;
    request.un.startTimer.milliseconds = milliseconds;

    /* Issue the operation request and wait for a response */
    if ((rc = synchronousOperation(&request, &pResult)) != Success)
    {
	/* We've had a local failure, or got back an error response */
	return rc;
    }

    /* Free the result structure */
    if (pResult != NULL)
    {
	OS_free(pResult);
    }

    return Success;
}


/*
 * MM_stopTimerByName()
 *
 * Stop a timer.
 *
 *
 * Parameters:
 *
 *   pApplicationEntityInstanceName --
 *     Name of the application instance in which the managable entity
 *     resides.
 *
 *   pModuleName --
 *     Name of the module in which the manangable entity resides.
 *
 *   pManagableObjectName --
 *     Name of the managable object for which the value is desired.
 */
ReturnCode
MM_stopTimerByName(char * pApplicationEntityInstanceName,
		   char * pModuleName,
		   char * pManagableObjectName)
{
    ReturnCode			rc;
    MM_Request			request;
    MM_Result *			pResult;

    /* Fill in the request structure */
    request.operationValue = MM_RemOp_StopTimer;

    request.un.stopTimer.pApplicationEntityInstanceName =
	pApplicationEntityInstanceName;
    request.un.stopTimer.pModuleName = pModuleName;
    request.un.stopTimer.pManagableObjectName = pManagableObjectName;

    /* Issue the operation request and wait for a response */
    if ((rc = synchronousOperation(&request, &pResult)) != Success)
    {
	/* We've had a local failure, or got back an error response */
	return rc;
    }

    /* Free the result structure */
    if (pResult != NULL)
    {
	OS_free(pResult);
    }

    return Success;
}


/*
 * MM_getApplicationEntityInstanceList()
 *
 * Get the names of the application entity instances which have been
 * registered.
 *
 * Notes:
 *
 *   1. The name of each application entity instance is terminated, in the
 *      buffer, with a null character ('\0').  Following the last application
 *      entity instance name is an additional (EXTRA) null character to
 *      terminate the entire list.
 *
 *   2. The returned buffer is allocated by
 *      MM_getApplicationEntityInstanceList() on each call, and it is the
 *      caller's responsibility to free the buffer with OS_free() when the
 *      buffer is no longer needed.
 *
 * Parameters:
 *
 *   ppBuf --
 *     Pointer to the location in which to place a pointer to an allocated
 *     buffer containing the list of modules.
 */
ReturnCode
MM_getApplicationEntityInstanceList(char ** ppBuf)
{
    ReturnCode			rc;
    MM_Request			request;
    MM_Result *			pResult;

    /* Fill in the request structure */
    request.operationValue = MM_RemOp_GetApplicationEntityList;

    /* Issue the operation request and wait for a response */
    if ((rc = synchronousOperation(&request, &pResult)) != Success)
    {
	/* We've had a local failure, or got back an error response */
	return rc;
    }

    /* Give 'em what they came for */
    *ppBuf = pResult->un.getApplicationEntityList.list;

    /* Free the result structure */
    if (pResult != NULL)
    {
	OS_free(pResult);
    }

    return Success;
}


/*
 * MM_getModuleList()
 *
 * Get the names of the modules which have been registered with the specified
 * application entity instance name.
 *
 * Notes:
 *
 *   1. The name of each module is terminated, in the buffer, with a null
 *      character ('\0').  Following the last module name is an
 *      additional (EXTRA) null character to terminate the entire
 *      list.
 *
 *   2. The returned buffer is allocated by MM_getModuleList() on each call,
 *      and it is the caller's responsibility to free the buffer with
 *      OS_free() when the buffer is no longer needed.
 *
 * Parameters:
 *
 *   pApplicationEntityInstanceName --
 *     Name of the application entity instance in which the list of modules is
 *     to be determined.
 *
 *   ppBuf --
 *     Pointer to the location in which to place a pointer to an allocated
 *     buffer containing the list of modules.
 */
ReturnCode
MM_getModuleList(char * pApplicationEntityInstanceName,
		 char ** ppBuf)
{
    ReturnCode			rc;
    MM_Request			request;
    MM_Result *			pResult;

    /* Fill in the request structure */
    request.operationValue = MM_RemOp_GetModuleList;
    request.un.getModuleList.pApplicationEntityInstanceName =
	pApplicationEntityInstanceName;

    /* Issue the operation request and wait for a response */
    if ((rc = synchronousOperation(&request, &pResult)) != Success)
    {
	/* We've had a local failure, or got back an error response */
	return rc;
    }

    /* Give 'em what they came for */
    *ppBuf = pResult->un.getModuleList.list;

    /* Free the result structure */
    if (pResult != NULL)
    {
	OS_free(pResult);
    }

    return Success;
}


/*
 * MM_getManagableObjectList()
 *
 * Get the names of the managable objects which have been registered
 * with the specified module name.
 *
 * Notes:
 *
 *   1. The name of each module is terminated, in the buffer, with a null
 *      character ('\0').  Following the last module name is an
 *      additional (EXTRA) null character to terminate the entire
 *      list.
 *
 *   2. The returned buffer is allocated by MM_getManagableObjectList() on
 *      each call, and it is the caller's responsibility to free the buffer
 *      with OS_free() when the buffer is no longer needed.
 *
 * Parameters:
 *
 *   pApplicationEntityInstanceName --
 *     Name of the application entity instance in which the list of managable
 *     objects is to be determined.
 *
 *   pModule Name --
 *     Name of the module in which the list of managable objects is to be
 *     determined.
 *
 *   ppBuf --
 *     Pointer to the location in which to place a pointer to an allocated
 *     buffer containing the list of managable objects.
 */
ReturnCode
MM_getManagableObjectList(char * pApplicationEntityInstanceName,
			  char * pModuleName,
			  char ** ppBuf)
{
    ReturnCode			rc;
    MM_Request			request;
    MM_Result *			pResult;

    /* Fill in the request structure */
    request.operationValue = MM_RemOp_GetManagableObjectList;
    request.un.getManagableObjectList.pApplicationEntityInstanceName =
	pApplicationEntityInstanceName;
    request.un.getManagableObjectList.pModuleName = pModuleName;

    /* Issue the operation request and wait for a response */
    if ((rc = synchronousOperation(&request, &pResult)) != Success)
    {
	/* We've had a local failure, or got back an error response */
	return rc;
    }

    /* Give 'em what they came for */
    *ppBuf = pResult->un.getManagableObjectList.list;

    /* Free the result structure */
    if (pResult != NULL)
    {
	OS_free(pResult);
    }

    return Success;
}



/*
 * MM_getManagableObjectInfo()
 *
 * Get the identification message associated with the specified
 * managable object, and the managable object type.  The
 * identification message describes the managable object to a user of
 * network manager software.
 *
 * Notes:
 *
 *   The returned buffer is allocated by MM_getManagableObjectInfo()
 *   on each call, and it is the caller's responsibility to free the buffer
 *   with OS_free() when the buffer is no longer needed.
 *
 * Parameters:
 *
 *   pApplicationEntityInstanceName --
 *     Name of the application entity instance in which the managable object
 *     resides.
 *
 *   pModuleName --
 *     Name of the module in which the managable object resides.
 *
 *   pManagableObjectName --
 *     Name of the managable object for which the identification message is to
 *     be determined.
 *
 *   pObjectType --
 *     Pointer to the location in which to place the type of object
 *     represented by the specified managable object name.
 *
 *   ppBuf --
 *     Pointer to the location in which to place a pointer to an allocated
 *     buffer containing the identification message.
 */
ReturnCode
MM_getManagableObjectInfo(char * pApplicationEntityInstanceName,
			  char * pModuleName,
			  char * pManagableObjectName,
			  MM_ManagableObjectType * pObjectType,
			  char ** ppBuf)
{
    ReturnCode			rc;
    MM_Request			request;
    MM_Result *			pResult;

    /* Fill in the request structure */
    request.operationValue = MM_RemOp_GetManagableObjectInfo;

    request.un.getManagableObjectInfo.pApplicationEntityInstanceName =
	pApplicationEntityInstanceName;
    request.un.getManagableObjectInfo.pModuleName = pModuleName;
    request.un.getManagableObjectInfo.pManagableObjectName =
	pManagableObjectName;

    /* Issue the operation request and wait for a response */
    if ((rc = synchronousOperation(&request, &pResult)) != Success)
    {
	/* We've had a local failure, or got back an error response */
	return rc;
    }

    /* Give 'em what they came for */
    *pObjectType = pResult->un.getManagableObjectInfo.objectType;

    *ppBuf = pResult->un.getManagableObjectInfo.description;

    /* Free the result structure */
    if (pResult != NULL)
    {
	OS_free(pResult);
    }

    return Success;
}


/*
 * MM_getManagableObjectNotifyMaskByName()
 *
 * Get the notification mask for the specified managable object, which
 * indicates the classification and urgency of events raised on that
 * object.
 *
 *
 * Parameters:
 *
 *   pApplicationEntityInstanceName --
 *     Name of the application entity instance in which the managable object
 *     resides.
 *
 *   pModuleName --
 *     Name of the module in which the managable object resides.
 *
 *   pManagableObjectName --
 *     Name of the managable object for which the identification message is to
 *     be determined.
 *
 *   pNotifyMask --
 *     Pointer to a location in which to place the bit mask indicating
 *     which notification levels are to be sent to this destination.
 */
ReturnCode
MM_getManagableObjectNotifyMaskByName(char * pApplicationEntityInstanceName,
				      char * pModuleName,
				      char * pManagableObjectName,
				      OS_Uint32 * pNotifyMask)
{
    ReturnCode			rc;
    MM_Request			request;
    MM_Result *			pResult;

    /* Fill in the request structure */
    request.operationValue = MM_RemOp_GetManagableObjectNotifyMask;

    request.un.getManagableObjectNotifyMask.pApplicationEntityInstanceName =
	pApplicationEntityInstanceName;
    request.un.getManagableObjectNotifyMask.pModuleName = pModuleName;
    request.un.getManagableObjectNotifyMask.pManagableObjectName =
	pManagableObjectName;

    /* Issue the operation request and wait for a response */
    if ((rc = synchronousOperation(&request, &pResult)) != Success)
    {
	/* We've had a local failure, or got back an error response */
	return rc;
    }

    /* Give 'em what they came for */
    *pNotifyMask = pResult->un.getManagableObjectNotifyMask.notifyMask;

    /* Free the result structure */
    if (pResult != NULL)
    {
	OS_free(pResult);
    }

    return Success;
}


/*
 * MM_setManagableObjectNotifyMaskByName()
 *
 * Set the notification mask for the specified managable object, which
 * indicates the classification and urgency of events raised on that
 * object.
 *
 *
 * Parameters:
 *
 *   pApplicationEntityInstanceName --
 *     Name of the application entity instance in which the managable object
 *     resides.
 *
 *   pModuleName --
 *     Name of the module in which the managable object resides.
 *
 *   pManagableObjectName --
 *     Name of the managable object for which the identification message is to
 *     be determined.
 *
 *   notifyMask --
 *     New bit mask indicating which notification levels are to be
 *     sent to this destination.
 */
ReturnCode
MM_setManagableObjectNotifyMaskByName(char * pApplicationEntityInstanceName,
				      char * pModuleName,
				      char * pManagableObjectName,
				      OS_Uint32 notifyMask)
{
    ReturnCode			rc;
    MM_Request			request;
    MM_Result *			pResult;

    /* Fill in the request structure */
    request.operationValue = MM_RemOp_SetManagableObjectNotifyMask;

    request.un.setManagableObjectNotifyMask.pApplicationEntityInstanceName =
	pApplicationEntityInstanceName;
    request.un.setManagableObjectNotifyMask.pModuleName = pModuleName;
    request.un.setManagableObjectNotifyMask.pManagableObjectName =
	pManagableObjectName;
    request.un.setManagableObjectNotifyMask.notifyMask = notifyMask;

    /* Issue the operation request and wait for a response */
    if ((rc = synchronousOperation(&request, &pResult)) != Success)
    {
	/* We've had a local failure, or got back an error response */
	return rc;
    }

    /* Free the result structure */
    if (pResult != NULL)
    {
	OS_free(pResult);
    }

    return Success;
}



/*
 * MM_getDestinationList()
 *
 * Get the names of the destinations which have been registered.
 *
 * Notes:
 *
 *   1. The name of each destination is terminated, in the buffer, with a null
 *      character ('\0').  Following the last destination name is an
 *      additional (EXTRA) null character to terminate the entire list.
 *
 *   2. The returned buffer is allocated by MM_getDestinationList() on
 *      each call, and it is the caller's responsibility to free the buffer
 *      with OS_free() when the buffer is no longer needed.
 *
 * Parameters:
 *
 *   pApplicationEntityInstanceName --
 *     Name of the application entity instance in which the list of managable
 *     objects is to be determined.
 *
 *   ppBuf --
 *     Pointer to the location in which to place a pointer to an allocated
 *     buffer containing the list of destinations.
 */
ReturnCode
MM_getDestinationList(char * pApplicationEntityInstanceName,
		      char ** ppBuf)
{
    ReturnCode			rc;
    MM_Request			request;
    MM_Result *			pResult;

    /* Fill in the request structure */
    request.operationValue = MM_RemOp_GetDestinationList;

    request.un.getDestinationList.pApplicationEntityInstanceName =
	pApplicationEntityInstanceName;

    /* Issue the operation request and wait for a response */
    if ((rc = synchronousOperation(&request, &pResult)) != Success)
    {
	/* We've had a local failure, or got back an error response */
	return rc;
    }

    /* Give 'em what they came for */
    *ppBuf = pResult->un.getDestinationList.list;

    /* Free the result structure */
    if (pResult != NULL)
    {
	OS_free(pResult);
    }

    return Success;
}



/*
 * MM_getDestinationMasksByName()
 *
 * Obtain the set of notification and event types which should be sent
 * to this destination.
 *
 * Parameters:
 *
 *   pApplicationEntityInstanceName --
 *     Name of the application instance in which the managable entity
 *     resides.
 *
 *   pDestinationName --
 *     Name of the destination from which the masks are to be retrieved.
 *
 *   pNotifyMask --
 *     Pointer to a location in which to place the bit mask indicating
 *     which notification levels are to be sent to this destination.
 *
 *   pEventMask --
 *     Pointer to a location in which to place the bit mask indicating
 *     which events are to be sent to this destination.
 */
ReturnCode
MM_getDestinationMasksByName(char * pApplicationEntityInstanceName,
			     char * pDestinationName,
			     OS_Uint32 * pNotifyMask,
			     OS_Uint32 * pEventMask)
{
    ReturnCode			rc;
    MM_Request			request;
    MM_Result *			pResult;

    /* Fill in the request structure */
    request.operationValue = MM_RemOp_GetDestinationMasks;

    request.un.getDestinationMasks.pApplicationEntityInstanceName =
	pApplicationEntityInstanceName;
    request.un.getDestinationMasks.pDestinationName = pDestinationName;

    /* Issue the operation request and wait for a response */
    if ((rc = synchronousOperation(&request, &pResult)) != Success)
    {
	/* We've had a local failure, or got back an error response */
	return rc;
    }

    /* Give 'em what they came for */
    *pNotifyMask = pResult->un.getDestinationMasks.notifyMask;
    *pEventMask = pResult->un.getDestinationMasks.eventMask;

    /* Free the result structure */
    if (pResult != NULL)
    {
	OS_free(pResult);
    }

    return Success;
}


/*
 * MM_setDestinationMasksByName()
 *
 * Modify the set of notification types which should be sent to this
 * destination.
 *
 * Parameters:
 *
 *   pApplicationEntityInstanceName --
 *     Name of the application instance in which the managable entity
 *     resides.
 *
 *   pDestinationName --
 *     Name of the destination in which the masks are to be modified.
 *
 *   newNotifyMask --
 *     New bit mask indicating which notification levels are to be sent to
 *     this destination.
 *
 *   newEventMask --
 *     New bit mask indicating which events are to be sent to this
 *     destination.
 */
ReturnCode
MM_setDestinationMasksByName(char * pApplicationEntityInstanceName,
			     char * pDestinationName,
			     OS_Uint32 newNotifyMask,
			     OS_Uint32 newEventMask)
{
    ReturnCode			rc;
    MM_Request			request;
    MM_Result *			pResult;

    /* Fill in the request structure */
    request.operationValue = MM_RemOp_GetDestinationMasks;

    request.un.setDestinationMasks.pApplicationEntityInstanceName =
	pApplicationEntityInstanceName;
    request.un.setDestinationMasks.pDestinationName = pDestinationName;
    request.un.setDestinationMasks.notifyMask = newNotifyMask;
    request.un.setDestinationMasks.eventMask = newEventMask;

    /* Issue the operation request and wait for a response */
    if ((rc = synchronousOperation(&request, &pResult)) != Success)
    {
	/* We've had a local failure, or got back an error response */
	return rc;
    }

    /* Free the result structure */
    if (pResult != NULL)
    {
	OS_free(pResult);
    }

    return Success;
}





/*
 * MM_remoteOperationEvent()
 *
 * Parse an Event PDU and provide event parameters.
 */
ReturnCode
MM_remoteOperationEvent(MM_RemoteOperationValue operationValue,
			void * hBuf,
			MM_Event ** ppEvent)
{
    ReturnCode	    rc;
    OS_Uint32	    pduLen;
    MM_Protocol	    protocol;
    MM_Event *	    pEvent = NULL;
    MM_EventType    eventType = MM_EVENT_DECODE(operationValue);

#define	APL_LEN								\
    (STR_getStringLength(protocol.hApplicationEntityInstanceName) + 1)
	
#define	MOD_LEN 					\
	(STR_getStringLength(protocol.hModuleName) + 1)
	
#define	OBJ_LEN						\
	(STR_getStringLength(protocol.hManagableObjectName) + 1)

#define	ID_LEN	(APL_LEN + MOD_LEN + OBJ_LEN)

    if (hBuf != NULL)
    {
	/* Zero out the event struture to make it easy to clean up later */
	OS_memSet((void *) &protocol, '\0', sizeof(MM_Protocol));

	/* Specify the maximum parse length.  Skip invokeId, opType, opValue */
	pduLen = BUF_getBufferLength(hBuf) - 3;

	/* Parse the MM Protocol PDU */
	if ((rc = ASN_parse(ASN_EncodingRules_Basic,
			    &mm_eventProtocols[eventType].eventQ,
			    hBuf, &protocol,
			    &pduLen)) != Success)
	{
	    goto cleanUp;
	}
    }

    /*
     * Put the parameters into the event structure
     */
    switch(operationValue)
    {
    case MM_RemOp_MaxThresholdExceededSigned:
	if (protocol.value.choice != MM_VALUE_CHOICE_SIGNEDINT)
	{
	    rc = MM_RC_WrongValueType;
	    goto cleanUp;
	}
	
	/* Allocate an event structure */
	if ((pEvent = OS_alloc(sizeof(MM_Event) + ID_LEN)) == NULL)
	{
	    rc = ResourceError;
	    goto cleanUp;
	}

	/* Assign the parameters */
	pEvent->un.maxThresholdExceededSigned.threshold =
	    protocol.value.un.signedInt;
	pEvent->un.maxThresholdExceededSigned.newValue =
	    protocol.value2.un.signedInt;

	break;

    case MM_RemOp_MaxThresholdExceededUnsigned:
	if (protocol.value.choice != MM_VALUE_CHOICE_UNSIGNEDINT)
	{
	    rc = MM_RC_WrongValueType;
	    goto cleanUp;
	}
	
	/* Allocate an event structure */
	if ((pEvent = OS_alloc(sizeof(MM_Event) + ID_LEN)) == NULL)
	{
	    rc = ResourceError;
	    goto cleanUp;
	}

	/* Assign the parameters */
	pEvent->un.maxThresholdExceededUnsigned.threshold =
	    protocol.value.un.unsignedInt;
	pEvent->un.maxThresholdExceededUnsigned.newValue =
	    protocol.value2.un.unsignedInt;

	break;

    case MM_RemOp_MinThresholdExceededSigned:
	if (protocol.value.choice != MM_VALUE_CHOICE_SIGNEDINT)
	{
	    rc = MM_RC_WrongValueType;
	    goto cleanUp;
	}
	
	/* Allocate an event structure */
	if ((pEvent = OS_alloc(sizeof(MM_Event) + ID_LEN)) == NULL)
	{
	    rc = ResourceError;
	    goto cleanUp;
	}

	/* Assign the parameters */
	pEvent->un.minThresholdExceededSigned.threshold =
	    protocol.value.un.signedInt;
	pEvent->un.minThresholdExceededSigned.newValue =
	    protocol.value2.un.signedInt;

	break;

    case MM_RemOp_MinThresholdExceededUnsigned:
	if (protocol.value.choice != MM_VALUE_CHOICE_UNSIGNEDINT)
	{
	    rc = MM_RC_WrongValueType;
	    goto cleanUp;
	}
	
	/* Allocate an event structure */
	if ((pEvent = OS_alloc(sizeof(MM_Event) + ID_LEN)) == NULL)
	{
	    rc = ResourceError;
	    goto cleanUp;
	}

	/* Assign the parameters */
	pEvent->un.minThresholdExceededUnsigned.threshold =
	    protocol.value.un.unsignedInt;
	pEvent->un.minThresholdExceededUnsigned.newValue =
	    protocol.value2.un.unsignedInt;

	break;

    case MM_RemOp_MaxThresholdReenteredSigned:
	if (protocol.value.choice != MM_VALUE_CHOICE_SIGNEDINT)
	{
	    rc = MM_RC_WrongValueType;
	    goto cleanUp;
	}
	
	/* Allocate an event structure */
	if ((pEvent = OS_alloc(sizeof(MM_Event) + ID_LEN)) == NULL)
	{
	    rc = ResourceError;
	    goto cleanUp;
	}

	/* Assign the parameters */
	pEvent->un.maxThresholdReenteredSigned.threshold =
	    protocol.value.un.signedInt;
	pEvent->un.maxThresholdReenteredSigned.newValue =
	    protocol.value2.un.signedInt;

	break;

    case MM_RemOp_MaxThresholdReenteredUnsigned:
	if (protocol.value.choice != MM_VALUE_CHOICE_UNSIGNEDINT)
	{
	    rc = MM_RC_WrongValueType;
	    goto cleanUp;
	}
	
	/* Allocate an event structure */
	if ((pEvent = OS_alloc(sizeof(MM_Event) + ID_LEN)) == NULL)
	{
	    rc = ResourceError;
	    goto cleanUp;
	}

	/* Assign the parameters */
	pEvent->un.maxThresholdReenteredUnsigned.threshold =
	    protocol.value.un.unsignedInt;
	pEvent->un.maxThresholdReenteredUnsigned.newValue =
	    protocol.value2.un.unsignedInt;

	break;

    case MM_RemOp_MinThresholdReenteredSigned:
	if (protocol.value.choice != MM_VALUE_CHOICE_SIGNEDINT)
	{
	    rc = MM_RC_WrongValueType;
	    goto cleanUp;
	}
	
	/* Allocate an event structure */
	if ((pEvent = OS_alloc(sizeof(MM_Event) + ID_LEN)) == NULL)
	{
	    rc = ResourceError;
	    goto cleanUp;
	}

	/* Assign the parameters */
	pEvent->un.minThresholdReenteredSigned.threshold =
	    protocol.value.un.signedInt;
	pEvent->un.minThresholdReenteredSigned.newValue =
	    protocol.value2.un.signedInt;

	break;

    case MM_RemOp_MinThresholdReenteredUnsigned:
	if (protocol.value.choice != MM_VALUE_CHOICE_UNSIGNEDINT)
	{
	    rc = MM_RC_WrongValueType;
	    goto cleanUp;
	}
	
	/* Allocate an event structure */
	if ((pEvent = OS_alloc(sizeof(MM_Event) + ID_LEN)) == NULL)
	{
	    rc = ResourceError;
	    goto cleanUp;
	}

	/* Assign the parameters */
	pEvent->un.minThresholdReenteredUnsigned.threshold =
	    protocol.value.un.unsignedInt;
	pEvent->un.minThresholdReenteredUnsigned.newValue =
	    protocol.value2.un.unsignedInt;

	break;

    case MM_RemOp_TimerExpired:
	/* Allocate an event structure */
	if ((pEvent = OS_alloc(sizeof(MM_Event) + ID_LEN)) == NULL)
	{
	    rc = ResourceError;
	    goto cleanUp;
	}

	/* No parameters for this event */
	break;

    case MM_RemOp_LogMessage:
	if (protocol.value.choice != MM_VALUE_CHOICE_STRING)
	{
	    rc = MM_RC_WrongValueType;
	    goto cleanUp;
	}
	
	/* Allocate an event structure */
	if ((pEvent =
	     OS_alloc(sizeof(MM_Event) + ID_LEN +
		      STR_getStringLength(protocol.value.un.string) + 1)) ==
	     NULL)
	{
	    rc = ResourceError;
	    goto cleanUp;
	}

	/* Assign the parameters */
	pEvent->un.logMessage.pMessage = (char *) (pEvent + 1) + ID_LEN;
	strcpy(pEvent->un.logMessage.pMessage,
	       STR_stringStart(protocol.value.un.string));

	break;

    case MM_RemOp_ValueChangedSigned:
	if (protocol.value.choice != MM_VALUE_CHOICE_SIGNEDINT)
	{
	    rc = MM_RC_WrongValueType;
	    goto cleanUp;
	}
	
	/* Allocate an event structure */
	if ((pEvent = OS_alloc(sizeof(MM_Event) + ID_LEN)) == NULL)
	{
	    rc = ResourceError;
	    goto cleanUp;
	}

	/* Assign the parameters */
	pEvent->un.valueChangedSigned.newValue =
	    protocol.value.un.signedInt;

	break;

    case MM_RemOp_ValueChangedUnsigned:
	if (protocol.value.choice != MM_VALUE_CHOICE_UNSIGNEDINT)
	{
	    rc = MM_RC_WrongValueType;
	    goto cleanUp;
	}
	
	/* Allocate an event structure */
	if ((pEvent = OS_alloc(sizeof(MM_Event) + ID_LEN)) == NULL)
	{
	    rc = ResourceError;
	    goto cleanUp;
	}

	/* Assign the parameters */
	pEvent->un.valueChangedUnsigned.newValue =
	    protocol.value.un.unsignedInt;

	break;

    case MM_RemOp_ValueChangedString:
	if (protocol.value.choice != MM_VALUE_CHOICE_STRING)
	{
	    rc = MM_RC_WrongValueType;
	    goto cleanUp;
	}
	
	/* Allocate an event structure */
	if ((pEvent =
	     OS_alloc(sizeof(MM_Event) + ID_LEN +
		      STR_getStringLength(protocol.value.un.string) + 1)) ==
	     NULL)
	{
	    rc = ResourceError;
	    goto cleanUp;
	}

	/* Assign the parameters */
	pEvent->un.valueChangedString.pNewValue =
	    (char *) (pEvent + 1) + ID_LEN;
	strcpy(pEvent->un.valueChangedString.pNewValue,
	       STR_stringStart(protocol.value.un.string));

	break;

    default:
	/* Avoid compiler warnings.  The remainder aren't handled here. */
	break;
    }

    /* Assign the event's event type */
    pEvent->type = MM_EVENT_DECODE(operationValue);

    /* Assign the parameters which are in all events */
    pEvent->pApplicationEntityInstanceName = (char *) (pEvent + 1);
    strcpy(pEvent->pApplicationEntityInstanceName,
	   STR_stringStart(protocol.hApplicationEntityInstanceName));

    pEvent->pModuleName = (char *) (pEvent + 1) + APL_LEN;
    strcpy(pEvent->pModuleName, STR_stringStart(protocol.hModuleName));

    pEvent->pObjectName = (char *) (pEvent + 1) + APL_LEN + MOD_LEN;
    strcpy(pEvent->pObjectName,
	   STR_stringStart(protocol.hManagableObjectName));

    /* Give 'em what they came for */
    *ppEvent = pEvent;

    /* We've completed successfully. */
    rc = Success;

  cleanUp:

    mm_cleanUp(&protocol);

    return rc;

#undef	APL_LEN
#undef	MOD_LEN
#undef	OBJ_LEN
#undef	ID_LEN
}



static ReturnCode
synchronousOperation(MM_Request * pRequest,
		     MM_Result ** ppResult)
{
    void *			hBuf = NULL;
    void *			hOperation;
    ReturnCode			rc;
    MM_RemoteOperationType	operationType;
    MM_RemoteOperationValue	operationValue;

    /* Create the remote operation request */
    if ((rc = MM_remoteOperationRequest(pRequest, &hBuf)) != Success)
    {
	return rc;
    }

    /* Issue the operation */
    if ((rc = (* pfRemoteOperationSend)(&hOperation,
					MM_RemOpType_Request,
					pRequest->operationValue,
					&hBuf)) != Success)
    {
	return rc;
    }

    TM_CALL(mm_hModCB, MM_TM_PDU, BUF_dump, hBuf, "Manager Request");

    /* Free the request buffer */
    BUF_free(hBuf);

    /* Wait for a response */
    if ((rc = (* pfRemoteOperationRecv)(&hOperation,
					&operationType,
					&operationValue,
					&hBuf)) != Success)
    {
	return rc;
    }

    TM_CALL(mm_hModCB, MM_TM_PDU, BUF_dump, hBuf, "Manager Event (Result)");

    /* See what type of response we got */
    switch(operationType)
    {
    case MM_RemOpType_Request:
	/* Should never occur */
	BUF_free(hBuf);
	return MM_RC_UnexpectedRemoteOperationType;
	break;
	
    case MM_RemOpType_Result:
	/* Parse the response */
	if ((rc = MM_remoteOperationResult(hBuf,
					   operationValue,
					   ppResult)) != Success)
	{
	    BUF_free(hBuf);
	    return rc;
	}
	break;
	
    case MM_RemOpType_Event:
	/* We won't get there here. */
	BUF_free(hBuf);
	return Success;

    case MM_RemOpType_Error:
	/* Parse the error */
	MM_remoteOperationError(hBuf, &rc);
	BUF_free(hBuf);
	return rc;
	
    case MM_RemOpType_Failure:
	/* Some type of local or operation failure occured */
	BUF_free(hBuf);
	return MM_RC_CommunicationFailure;

    case MM_RemOpType_User:
    case MM_RemOpType_Reserved:
	/* won't occur here. */
	return Success;
    }

    /* Free the response buffer */
    BUF_free(hBuf);

    return Success;
}
