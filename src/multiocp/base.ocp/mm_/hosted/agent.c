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


/*
 * Global and Static Variables
 *
 * CAUTION: This may not be thread-safe.  The remainder of this module
 *	    should be thread-safe.
 */
static QU_Head * pApplicationEntityInstances = NULL;

static ReturnCode (* pfRemoteOperationSend)(void ** phOperation,
					    MM_RemoteOperationType opType,
					    MM_RemoteOperationValue opValue,
					    void ** phBuf) = NULL;
TM_ModuleCB *	mm_hModCB;


/*
 * Forward declaratations
 */
static ReturnCode
getValueByName(char * pApplicationEntityInstanceName,
	       char * pModuleName,
	       char * pManagableObjectName,
	       MM_ManagableObjectType * pObjectType,
	       MM_Value * pValue);

static ReturnCode
setValueByName(void * pApplicationEntityInstanceName,
	       char * pModuleName,
	       char * pManagableObjectName,
	       MM_ManagableObjectType objectType,
	       MM_Value * pValue);

static ReturnCode
getThresholdByName(char * pApplicationEntityInstanceName,
		   char * pModuleName,
		   char * pManagableObjectName,
		   MM_ThresholdType thresholdType,
		   MM_ManagableObjectType * pObjectType,
		   MM_Value * pValue);

static ReturnCode
setThresholdByName(void * pApplicationEntityInstanceName,
		   char * pModuleName,
		   char * pManagableObjectName,
		   MM_ThresholdType thresholdType,
		   MM_ManagableObjectType objectType,
		   MM_Value * pValue);

static ReturnCode
startTimerByName(char * pApplicationEntityInstanceName,
		 char * pModuleName,
		 char * pManagableObjectName,
		 OS_Uint32 milliseconds);

static ReturnCode
stopTimerByName(char * pApplicationEntityInstanceName,
		char * pModuleName,
		char * pManagableObjectName);

static ReturnCode
getApplicationEntityInstanceList(char ** ppBuf);

static ReturnCode
getModuleList(char * pApplicationEntityInstanceName,
	      char ** ppBuf);

static ReturnCode
getManagableObjectList(char * pApplicationEntityInstanceName,
		       char * pModuleName,
		       char ** ppBuf);

static ReturnCode
getManagableObjectInfo(char * pApplicationEntityInstanceName,
		       char * pModuleName,
		       char * pManagableObjectName,
		       MM_ManagableObjectType * pObjectType,
		       char ** ppBuf);

static ReturnCode
getManagableObjectNotifyMaskByName(char * pApplicationEntityInstanceName,
				   char * pModuleName,
				   char * pManagableObjectName,
				   OS_Uint32 * pNotifyMask);

static ReturnCode
setManagableObjectNotifyMaskByName(char * pApplicationEntityInstanceName,
				   char * pModuleName,
				   char * pManagableObjectName,
				   OS_Uint32 notifyMask);

static ReturnCode
getDestinationList(char * pApplicationEntityInstanceName,
		   char ** ppBuf);

static ReturnCode
getDestinationMasksByName(char * pApplicationEntityInstanceName,
			  char * pDestinationName,
			  OS_Uint32 * pNotifyMask,
			  OS_Uint32 * pEventMask);

static ReturnCode
setDestinationMasksByName(char * pApplicationEntityInstanceName,
			  char * pDestinationName,
			  OS_Uint32 newNotifyMask,
			  OS_Uint32 newEventMask);

static void
agentErrorResponse(void ** phOperation,
		   OS_Uint8 operationValue,
		   ReturnCode rc);

static ReturnCode
formatEventPdu(MM_RemoteOperationType operationType,
	       MM_RemoteOperationValue operationValue,
	       MM_Protocol * p,
	       void ** phBuf);

static void
cleanUp(MM_Protocol * p);

#ifdef TM_ENABLED

static char *
valueTypeString(MM_Value * pValue);

static char *
valueString(MM_Value * pValue);

#endif


/*
 * MM_agentInit()
 *
 * Initialize the Module Management agent.
 */
ReturnCode
MM_agentInit(ReturnCode (* pfRemOpSend)(void ** phOperation,
					MM_RemoteOperationType operationType,
					MM_RemoteOperationValue operationValue,
					void ** phBuf))
{
    ReturnCode	    rc;

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

    /* Get the pointer to the queue of application entity instances */
    pApplicationEntityInstances = mm_getApplicationEntityInstances();

    /* Keep track of the function to call to send part of an operation */
    pfRemoteOperationSend = pfRemOpSend;

    return Success;
}



/*
 * getValueByName()
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
 *   pObjectType --
 *     Pointer to the location in which to place the type of object
 *     represented by the specified managable object name.
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
static ReturnCode
getValueByName(char * pApplicationEntityInstanceName,
	       char * pModuleName,
	       char * pManagableObjectName,
	       MM_ManagableObjectType * pObjectType,
	       MM_Value * pValue)
{
    ManagableObject * 		pObject;
    Module *			pModule;
    ApplicationEntityInstance *	pApplicationEntityInstance;

    /* Find the specified application entity instance */
    if ((pApplicationEntityInstance =
	 mm_findApplicationEntityInstance(pApplicationEntityInstanceName)) ==
	NULL)
    {
	return FAIL_RC(MM_RC_NoSuchApplicationEntityInstance,
		       ("Could not find application entity instance (%s)\n",
			pApplicationEntityInstanceName));
    }

    /* Find the specified module */
    if ((pModule =
	 mm_findModule(pModuleName, pApplicationEntityInstance)) == NULL)
    {
	return FAIL_RC(MM_RC_NoSuchModule,
		       ("Could not find module (%s)\n", pModuleName));
    }

    /* Find the specified object */
    if ((pObject = mm_findObject(pManagableObjectName, pModule)) == NULL)
    {
	return FAIL_RC(MM_RC_NoSuchManagableObject,
		       ("Could not find module (%s)\n", pModuleName));
    }

    /* Make sure the object provides byName read access */
    if ((pObject->accessByName & MM_AccessByName_Read) == 0)
    {
	/* It doesn't. */
	return FAIL_RC(MM_RC_ByNameReadAccessDenied,
		       ("getValueByName() "
			"byName access to object (%s)denied\n",
			pObject->pName));
    }

    /* Give 'em the object type of this object */
    *pObjectType = pObject->objectType;

    return MM_getValueByHandle(pObject, pValue);
}

/*
 * setValueByName()
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
 *   objectType --
 *     Type of object represented by the specified managable object
 *     name.
 *
 *   pValue --
 *     Pointer to the new value for this managable object.  It is up
 *     to the caller to provide a pointer to the correct type of
 *     variable into which the value will be placed.
 */
static ReturnCode
setValueByName(void * pApplicationEntityInstanceName,
	       char * pModuleName,
	       char * pManagableObjectName,
	       MM_ManagableObjectType objectType,
	       MM_Value * pValue)
{
    ManagableObject * 		pObject;
    Module *			pModule;
    ApplicationEntityInstance *	pApplicationEntityInstance;

    /* Find the specified application entity instance */
    if ((pApplicationEntityInstance =
	 mm_findApplicationEntityInstance(pApplicationEntityInstanceName)) ==
	NULL)
    {
	return FAIL_RC(MM_RC_NoSuchApplicationEntityInstance,
		       ("Could not find application entity instance (%s)\n",
			(char *) pApplicationEntityInstanceName));
    }

    /* Find the specified module */
    if ((pModule =
	 mm_findModule(pModuleName, pApplicationEntityInstance)) == NULL)
    {
	return FAIL_RC(MM_RC_NoSuchModule,
		       ("Could not find module (%s)\n", pModuleName));
    }

    /* Find the specified object */
    if ((pObject = mm_findObject(pManagableObjectName, pModule)) == NULL)
    {
	return FAIL_RC(MM_RC_NoSuchManagableObject,
		       ("Could not find module (%s)\n", pModuleName));
    }

    /* Make sure the object type of the value is correct */
    if (objectType != pObject->objectType)
    {
	/* It isn't. */
	return FAIL_RC(MM_RC_WrongObjectType,
		       ("setValueByName() "
			"type of value doesn't match object type's value\n"));
    }

    /* Make sure the object provides byName write access */
    if ((pObject->accessByName & MM_AccessByName_Write) == 0)
    {
	/* It doesn't. */
	return FAIL_RC(MM_RC_ByNameWriteAccessDenied,
		       ("setValueByName() "
			"byName access to object (%s) denied\n",
			pObject->pName));
    }

    return MM_setValueByHandle(pObject, pValue);
}



/*
 * getThresholdByName()
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
 *   pObjectType --
 *     Pointer to the location in which to place the type of object
 *     represented by the specified managable object name.
 *
 *   pValue --
 *     Pointer to the location where the current threshold of the
 *     specified managable object is to be placed.  It is up to the
 *     caller to provide a pointer to the correct type of variable
 *     into which the threshold will be placed.
 */
static ReturnCode
getThresholdByName(char * pApplicationEntityInstanceName,
		   char * pModuleName,
		   char * pManagableObjectName,
		   MM_ThresholdType thresholdType,
		   MM_ManagableObjectType * pObjectType,
		   MM_Value * pValue)
{
    ManagableObject * 		pObject;
    Module *			pModule;
    ApplicationEntityInstance *	pApplicationEntityInstance;

    /* Find the specified application entity instance */
    if ((pApplicationEntityInstance =
	 mm_findApplicationEntityInstance(pApplicationEntityInstanceName)) ==
	NULL)
    {
	return FAIL_RC(MM_RC_NoSuchApplicationEntityInstance,
		       ("Could not find application entity instance (%s)\n",
			pApplicationEntityInstanceName));
    }

    /* Find the specified module */
    if ((pModule =
	 mm_findModule(pModuleName, pApplicationEntityInstance)) == NULL)
    {
	return FAIL_RC(MM_RC_NoSuchModule,
		       ("Could not find module (%s)\n", pModuleName));
    }

    /* Find the specified object */
    if ((pObject = mm_findObject(pManagableObjectName, pModule)) == NULL)
    {
	return FAIL_RC(MM_RC_NoSuchManagableObject,
		       ("Could not find module (%s)\n", pModuleName));
    }

    /* Make sure the object provides byName read access */
    if ((pObject->accessByName & MM_AccessByName_Read) == 0)
    {
	/* It doesn't. */
	return FAIL_RC(MM_RC_ByNameReadAccessDenied,
		       ("getThresholdByName() "
			"byName access to object (%s)denied\n",
			pObject->pName));
    }

    /* Give 'em the object type of this object */
    *pObjectType = pObject->objectType;

    return MM_getThresholdByHandle(pObject, thresholdType, pValue);
}

/*
 * setThresholdByName()
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
 *   objectType --
 *     Type of object represented by the specified managable object
 *     name.
 *
 *   pValue --
 *     Pointer to the new threshold for this managable object.  It is up
 *     to the caller to provide a pointer to the correct type of
 *     variable into which the threshold will be placed.
 */
static ReturnCode
setThresholdByName(void * pApplicationEntityInstanceName,
		   char * pModuleName,
		   char * pManagableObjectName,
		   MM_ThresholdType thresholdType,
		   MM_ManagableObjectType objectType,
		   MM_Value * pValue)
{
    ManagableObject * 		pObject;
    Module *			pModule;
    ApplicationEntityInstance *	pApplicationEntityInstance;

    /* Make sure it's a legal threshold type for remote access */
    if (thresholdType != MM_ThresholdType_MinimumThreshold &&
	thresholdType != MM_ThresholdType_MaximumThreshold)
    {
	return FAIL_RC(MM_RC_InvalidThresholdType,
		       ("Invalid threshold type for remote access: %d\n",
			thresholdType));
    }

    /* Find the specified application entity instance */
    if ((pApplicationEntityInstance =
	 mm_findApplicationEntityInstance(pApplicationEntityInstanceName)) ==
	NULL)
    {
	return FAIL_RC(MM_RC_NoSuchApplicationEntityInstance,
		       ("Could not find application entity instance (%s)\n",
			(char *) pApplicationEntityInstanceName));
    }

    /* Find the specified module */
    if ((pModule =
	 mm_findModule(pModuleName, pApplicationEntityInstance)) == NULL)
    {
	return FAIL_RC(MM_RC_NoSuchModule,
		       ("Could not find module (%s)\n", pModuleName));
    }

    /* Find the specified object */
    if ((pObject = mm_findObject(pManagableObjectName, pModule)) == NULL)
    {
	return FAIL_RC(MM_RC_NoSuchManagableObject,
		       ("Could not find module (%s)\n", pModuleName));
    }

    /* Make sure the object type of the value is correct */
    if (objectType != pObject->objectType)
    {
	/* It isn't. */
	return FAIL_RC(MM_RC_WrongObjectType,
		       ("setThresholdByName() "
			"type of value doesn't match object type's value\n"));
    }

    /* Make sure the object provides byName write access */
    if ((pObject->accessByName & MM_AccessByName_Write) == 0)
    {
	/* It doesn't. */
	return FAIL_RC(MM_RC_ByNameWriteAccessDenied,
		       ("setThresholdByName() "
			"byName access to object (%s) denied\n",
			pObject->pName));
    }

    return MM_setThresholdByHandle(pObject, thresholdType, pValue);
}



/*
 * startTimerByName()
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
static ReturnCode
startTimerByName(char * pApplicationEntityInstanceName,
		 char * pModuleName,
		 char * pManagableObjectName,
		 OS_Uint32 milliseconds)
{
    ManagableObject * 		pObject;
    Module *			pModule;
    ApplicationEntityInstance *	pApplicationEntityInstance;

    /* Find the specified application entity instance */
    if ((pApplicationEntityInstance =
	 mm_findApplicationEntityInstance(pApplicationEntityInstanceName)) ==
	NULL)
    {
	return FAIL_RC(MM_RC_NoSuchApplicationEntityInstance,
		       ("Could not find application entity instance (%s)\n",
			pApplicationEntityInstanceName));
    }

    /* Find the specified module */
    if ((pModule = mm_findModule(pModuleName,
				 pApplicationEntityInstance)) == NULL)
    {
	return FAIL_RC(MM_RC_NoSuchModule,
		       ("Could not find module (%s)\n", pModuleName));
    }

    /* Find the specified object */
    if ((pObject = mm_findObject(pManagableObjectName, pModule)) == NULL)
    {
	return FAIL_RC(MM_RC_NoSuchManagableObject,
		       ("Could not find module (%s)\n", pModuleName));
    }

    /* Make sure the object provides byName write access */
    if ((pObject->accessByName & MM_AccessByName_Write) == 0)
    {
	/* It doesn't. */
	return FAIL_RC(MM_RC_ByNameWriteAccessDenied,
		       ("startTimerByName() "
			"byName access to object (%s) denied\n",
			pObject->pName));
    }

    /* Now that we have the object handle, go start the timer. */
    return MM_startTimerByHandle(pObject, milliseconds);
}


/*
 * stopTimerByName()
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
static ReturnCode
stopTimerByName(char * pApplicationEntityInstanceName,
		char * pModuleName,
		char * pManagableObjectName)
{
    ManagableObject * 		pObject;
    Module *			pModule;
    ApplicationEntityInstance *	pApplicationEntityInstance;

    /* Find the specified application entity instance */
    if ((pApplicationEntityInstance =
	 mm_findApplicationEntityInstance(pApplicationEntityInstanceName)) ==
	NULL)
    {
	return FAIL_RC(MM_RC_NoSuchApplicationEntityInstance,
		       ("Could not find application entity instance (%s)\n",
			pApplicationEntityInstanceName));
    }

    /* Find the specified module */
    if ((pModule = mm_findModule(pModuleName,
				 pApplicationEntityInstance)) == NULL)
    {
	return FAIL_RC(MM_RC_NoSuchModule,
		       ("Could not find module (%s)\n", pModuleName));
    }

    /* Find the specified object */
    if ((pObject = mm_findObject(pManagableObjectName, pModule)) == NULL)
    {
	return FAIL_RC(MM_RC_NoSuchManagableObject,
		       ("Could not find module (%s)\n", pModuleName));
    }

    /* Make sure the object provides byName write access */
    if ((pObject->accessByName & MM_AccessByName_Write) == 0)
    {
	/* It doesn't. */
	return FAIL_RC(MM_RC_ByNameWriteAccessDenied,
		       ("stopTimerByName() "
			"byName access to object (%s) denied\n",
			pObject->pName));
    }

    /* Now that we have the object handle, go start the timer. */
    return MM_stopTimerByHandle(pObject);
}


/*
 * getApplicationEntityInstanceList()
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
static ReturnCode
getApplicationEntityInstanceList(char ** ppBuf)
{
    ApplicationEntityInstance *	pApplicationEntityInstance;
    int 			nBytes;
    char *			pBuf;

    /* Initialize the byte count */
    nBytes = 0;

    /*
     * Go through the application entity instance list and find out
     * how many bytes we'll need for all of the application entity
     * instance names.
     */
    /* Find the specified application entity instance */
    for (pApplicationEntityInstance = QU_FIRST(pApplicationEntityInstances);
	 ! QU_EQUAL(pApplicationEntityInstance, pApplicationEntityInstances);
	 pApplicationEntityInstance = QU_NEXT(pApplicationEntityInstance))
    {
	nBytes += strlen(pApplicationEntityInstance->pName) + 1;
    }
    
    /*
     * Allocate a string long enough to hold all of the application
     * entity instance names.
     */
    if ((pBuf = OS_alloc(nBytes + 1)) == NULL)
    {
	return FAIL_RC(ResourceError,
		       ("MM_getApplicationEntityInstanceList() "
			"out of memory getting buffer\n"));
    }
    
    /* Give 'em what they came for */
    *ppBuf = pBuf;

    /* Initially, there's nothing in the buffer. */
    pBuf[0] = pBuf[1] = '\0';

    /*
     * Go through the application entity instance list again and copy
     * the names of the application entity instances into the buffer
     * which will be returned.
     */
    for (pApplicationEntityInstance = QU_FIRST(pApplicationEntityInstances);
	 ! QU_EQUAL(pApplicationEntityInstance, pApplicationEntityInstances);
	 pApplicationEntityInstance = QU_NEXT(pApplicationEntityInstance))
    {
	/* Copy the module name to the end of the buffer */
	strcpy(pBuf, pApplicationEntityInstance->pName);

	/* Update our pointer to the location of the next name */
	pBuf += strlen(pBuf) + 1;
    }

    /* Add the trailing EXTRA null terminator to indicate end of name list */
    *pBuf = '\0';

    return Success;
}


/*
 * getModuleList()
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
static ReturnCode
getModuleList(char * pApplicationEntityInstanceName,
	      char ** ppBuf)
{
    ApplicationEntityInstance *	pApplicationEntityInstance;
    Module * 			pModule;
    int 			nBytes;
    char *			pBuf;

    /* Find the specified application entity instance */
    if ((pApplicationEntityInstance =
	 mm_findApplicationEntityInstance(pApplicationEntityInstanceName)) ==
	NULL)
    {
	return FAIL_RC(MM_RC_NoSuchApplicationEntityInstance,
		       ("MM_getModuleList() could not find application %s\n",
			pApplicationEntityInstanceName));
    }

    /* Initialize the byte count */
    nBytes = 0;

    /*
     * Go through the module list and find out how many bytes we'll need for
     * all of the module names.
     */
    for (pModule = QU_FIRST(&pApplicationEntityInstance->modules);
	 ! QU_EQUAL(pModule, &pApplicationEntityInstance->modules);
	 pModule = QU_NEXT(pModule))
    {
	nBytes += strlen(pModule->pName) + 1;
    }

    /* Allocate a string long enough to hold all of the module names */
    if ((pBuf = OS_alloc(nBytes + 1)) == NULL)
    {
	return FAIL_RC(ResourceError,
		       ("MM_getModuleList() out of memory getting buffer\n"));
    }
    
    /* Give 'em what they came for */
    *ppBuf = pBuf;

    /* Initially, there's nothing in the buffer. */
    pBuf[0] = pBuf[1] = '\0';

    /*
     * Go through the module list again and copy the names of the
     * modules into the buffer which will be returned.
     */
    for (pModule = QU_FIRST(&pApplicationEntityInstance->modules);
	 ! QU_EQUAL(pModule, &pApplicationEntityInstance->modules);
	 pModule = QU_NEXT(pModule))
    {
	/* Copy the module name to the end of the buffer */
	strcpy(pBuf, pModule->pName);

	/* Update our pointer to the location of the next name */
	pBuf += strlen(pBuf) + 1;
    }

    /* Add the trailing EXTRA null terminator to indicate end of name list */
    *pBuf = '\0';

    return Success;
}


/*
 * getManagableObjectList()
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
static ReturnCode
getManagableObjectList(char * pApplicationEntityInstanceName,
		       char * pModuleName,
		       char ** ppBuf)
{
    ApplicationEntityInstance *	pApplicationEntityInstance;
    Module * 			pModule;
    ManagableObject *		pObject;
    int 			nBytes;
    char *			pBuf;

    /* Find the specified application entity instance */
    if ((pApplicationEntityInstance =
	 mm_findApplicationEntityInstance(pApplicationEntityInstanceName)) ==
	NULL)
    {
	return FAIL_RC(MM_RC_NoSuchApplicationEntityInstance,
		       ("MM_getManagableObjectList() "
			"could not find application (%s)\n",
			pApplicationEntityInstanceName));
    }

    /* Find the specified module */
    if ((pModule = mm_findModule(pModuleName,
				 pApplicationEntityInstance)) == NULL)
    {
	return FAIL_RC(MM_RC_NoSuchModule,
		       ("MM_getManagableObjectList() "
			"could not find module (%s)\n",
			pModuleName));
    }

    /* Initialize the byte count */
    nBytes = 0;

    /*
     * Go through the managable object list and find out how many
     * bytes we'll need for all of the managable object names.  Only
     * provide object names that have byName read and/or write access.
     */
    for (pObject = QU_FIRST(&pModule->managableObjects);
	 ! QU_EQUAL(pObject, &pModule->managableObjects);
	 pObject = QU_NEXT(pObject))
    {
	/* Make sure this object provides byName read and/or write access */
	if ((pObject->accessByName & MM_AccessByName_ReadWrite) != 0)
	{
	    nBytes += strlen(pObject->pName) + 1;
	}
    }

    /* Allocate a string long enough to hold all of the module names */
    if ((pBuf = OS_alloc(nBytes + 1)) == NULL)
    {
	return FAIL_RC(ResourceError,
		       ("MM_getManagableObjectList() "
			"out of memory getting buffer\n"));
    }
    
    /* Give 'em what they came for */
    *ppBuf = pBuf;

    /* Initially, there's nothing in the buffer. */
    pBuf[0] = pBuf[1] = '\0';

    /*
     * Go through the managable object list again and copy the names
     * of the managable objects into the buffer which will be
     * returned.
     */
    for (pObject = QU_FIRST(&pModule->managableObjects);
	 ! QU_EQUAL(pObject, &pModule->managableObjects);
	 pObject = QU_NEXT(pObject))
    {
	/* Make sure this object provides byName read and/or write access */
	if ((pObject->accessByName & MM_AccessByName_ReadWrite) != 0)
	{
	    /* Copy the module name to the end of the buffer */
	    strcpy(pBuf, pObject->pName);
	    
	    /* Update our pointer to the location of the next name */
	    pBuf += strlen(pBuf) + 1;
	}
    }

    /* Add the trailing EXTRA null terminator to indicate end of name list */
    *pBuf = '\0';

    return Success;
}



/*
 * getManagableObjectInfo()
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
static ReturnCode
getManagableObjectInfo(char * pApplicationEntityInstanceName,
		       char * pModuleName,
		       char * pManagableObjectName,
		       MM_ManagableObjectType * pObjectType,
		       char ** ppBuf)
{
    ApplicationEntityInstance *	pApplicationEntityInstance;
    Module * 			pModule;
    ManagableObject *		pObject;
    char *			pBuf;

    /* Find the specified application entity instance */
    if ((pApplicationEntityInstance =
	 mm_findApplicationEntityInstance(pApplicationEntityInstanceName)) ==
	NULL)
    {
	return FAIL_RC(MM_RC_NoSuchApplicationEntityInstance,
		       ("MM_getManagableObjectInfo() "
			"could not find application (%s)\n",
			pApplicationEntityInstanceName));
    }

    /* Find the specified module */
    if ((pModule =
	 mm_findModule(pModuleName, pApplicationEntityInstance)) == NULL)
    {
	return FAIL_RC(MM_RC_NoSuchModule,
		       ("MM_getManagableObjectInfo() "
			"could not find module (%s)\n",
			pModuleName));
    }

    /* Find the specified managable object */
    if ((pObject = mm_findObject(pManagableObjectName, pModule)) == NULL)
    {
	return FAIL_RC(MM_RC_NoSuchManagableObject,
		       ("MM_getManagableObjectInfo() "
			"could not find managable object (%s)\n",
			pManagableObjectName));
    }

    /* Allocate a buffer for the managable object's identification string */
    if ((pBuf = OS_allocStringCopy(pObject->pDescription)) == NULL)
    {
	return FAIL_RC(ResourceError,
		       ("MM_getManagableObjectInfo() "
			"out of memory for module (%s) id message (%s)\n",
			pObject->pName,
			pObject->pDescription));
    }

    /* Give 'em what they came for */
    *ppBuf = pBuf;
    *pObjectType = pObject->objectType;
    
    /* Make sure the object provides byName read and/or write access */
    if ((pObject->accessByName & MM_AccessByName_ReadWrite) == 0)
    {
	/* It doesn't.  Fail silently */
	*pBuf = '\0';
	return Success;
    }
    
    return Success;
}


/*
 * getManagableObjectNotifyMaskByName()
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
static ReturnCode
getManagableObjectNotifyMaskByName(char * pApplicationEntityInstanceName,
				   char * pModuleName,
				   char * pManagableObjectName,
				   OS_Uint32 * pNotifyMask)
{
    ApplicationEntityInstance *	pApplicationEntityInstance;
    Module * 			pModule;
    ManagableObject *		pObject;

    /* Find the specified application entity instance */
    if ((pApplicationEntityInstance =
	 mm_findApplicationEntityInstance(pApplicationEntityInstanceName)) ==
	NULL)
    {
	return FAIL_RC(MM_RC_NoSuchApplicationEntityInstance,
		       ("getManagableObjectNotifyMaskByName() "
			"could not find application (%s)\n",
			pApplicationEntityInstanceName));
    }

    /* Find the specified module */
    if ((pModule =
	 mm_findModule(pModuleName, pApplicationEntityInstance)) == NULL)
    {
	return FAIL_RC(MM_RC_NoSuchModule,
		       ("getManagableObjectNotifyMaskByName() "
			"could not find module (%s)\n",
			pModuleName));
    }

    /* Find the specified managable object */
    if ((pObject = mm_findObject(pManagableObjectName, pModule)) == NULL)
    {
	return FAIL_RC(MM_RC_NoSuchManagableObject,
		       ("getManagableObjectNotifyMaskByName() "
			"could not find managable object (%s)\n",
			pManagableObjectName));
    }

    /* Now that we have the object handle, give 'em what they came for */
    return MM_getManagableObjectNotifyMaskByHandle(pObject, pNotifyMask);
}


/*
 * setManagableObjectNotifyMaskByName()
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
static ReturnCode
setManagableObjectNotifyMaskByName(char * pApplicationEntityInstanceName,
				   char * pModuleName,
				   char * pManagableObjectName,
				   OS_Uint32 notifyMask)
{
    ApplicationEntityInstance *	pApplicationEntityInstance;
    Module * 			pModule;
    ManagableObject *		pObject;

    /* Find the specified application entity instance */
    if ((pApplicationEntityInstance =
	 mm_findApplicationEntityInstance(pApplicationEntityInstanceName)) ==
	NULL)
    {
	return FAIL_RC(MM_RC_NoSuchApplicationEntityInstance,
		       ("setManagableObjectNotifyMaskByName() "
			"could not find application (%s)\n",
			pApplicationEntityInstanceName));
    }

    /* Find the specified module */
    if ((pModule =
	 mm_findModule(pModuleName, pApplicationEntityInstance)) == NULL)
    {
	return FAIL_RC(MM_RC_NoSuchModule,
		       ("setManagableObjectNotifyMaskByName() "
			"could not find module (%s)\n",
			pModuleName));
    }

    /* Find the specified managable object */
    if ((pObject = mm_findObject(pManagableObjectName, pModule)) == NULL)
    {
	return FAIL_RC(MM_RC_NoSuchManagableObject,
		       ("setManagableObjectNotifyMaskByName() "
			"could not find managable object (%s)\n",
			pManagableObjectName));
    }

    /* Now that we have the object handle, give 'em what they came for */
    return MM_setManagableObjectNotifyMaskByHandle(pObject, notifyMask);
}



/*
 * getDestinationList()
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
static ReturnCode
getDestinationList(char * pApplicationEntityInstanceName,
		   char ** ppBuf)
{
    Destination *		pDestination;
    ApplicationEntityInstance *	pApplicationEntityInstance;
    int 			nBytes;
    char *			pBuf;

    /* Find the specified application entity instance */
    if ((pApplicationEntityInstance =
	 mm_findApplicationEntityInstance(pApplicationEntityInstanceName)) ==
	NULL)
    {
	return FAIL_RC(MM_RC_NoSuchApplicationEntityInstance,
		       ("MM_getDestinationList() "
			"could not find application (%s)\n",
			pApplicationEntityInstanceName));
    }

    /* Initialize the byte count */
    nBytes = 0;

    /*
     * Go through the destination list and find out how many
     * bytes we'll need for all of the destination names.  Only
     * provide object names that have byName read and/or write access.
     */
    for (pDestination = QU_FIRST(&pApplicationEntityInstance->destinations);
	 ! QU_EQUAL(pDestination, &pApplicationEntityInstance->destinations);
	 pDestination = QU_NEXT(pDestination))
    {
	/*
	 * Make sure this destination provides byName read and/or
	 * write access.
	 */
	if ((pDestination->accessByName & MM_AccessByName_ReadWrite) != 0)
	{
	    nBytes += strlen(pDestination->pName) + 1;
	}
    }

    /* Allocate a string long enough to hold all of the destination names */
    if ((pBuf = OS_alloc(nBytes + 1)) == NULL)
    {
	return FAIL_RC(ResourceError,
		       ("MM_getDestinationList() "
			"out of memory getting buffer\n"));
    }
    
    /* Give 'em what they came for */
    *ppBuf = pBuf;

    /* Initially, there's nothing in the buffer. */
    pBuf[0] = pBuf[1] = '\0';

    /*
     * Go through the destination list again and copy the names
     * of the destinations into the buffer which will be
     * returned.
     */
    for (pDestination = QU_FIRST(&pApplicationEntityInstance->destinations);
	 ! QU_EQUAL(pDestination, &pApplicationEntityInstance->destinations);
	 pDestination = QU_NEXT(pDestination))
    {
	/*
	 * Make sure this destination provides byName read and/or
	 * write access.
	 */
	if ((pDestination->accessByName & MM_AccessByName_ReadWrite) != 0)
	{
	    /* Copy the module name to the end of the buffer */
	    strcpy(pBuf, pDestination->pName);
	    
	    /* Update our pointer to the location of the next name */
	    pBuf += strlen(pBuf) + 1;
	}
    }

    /* Add the trailing EXTRA null terminator to indicate end of name list */
    *pBuf = '\0';

    return Success;
}



/*
 * getDestinationMasksByName()
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
static ReturnCode
getDestinationMasksByName(char * pApplicationEntityInstanceName,
			  char * pDestinationName,
			  OS_Uint32 * pNotifyMask,
			  OS_Uint32 * pEventMask)
{
    ApplicationEntityInstance *	pApplicationEntityInstance;
    Destination *		pDestination;

    /* Find the specified application entity instance */
    if ((pApplicationEntityInstance =
	 mm_findApplicationEntityInstance(pApplicationEntityInstanceName)) ==
	NULL)
    {
	return FAIL_RC(MM_RC_NoSuchApplicationEntityInstance,
		       ("Could not find application entity instance (%s)\n",
			pApplicationEntityInstanceName));
    }

    /* Find the specified destination */
    if ((pDestination =
	 mm_findDestination(pDestinationName,
			    pApplicationEntityInstance)) == NULL)
    {
	return FAIL_RC(MM_RC_NoSuchDestination,
		       ("Could not find destination (%s)\n",
			pDestinationName));
    }

    /* Make sure the object provides byName read access */
    if ((pDestination->accessByName & MM_AccessByName_Read) == 0)
    {
	/* It doesn't. */
	return FAIL_RC(MM_RC_ByNameReadAccessDenied,
		       ("getDestinationMasksByName() "
			"byName access to destination (%s) denied\n",
			pDestinationName));
    }

    return MM_getDestinationMasksByHandle(pDestination,
					  pNotifyMask, pEventMask);
}


/*
 * setDestinationMasksByName()
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
static ReturnCode
setDestinationMasksByName(char * pApplicationEntityInstanceName,
			  char * pDestinationName,
			  OS_Uint32 newNotifyMask,
			  OS_Uint32 newEventMask)
{
    ApplicationEntityInstance *	pApplicationEntityInstance;
    Destination *		pDestination;

    /* Find the specified application entity instance */
    if ((pApplicationEntityInstance =
	 mm_findApplicationEntityInstance(pApplicationEntityInstanceName)) ==
	NULL)
    {
	return FAIL_RC(MM_RC_NoSuchApplicationEntityInstance,
		       ("Could not find application entity instance (%s)\n",
			pApplicationEntityInstanceName));
    }

    /* Find the specified destination */
    if ((pDestination =
	 mm_findDestination(pDestinationName,
			    pApplicationEntityInstance)) == NULL)
    {
	return FAIL_RC(MM_RC_NoSuchDestination,
		       ("Could not find destination (%s)\n",
			pDestinationName));
    }

    /* Make sure the object provides byName read access */
    if ((pDestination->accessByName & MM_AccessByName_Read) == 0)
    {
	/* It doesn't. */
	return FAIL_RC(MM_RC_ByNameReadAccessDenied,
		       ("modifyDestinationMasksByName() "
			"byName access to destination (%s) denied\n",
			pDestinationName));
    }

    /* Now that we have the destination handle, set the values as requested */
    return MM_setDestinationMasksByHandle(pDestination,
					  newNotifyMask, newEventMask);
}


/*
 * MM_agentProcessEvent()
 *
 * Parse a received request PDU, and call the appropriate static (local)
 * ByName function to accomplish the request.  Then, format a result or error
 * PDU and call the registered function to send the result or error remote
 * operation.
 *
 *
 * Parameters:
 *
 *   operationType --
 *     The type of operation just received.  (The only type of operation
 *     received by the agent is a Request.
 *
 *   operationValue --
 *     The operation value specifying which operation was requested.
 *
 *   phOperation --
 *     The handle to the received operation request (often an invoke id, but
 *     depends on the remote operation mechanism in use).
 *
 *   hRequestBuf --
 *     Handle of the buffer containing the request PDU.
 */
void
MM_agentProcessEvent(MM_RemoteOperationType operationType,
		     MM_RemoteOperationValue operationValue,
		     void ** phOperation,
		     void * hRequestBuf)
{
    int				i;
    int 			len;
    OS_Uint32			milliseconds;
    OS_Uint32			notifyMask;
    OS_Uint32			eventMask;
    OS_Uint32			pduLen;
    STR_String			hString = NULL;
    void *			hResultBuf = NULL;
    char *			pApplicationName = NULL;
    char *			pModuleName = NULL;
    char *			pObjectName = NULL;
    char *			pDestinationName = NULL;
    char *			p;
    char *			pList;
    ReturnCode			rc = SUCCESS;
    MM_ManagableObjectType	objectType;
    MM_ThresholdType		thresholdType;
    MM_Value			value;
    MM_Protocol			protocol[2];
#define	request	protocol[0]
#define	result	protocol[1]

    TM_CALL(mm_hModCB, MM_TM_PDU, BUF_dump, hRequestBuf, "Agent Event");

    /* Make sure we got a Request */
    if (operationType != MM_RemOpType_Request)
    {
	rc = MM_RC_UnexpectedRemoteOperationType;
	goto cleanUp;
    }

    /* Make sure we got a known operation value */
    if (operationValue <= 0 || operationValue >= MM_RemOp_MAX)
    {
	rc = MM_RC_UnrecognizedRemoteOperationValue;
	goto cleanUp;
    }

    /*
     * Zero-out the request and result structures so we know how
     * to clean up.
     */
    OS_memSet((void *) &request, '\0', sizeof(MM_Protocol));
    OS_memSet((void *) &result, '\0', sizeof(MM_Protocol));

    /* Specify the maximum parse length */
    pduLen = BUF_getBufferLength(hRequestBuf) - 2;

    /* Parse the MM Protocol PDU */
    if ((rc = ASN_parse(ASN_EncodingRules_Basic,
			&mm_protocols[operationValue].requestQ,
			hRequestBuf, &request,
			&pduLen)) != Success)
    {
/* check this */
	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "ASN_parse failed, module %s, reason %d\n",
		  MODID_RCPARAMS(rc)));
	goto cleanUp;
    }

    /* Which operation was requested? */
    switch(operationValue)
    {
    case MM_RemOp_GetValue:
	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "Agent received GetValue event:\n"));

	pApplicationName =
	    OS_allocStringNCopy(
		STR_stringStart(request.hApplicationEntityInstanceName),
		(OS_Uint16)
		STR_getStringLength(request.hApplicationEntityInstanceName)
		);
	pModuleName =
	    OS_allocStringNCopy(
		STR_stringStart(request.hModuleName),
		(OS_Uint16) STR_getStringLength(request.hModuleName)
		);
	pObjectName =
	    OS_allocStringNCopy(
		STR_stringStart(request.hManagableObjectName),
		(OS_Uint16) STR_getStringLength(request.hManagableObjectName)
		);

	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "\tApplication Name = (%s)\n"
		  "\tModule Name = (%s)\n"
		  "\tObject Name = (%s)\n",
		  pApplicationName, pModuleName, pObjectName));

	if ((rc = getValueByName(pApplicationName,
				 pModuleName,
				 pObjectName,
				 &objectType,
				 &value)) != Success)
	{
	    TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		      "getValue failed, module %s, reason %d\n",
		      MODID_RCPARAMS(rc)));
	    agentErrorResponse(phOperation,
			       (OS_Uint8) operationValue, rc);
	    goto cleanUp;
	}
				   
	switch(value.type)
	{
	case MM_ValueType_SignedInt:
	    result.value.choice = MM_VALUE_CHOICE_SIGNEDINT;
	    result.value.un.signedInt = value.un.signedInt;
	    TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		      "\t\tRetrieved Signed Int %ld\n",
		      result.value.un.signedInt));
	    break;
		
	case MM_ValueType_UnsignedInt:
	    result.value.choice = MM_VALUE_CHOICE_UNSIGNEDINT;
	    result.value.un.unsignedInt = value.un.unsignedInt;
	    TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		      "\t\tRetrieved Unsigned Int %ld\n",
		      result.value.un.unsignedInt));
	    break;
		
	case MM_ValueType_String:
	    result.value.choice = MM_VALUE_CHOICE_STRING;

	    if ((rc =
		 STR_attachZString(value.un.string,
				   TRUE,
				   &result.value.un.string)) != Success)
	    {
		TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
			  "STR_attachString failed, module %s, reason %d\n",
			  MODID_RCPARAMS(rc)));
		agentErrorResponse(phOperation,
				   (OS_Uint8) operationValue, rc);
		goto cleanUp;
	    }
	    TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		      "\t\tRetrieved String (%s)\n",
		      result.value.un.string));
	    break;
	}

	break;
	    
    case MM_RemOp_SetValue:
	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "Agent received SetValue event:\n"));
	
	pApplicationName =
	    OS_allocStringNCopy(
		STR_stringStart(request.hApplicationEntityInstanceName),
		(OS_Uint16)
		STR_getStringLength(request.hApplicationEntityInstanceName)
		);
	pModuleName =
	    OS_allocStringNCopy(
		STR_stringStart(request.hModuleName),
		(OS_Uint16) STR_getStringLength(request.hModuleName)
		);
	pObjectName =
	    OS_allocStringNCopy(
		STR_stringStart(request.hManagableObjectName),
		(OS_Uint16) STR_getStringLength(request.hManagableObjectName)
		);

	if ((rc = getValueByName(pApplicationName,
				 pModuleName,
				 pObjectName,
				 &objectType,
				 &value)) != Success)
	{
	    TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		      "getValue failed, module %s, reason %d\n",
		      MODID_RCPARAMS(rc)));
	    agentErrorResponse(phOperation,
			       (OS_Uint8) operationValue, rc);
	    goto cleanUp;
	}
				   
	switch(value.type)
	{
	case MM_ValueType_SignedInt:
	    if (request.value.choice != MM_VALUE_CHOICE_SIGNEDINT)
	    {
		agentErrorResponse(phOperation,
				   (OS_Uint8) operationValue,
				   MM_RC_WrongValueType);
		goto cleanUp;
	    }
	    value.un.signedInt = request.value.un.signedInt;
	    break;
		
	case MM_ValueType_UnsignedInt:
	    if (request.value.choice != MM_VALUE_CHOICE_UNSIGNEDINT)
	    {
		agentErrorResponse(phOperation,
				   (OS_Uint8) operationValue,
				   MM_RC_WrongValueType);
		goto cleanUp;
	    }
	    value.un.unsignedInt = request.value.un.unsignedInt;
	    break;
		
	case MM_ValueType_String:
	    if (request.value.choice != MM_VALUE_CHOICE_STRING)
	    {
		agentErrorResponse(phOperation,
				   (OS_Uint8) operationValue,
				   MM_RC_WrongValueType);
		goto cleanUp;
	    }
	    value.un.string = STR_stringStart(request.value.un.string);
	    break;
	}

	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "\tApplication Name = (%s)\n"
		  "\tModule Name = (%s)\n"
		  "\tObject Name = (%s)\n"
		  "\tObjectType = %d\n"
		  "\tValue Type = %s\n"
		  "\tValue = %s\n",
		  pApplicationName, pModuleName, pObjectName,
		  objectType,
		  valueTypeString(&value), valueString(&value)));

	if ((rc = setValueByName(pApplicationName,
				 pModuleName,
				 pObjectName,
				 objectType,
				 &value)) != Success)
	{
	    TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		      "setValue failed, module %s, reason %d\n",
		      MODID_RCPARAMS(rc)));
	    agentErrorResponse(phOperation,
			       (OS_Uint8) operationValue, rc);
	    goto cleanUp;
	}
	break;
	    
    case MM_RemOp_GetThreshold:
	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "Agent received GetThreshold event:\n"));
	
	pApplicationName =
	    OS_allocStringNCopy(
		STR_stringStart(request.hApplicationEntityInstanceName),
		(OS_Uint16)
		STR_getStringLength(request.hApplicationEntityInstanceName)
		);
	pModuleName =
	    OS_allocStringNCopy(
		STR_stringStart(request.hModuleName),
		(OS_Uint16) STR_getStringLength(request.hModuleName)
		);
	pObjectName =
	    OS_allocStringNCopy(
		STR_stringStart(request.hManagableObjectName),
		(OS_Uint16) STR_getStringLength(request.hManagableObjectName)
		);
	thresholdType = request.thresholdType;
	    
	if ((rc = getThresholdByName(pApplicationName,
				     pModuleName,
				     pObjectName,
				     thresholdType,
				     &objectType,
				     &value)) != Success)
	{
	    TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		      "getThreshold failed, module %s, reason %d\n",
		      MODID_RCPARAMS(rc)));
	    agentErrorResponse(phOperation,
			       (OS_Uint8) operationValue, rc);
	    goto cleanUp;
	}
				   
	switch(value.type)
	{
	case MM_ValueType_SignedInt:
	    request.value.choice = MM_VALUE_CHOICE_SIGNEDINT;
	    request.value.un.signedInt = value.un.signedInt;
	    TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		      "\t\tRetrieved Signed Int %ld\n",
		      result.value.un.signedInt));
	    break;
		
	case MM_ValueType_UnsignedInt:
	    request.value.choice = MM_VALUE_CHOICE_UNSIGNEDINT;
	    request.value.un.unsignedInt = value.un.unsignedInt;
	    TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		      "\t\tRetrieved Unsigned Int %ld\n",
		      result.value.un.unsignedInt));
	    break;

	default:
	    agentErrorResponse(phOperation,
			       (OS_Uint8) operationValue,
			       MM_RC_WrongObjectType);
	    goto cleanUp;
	}
	break;
	    
    case MM_RemOp_SetThreshold:
	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "Agent received SetThreshold event:\n"));
	
	pApplicationName =
	    OS_allocStringNCopy(
		STR_stringStart(request.hApplicationEntityInstanceName),
		(OS_Uint16)
		STR_getStringLength(request.hApplicationEntityInstanceName)
		);
	pModuleName =
	    OS_allocStringNCopy(
		STR_stringStart(request.hModuleName),
		(OS_Uint16) STR_getStringLength(request.hModuleName)
		);
	pObjectName =
	    OS_allocStringNCopy(
		STR_stringStart(request.hManagableObjectName),
		(OS_Uint16) STR_getStringLength(request.hManagableObjectName)
		);
	thresholdType = request.thresholdType;
	    
	if ((rc = getThresholdByName(pApplicationName,
				     pModuleName,
				     pObjectName,
				     thresholdType,
				     &objectType,
				     &value)) != Success)
	{
	    TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		      "getThreshold failed, module %s, reason %d\n",
		      MODID_RCPARAMS(rc)));
	    agentErrorResponse(phOperation,
			       (OS_Uint8) operationValue, rc);
	    goto cleanUp;
	}
				   
	switch(value.type)
	{
	case MM_ValueType_SignedInt:
	    if (request.value.choice != MM_VALUE_CHOICE_SIGNEDINT)
	    {
		agentErrorResponse(phOperation,
				   (OS_Uint8) operationValue,
				   MM_RC_WrongValueType);
		goto cleanUp;
	    }
	    value.un.signedInt = request.value.un.signedInt;
	    break;
		
	case MM_ValueType_UnsignedInt:
	    if (request.value.choice != MM_VALUE_CHOICE_UNSIGNEDINT)
	    {
		agentErrorResponse(phOperation,
				   (OS_Uint8) operationValue,
				   MM_RC_WrongValueType);
		goto cleanUp;
	    }
	    value.un.unsignedInt = request.value.un.unsignedInt;
	    break;
		
	default:
	    agentErrorResponse(phOperation,
			       (OS_Uint8) operationValue,
			       MM_RC_WrongObjectType);
	    goto cleanUp;
	}

	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "\tApplication Name = (%s)\n"
		  "\tModule Name = (%s)\n"
		  "\tObject Name = (%s)\n"
		  "\tObjectType = %d\n"
		  "\tThreshold Type = %s\n"
		  "\tThreshold Value Type = %s\n"
		  "\tThreshold Value = %s\n",
		  pApplicationName, pModuleName, pObjectName,
		  objectType,
		  (thresholdType == MM_ThresholdType_MaximumThreshold ?
		   "Maximum Threshold" :
		   (thresholdType == MM_ThresholdType_MinimumThreshold ?
		    "Minimum Threshold" :
		    (thresholdType == MM_ThresholdType_MaximumAbsolute ?
		     "Maximum Absolute" :
		     (thresholdType == MM_ThresholdType_MinimumAbsolute ?
		      "Minimum Absolute" :
		      "Unknown")))),
		  valueTypeString(&value), valueString(&value)));

	if ((rc = setThresholdByName(pApplicationName,
				     pModuleName,
				     pObjectName,
				     thresholdType,
				     objectType,
				     &value)) != Success)
	{
	    TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		      "setThreshold failed, module %s, reason %d\n",
		      MODID_RCPARAMS(rc)));
	    agentErrorResponse(phOperation,
			       (OS_Uint8) operationValue, rc);
	    goto cleanUp;
	}
				   
	break;
	    
    case MM_RemOp_StartTimer:
	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "Agent received StartTimer event:\n"));
	
	pApplicationName =
	    OS_allocStringNCopy(
		STR_stringStart(request.hApplicationEntityInstanceName),
		(OS_Uint16)
		STR_getStringLength(request.hApplicationEntityInstanceName)
		);
	pModuleName =
	    OS_allocStringNCopy(
		STR_stringStart(request.hModuleName),
		(OS_Uint16) STR_getStringLength(request.hModuleName)
		);
	pObjectName =
	    OS_allocStringNCopy(
		STR_stringStart(request.hManagableObjectName),
		(OS_Uint16) STR_getStringLength(request.hManagableObjectName)
		);
	milliseconds = request.milliseconds;
	    
	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "\tApplication Name = (%s)\n"
		  "\tModule Name = (%s)\n"
		  "\tObject Name = (%s)\n"
		  "\tMilliseconds = %lu\n",
		  pApplicationName, pModuleName, pObjectName, milliseconds));

	if ((rc = startTimerByName(pApplicationName,
				   pModuleName,
				   pObjectName,
				   milliseconds)) != Success)
	{
	    TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		      "startTimer failed, module %s, reason %d\n",
		      MODID_RCPARAMS(rc)));
	    agentErrorResponse(phOperation,
			       (OS_Uint8) operationValue, rc);
	    goto cleanUp;
	}
	break;
	    
    case MM_RemOp_StopTimer:
	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "Agent received StopTimer event:\n"));
	
	pApplicationName =
	    OS_allocStringNCopy(
		STR_stringStart(request.hApplicationEntityInstanceName),
		(OS_Uint16)
		STR_getStringLength(request.hApplicationEntityInstanceName)
		);
	pModuleName =
	    OS_allocStringNCopy(
		STR_stringStart(request.hModuleName),
		(OS_Uint16) STR_getStringLength(request.hModuleName)
		);
	pObjectName =
	    OS_allocStringNCopy(
		STR_stringStart(request.hManagableObjectName),
		(OS_Uint16) STR_getStringLength(request.hManagableObjectName)
		);
	    
	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "\tApplication Name = (%s)\n"
		  "\tModule Name = (%s)\n"
		  "\tObject Name = (%s)\n",
		  pApplicationName, pModuleName, pObjectName));

	if ((rc = stopTimerByName(pApplicationName,
				  pModuleName,
				  pObjectName)) != Success)
	{
	    TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		      "stopTimer failed, module %s, reason %d\n",
		      MODID_RCPARAMS(rc)));
	    agentErrorResponse(phOperation,
			       (OS_Uint8) operationValue, rc);
	    goto cleanUp;
	}
	break;
	    
    case MM_RemOp_GetApplicationEntityList:
	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "Agent received GetApplicationEntityList event:\n"));
	
	if ((rc = getApplicationEntityInstanceList(&pList)) != Success)
	{
	    TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		      "getApplicationEntityList failed, "
		      "module %s, reason %d\n",
		      MODID_RCPARAMS(rc)));
	    agentErrorResponse(phOperation,
			       (OS_Uint8) operationValue, rc);
	    goto cleanUp;
	}

	/* Scan for the terminating double-null */
	for (p = pList; *p != '\0'; p += strlen(p) + 1)
	{
	    TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		      "\t\tRetrieved (%s)\n", p));
	}
	    
	len = p - pList + 2;

	if ((rc =
	     STR_attachString(len, len,
			      pList,
			      TRUE,
			      &result.hString)) != Success)
	{
	    TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		      "STR_attachString failed, module %s, reason %d\n",
		      MODID_RCPARAMS(rc)));
	    agentErrorResponse(phOperation,
			       (OS_Uint8) operationValue, rc);
	    goto cleanUp;
	}
	break;
	    
    case MM_RemOp_GetModuleList:
	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "Agent received GetModuleList event:\n"));
	
	pApplicationName =
	    OS_allocStringNCopy(
		STR_stringStart(request.hApplicationEntityInstanceName),
		(OS_Uint16)
		STR_getStringLength(request.hApplicationEntityInstanceName)
		);

	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "\tApplication Name = (%s)\n", pApplicationName));

	if ((rc = getModuleList(pApplicationName,
				&pList)) != Success)
	{
	    TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		      "getModuleList failed, module %s, reason %d\n",
		      MODID_RCPARAMS(rc)));
	    agentErrorResponse(phOperation,
			       (OS_Uint8) operationValue, rc);
	    goto cleanUp;
	}

	/* Scan for the terminating double-null */
	for (p = pList; *p != '\0'; p += strlen(p) + 1)
	{
	    TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		      "\t\tRetrieved (%s)\n", p));
	}
	    
	len = p - pList + 2;

	if ((rc =
	     STR_attachString(len, len,
			      pList,
			      TRUE,
			      &result.hString)) != Success)
	{
	    TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		      "STR_attachString failed, module %s, reason %d\n",
		      MODID_RCPARAMS(rc)));
	    agentErrorResponse(phOperation,
			       (OS_Uint8) operationValue, rc);
	    goto cleanUp;
	}
	break;
	    
    case MM_RemOp_GetManagableObjectList:
	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "Agent received GetManagableObjectList event:\n"));
	
	pApplicationName =
	    OS_allocStringNCopy(
		STR_stringStart(request.hApplicationEntityInstanceName),
		(OS_Uint16)
		STR_getStringLength(request.hApplicationEntityInstanceName)
		);
	pModuleName =
	    OS_allocStringNCopy(
		STR_stringStart(request.hModuleName),
		(OS_Uint16) STR_getStringLength(request.hModuleName)
		);
	    
	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "\tApplication Name = (%s)\n"
		  "\tModule Name = (%s)\n",
		  pApplicationName, pModuleName));

	if ((rc = getManagableObjectList(pApplicationName,
					 pModuleName,
					 &pList)) != Success)
	{
	    TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		      "getManagableObjectList failed, module %s, reason %d\n",
		      MODID_RCPARAMS(rc)));
	    agentErrorResponse(phOperation,
			       (OS_Uint8) operationValue, rc);
	    goto cleanUp;
	}

	/* Scan for the terminating double-null */
	for (p = pList; *p != '\0'; p += strlen(p) + 1)
	{
	    TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		      "\t\tRetrieved (%s)\n", p));
	}
	    
	len = p - pList + 2;

	if ((rc =
	     STR_attachString(len, len,
			      pList,
			      TRUE,
			      &result.hString)) != Success)
	{
	    TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		      "STR_attachString failed, module %s, reason %d\n",
		      MODID_RCPARAMS(rc)));
	    agentErrorResponse(phOperation,
			       (OS_Uint8) operationValue, rc);
	    goto cleanUp;
	}
	break;
	    
    case MM_RemOp_GetManagableObjectInfo:
	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "Agent received GetManagableObjectInfo event:\n"));
	
	pApplicationName =
	    OS_allocStringNCopy(
		STR_stringStart(request.hApplicationEntityInstanceName),
		(OS_Uint16)
		STR_getStringLength(request.hApplicationEntityInstanceName)
		);
	pModuleName =
	    OS_allocStringNCopy(
		STR_stringStart(request.hModuleName),
		(OS_Uint16) STR_getStringLength(request.hModuleName)
		);
	pObjectName =
	    OS_allocStringNCopy(
		STR_stringStart(request.hManagableObjectName),
		(OS_Uint16) STR_getStringLength(request.hManagableObjectName)
		);
	    
	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "\tApplication Name = (%s)\n"
		  "\tModule Name = (%s)\n"
		  "\tObject Name = (%s)\n",
		  pApplicationName, pModuleName, pObjectName));

	if ((rc = getManagableObjectInfo(pApplicationName,
					 pModuleName,
					 pObjectName,
					 &objectType,
					 &p)) != Success)
	{
	    TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		      "getManagableObjectInfo failed, module %s, reason %d\n",
		      MODID_RCPARAMS(rc)));
	    agentErrorResponse(phOperation,
			       (OS_Uint8) operationValue, rc);
	    goto cleanUp;
	}

	result.objectType = objectType;

	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "\t\tRetrieved object type %d\n"
		  "\t\tRetrieved description (%s)\n",
		  objectType, p));

	if ((rc =
	     STR_attachZString(p,
			       TRUE,
			       &result.hString)) != Success)
	{
	    TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		      "STR_attachString failed, module %s, reason %d\n",
		      MODID_RCPARAMS(rc)));
	    agentErrorResponse(phOperation,
			       (OS_Uint8) operationValue, rc);
	    goto cleanUp;
	}
	break;
	    
    case MM_RemOp_GetManagableObjectNotifyMask:
	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "Agent received GetManagableObjectNotifyMask event:\n"));
	
	pApplicationName =
	    OS_allocStringNCopy(
		STR_stringStart(request.hApplicationEntityInstanceName),
		(OS_Uint16)
		STR_getStringLength(request.hApplicationEntityInstanceName)
		);
	pModuleName =
	    OS_allocStringNCopy(
		STR_stringStart(request.hModuleName),
		(OS_Uint16) STR_getStringLength(request.hModuleName)
		);
	pObjectName =
	    OS_allocStringNCopy(
		STR_stringStart(request.hManagableObjectName),
		(OS_Uint16) STR_getStringLength(request.hManagableObjectName)
		);
	    
	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "\tApplication Name = (%s)\n"
		  "\tModule Name = (%s)\n"
		  "\tObject Name = (%s)\n",
		  pApplicationName, pModuleName, pObjectName));

	if ((rc =
	     getManagableObjectNotifyMaskByName(pApplicationName,
						pModuleName,
						pObjectName,
						&notifyMask)) !=
	    Success)
	{
	    TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		      "getManagableObjectNotifyMask failed, "
		      "module %s, reason %d\n",
		      MODID_RCPARAMS(rc)));
	    agentErrorResponse(phOperation,
			       (OS_Uint8) operationValue, rc);
	    goto cleanUp;
	}

	result.notifyMask = notifyMask;
	
	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "\t\tRetrieved notifyMask = 0x%lx\n",
		  notifyMask));

	break;
	    
    case MM_RemOp_SetManagableObjectNotifyMask:
	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "Agent received SetManagableObjectNotifyMask event:\n"));
	
	pApplicationName =
	    OS_allocStringNCopy(
		STR_stringStart(request.hApplicationEntityInstanceName),
		(OS_Uint16)
		STR_getStringLength(request.hApplicationEntityInstanceName)
		);
	pModuleName =
	    OS_allocStringNCopy(
		STR_stringStart(request.hModuleName),
		(OS_Uint16) STR_getStringLength(request.hModuleName)
		);
	pObjectName =
	    OS_allocStringNCopy(
		STR_stringStart(request.hManagableObjectName),
		(OS_Uint16) STR_getStringLength(request.hManagableObjectName)
		);
	    
	notifyMask = request.notifyMask;

	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "\tApplication Name = (%s)\n"
		  "\tModule Name = (%s)\n"
		  "\tObject Name = (%s)\n"
		  "\tNotify Mask = 0x%lx\n",
		  pApplicationName, pModuleName, pObjectName, notifyMask));

	if ((rc =
	     setManagableObjectNotifyMaskByName(pApplicationName,
						pModuleName,
						pObjectName,
						notifyMask)) != Success)
	{
	    TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		      "setMangableObjectNotifyMask failed,"
		      "module %s, reason %d\n",
		      MODID_RCPARAMS(rc)));
	    agentErrorResponse(phOperation,
			       (OS_Uint8) operationValue, rc);
	    goto cleanUp;
	}
	break;
	    
    case MM_RemOp_GetDestinationList:
	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "Agent received GetDestinationList event:\n"));
	
	pApplicationName =
	    OS_allocStringNCopy(
		STR_stringStart(request.hApplicationEntityInstanceName),
		(OS_Uint16)
		STR_getStringLength(request.hApplicationEntityInstanceName)
		);

	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "\tApplication Name = (%s)\n", pApplicationName));

	if ((rc = getDestinationList(pApplicationName,
				     &pList)) != Success)
	{
	    TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		      "getDestinationList failed, module %s, reason %d\n",
		      MODID_RCPARAMS(rc)));
	    agentErrorResponse(phOperation,
			       (OS_Uint8) operationValue, rc);
	    goto cleanUp;
	}

	/* Scan for the terminating double-null */
	for (p = pList; *p != '\0'; p += strlen(p) + 1)
	{
	    TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		      "\t\tRetrieved (%s)\n", p));
	}
	    
	len = p - pList + 2;

	if ((rc =
	     STR_attachString(len, len,
			      pList,
			      TRUE,
			      &result.hString)) != Success)
	{
	    TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		      "STR_attachString failed, module %s, reason %d\n",
		      MODID_RCPARAMS(rc)));
	    agentErrorResponse(phOperation,
			       (OS_Uint8) operationValue, rc);
	    goto cleanUp;
	}
	break;
	    
    case MM_RemOp_GetDestinationMasks:
	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "Agent received GetDestinationMasks event:\n"));
	
	pApplicationName =
	    OS_allocStringNCopy(
		STR_stringStart(request.hApplicationEntityInstanceName),
		(OS_Uint16)
		STR_getStringLength(request.hApplicationEntityInstanceName)
		);
	pDestinationName =
	    OS_allocStringNCopy(
		STR_stringStart(request.hDestinationName),
		(OS_Uint16) STR_getStringLength(request.hDestinationName)
		);

	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "\tApplication Name = (%s)\n"
		  "\tDestination Name = (%s)\n",
		  pApplicationName, pDestinationName));

	if ((rc = getDestinationMasksByName(pApplicationName,
					    pDestinationName,
					    &notifyMask,
					    &eventMask)) != Success)
	{
	    TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		      "getDestinationMasks failed, module %s, reason %d\n",
		      MODID_RCPARAMS(rc)));
	    agentErrorResponse(phOperation,
			       (OS_Uint8) operationValue, rc);
	    goto cleanUp;
	}

	result.notifyMask = notifyMask;
	result.eventMask = eventMask;

	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "\t\tRetrieved notifyMask = 0x%lx\n"
		  "\t\tRetrieved eventMask = 0x%lx\n",
		  notifyMask, eventMask));

	break;
	    
    case MM_RemOp_SetDestinationMasks:
	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "Agent received SetDestinationMasks event:\n"));
	
	pApplicationName =
	    OS_allocStringNCopy(
		STR_stringStart(request.hApplicationEntityInstanceName),
		(OS_Uint16)
		STR_getStringLength(request.hApplicationEntityInstanceName)
		);
	pDestinationName =
	    OS_allocStringNCopy(
		STR_stringStart(request.hDestinationName),
		(OS_Uint16) STR_getStringLength(request.hDestinationName)
		);

	notifyMask = result.notifyMask;
	eventMask = result.eventMask;

	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "\tApplication Name = (%s)\n"
		  "\tModule Name = (%s)\n"
		  "\tObject Name = (%s)\n"
		  "\tNotify Mask = 0x%lx\n"
		  "\tEvent Mask = 0x%lx\n",
		  pApplicationName, pModuleName, pObjectName,
		  notifyMask, eventMask));

	if ((rc = setDestinationMasksByName(pApplicationName,
					    pDestinationName,
					    notifyMask,
					    eventMask)) != Success)
	{
	    TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		      "setDestinationMasks failed, module %s, reason %d\n",
		      MODID_RCPARAMS(rc)));
	    agentErrorResponse(phOperation,
			       (OS_Uint8) operationValue, rc);
	    goto cleanUp;
	}
	break;

    default:
	break;
    }

    /* If there's a Result PDU to be formatted... */
    if (mm_protocols[operationValue].pfCompRes != NULL)
    {
	/* ... allocate a buffer into which we will format, ... */
	if ((rc = BUF_alloc(0, &hResultBuf)) != Success)
	{
	    TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		      "BUF_alloc failed, module %s, reason %d\n",
		      MODID_RCPARAMS(rc)));
	    goto cleanUp;
	}

	/* ... and format it. */
	if ((rc = ASN_format(ASN_EncodingRules_Basic,
			     &mm_protocols[operationValue].resultQ,
			     hResultBuf,
			     &result,
			     &pduLen)) != Success)
	{
	    TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		      "ASN_format failed, module %s, reason %d\n",
		      MODID_RCPARAMS(rc)));
	    goto cleanUp;
	}
    }
    else
    {
	/* ... otherwise, specify that there's no result PDU. */
	hResultBuf = NULL;
    }

    /* Issue the result operation. */
    if ((rc = (* pfRemoteOperationSend)(phOperation,
					MM_RemOpType_Result,
					operationValue,
					&hResultBuf)) != Success)
    {
	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "Remote Operation Send failed, module %s, reason %d\n",
		  MODID_RCPARAMS(rc)));
	goto cleanUp;
    }

    TM_CALL(mm_hModCB, MM_TM_PDU, BUF_dump, hResultBuf, "Agent Response");
    TM_TRACE((mm_hModCB, MM_TM_ACTIVITY, "Successful completion.\n"));

  cleanUp:
	
    if (hString != NULL)
    {
	STR_free(hString);
    }
	
    if (hResultBuf != NULL)
    {
	BUF_free(hResultBuf);
    }

    if (pApplicationName != NULL)
    {
	OS_free(pApplicationName);
    }

    if (pModuleName != NULL)
    {
	OS_free(pModuleName);
    }

    if (pObjectName != NULL)
    {
	OS_free(pObjectName);
    }

    for (i=0; i<2; i++)
    {
	if (protocol[i].hApplicationEntityInstanceName != NULL)
	{
	    STR_free(protocol[i].hApplicationEntityInstanceName);
	}

	if (protocol[i].hModuleName != NULL)
	{
	    STR_free(protocol[i].hModuleName);
	}

	if (protocol[i].hManagableObjectName != NULL)
	{
	    STR_free(protocol[i].hManagableObjectName);
	}

	if (protocol[i].hDestinationName != NULL)
	{
	    STR_free(protocol[i].hDestinationName);
	}

	if (protocol[i].hString != NULL)
	{
	    STR_free(protocol[i].hString);
	}
    }

#undef request
#undef result
}


/*
 * MM_agentDestination()
 *
 * This function may be used as a Destination, to send events to a remote
 * Module Management manager.
 */
void
MM_agentDestination(char * pDestinationName,
		    char * pApplicationEntityInstanceName,
		    char * pModuleName,
		    char * pObjectName,
		    char * pDescription,
		    MM_EventType eventType,
		    ...)
{
    char * 			p;
    void *			hBuf = NULL;
    void *			hOperation = NULL;
    va_list 			pArgs;
    MM_Protocol 		protocol;
    ReturnCode			rc = Success;
    MM_RemoteOperationValue	event = MM_EVENT_ENCODE(eventType);

    TM_TRACE((mm_hModCB, MM_TM_ERROR,
	      "Agent Event to Manager:\n"
	      "\tApplication Entity Instance Name = (%s)\n"
	      "\tModule Name = (%s)\n"
	      "\tObject Name = (%s)\n"
	      "\tDescription = (%s)\n"
	      "\tEventType = %d\n",
	      pApplicationEntityInstanceName,
	      pModuleName,
	      pObjectName,
	      pDescription,
	      eventType));

    /* Zero out the protocol struture to make it easy to clean up later */
    OS_memSet((void *) &protocol, '\0', sizeof(MM_Protocol));

    /* Get the application entity instance name */
    if ((rc =
	 STR_attachZString(pApplicationEntityInstanceName,
			   FALSE,
			   &protocol.hApplicationEntityInstanceName)) !=
	Success)
    {
	goto cleanUp;
    }
    
    /* Get the module name */
    if ((rc = STR_attachZString(pModuleName,
				FALSE,
				&protocol.hModuleName)) != Success)
    {
	goto cleanUp;
    }
    
    /* Get the managable object name */
    if ((rc = STR_attachZString(pObjectName,
				FALSE,
				&protocol.hManagableObjectName)) !=
	Success)
    {
	goto cleanUp;
    }

    va_start(pArgs, eventType);

    switch(eventType)
    {
    case MM_EventType_MaxThresholdExceededSigned:
	/*
	 * Parameters passed to the Alert function when an event of this
	 * type is raised:
	 *
	 *     Two optional parameters are passed:
	 * 
	 *           - the threshold value, as an "OS_Sint32"
	 *           - the value which caused the event by exceeding the
	 *             threshold, as an "OS_Sint32"
	 *
	 */
	protocol.value.choice = MM_VALUE_CHOICE_SIGNEDINT;
	protocol.value2.choice = MM_VALUE_CHOICE_SIGNEDINT;
	protocol.value.un.signedInt = va_arg(pArgs, OS_Sint32);
	protocol.value2.un.signedInt = va_arg(pArgs, OS_Sint32);

	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		 "Event: Maximum Threshold Exceeded (Signed): "
		  "threshold=%ld  value=%ld\n",
		  protocol.value.un.signedInt,
		  protocol.value2.un.signedInt));
	break;

    case MM_EventType_MaxThresholdExceededUnsigned:
	/*
	 * Parameters passed to the Alert function when an event of this
	 * type is raised:
	 *
	 *     Two optional parameters are passed:
	 * 
	 *           - the threshold value, as an "OS_Uint32"
	 *           - the value which caused the event by exceeding the
	 *             threshold, as an "OS_Uint32"
	 *
	 */
	protocol.value.choice = MM_VALUE_CHOICE_UNSIGNEDINT;
	protocol.value2.choice = MM_VALUE_CHOICE_UNSIGNEDINT;
	protocol.value.un.unsignedInt = va_arg(pArgs, OS_Uint32);
	protocol.value2.un.unsignedInt = va_arg(pArgs, OS_Uint32);
	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "Event: Maximum Threshold Exceeded (Unsigned): "
		  "threshold=%lu  value=%lu\n",
		  protocol.value.un.unsignedInt,
		  protocol.value2.un.unsignedInt));
	break;

    case MM_EventType_MinThresholdExceededSigned:
	/*
	 * Parameters passed to the Alert function when an event of this
	 * type is raised:
	 *
	 *     Two optional parameters are passed:
	 * 
	 *           - the threshold value, as an "OS_Sint32"
	 *           - the value which caused the event by exceeding the
	 *             threshold, as an "OS_Sint32"
	 *
	 */
	protocol.value.choice = MM_VALUE_CHOICE_SIGNEDINT;
	protocol.value2.choice = MM_VALUE_CHOICE_SIGNEDINT;
	protocol.value.un.signedInt = va_arg(pArgs, OS_Sint32);
	protocol.value2.un.signedInt = va_arg(pArgs, OS_Sint32);
	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "Event: Minimum Threshold Exceeded (Signed): "
		  "threshold=%ld  value=%ld\n",
		  protocol.value.un.signedInt,
		  protocol.value2.un.signedInt));
	break;
	
    case MM_EventType_MinThresholdExceededUnsigned:
	/*
	 * Parameters passed to the Alert function when an event of this
	 * type is raised:
	 *
	 *     Two optional parameters are passed:
	 * 
	 *           - the threshold value, as an "OS_Uint32"
	 *           - the value which caused the event by exceeding the
	 *             threshold, as an "OS_Uint32"
	 *
	 */
	protocol.value.choice = MM_VALUE_CHOICE_UNSIGNEDINT;
	protocol.value2.choice = MM_VALUE_CHOICE_UNSIGNEDINT;
	protocol.value.un.unsignedInt = va_arg(pArgs, OS_Uint32);
	protocol.value2.un.unsignedInt = va_arg(pArgs, OS_Uint32);
	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "Event: Minimum Threshold Exceeded Unsigned: "
		  "threshold=%lu  value=%lu\n",
		  protocol.value.un.unsignedInt,
		  protocol.value2.un.unsignedInt));
	break;

    case MM_EventType_MaxThresholdReenteredSigned:
	/*
	 * Parameters passed to the Alert function when an event of this
	 * type is raised:
	 *
	 *     Two optional parameters are passed:
	 * 
	 *           - the threshold value, as an "OS_Sint32"
	 *           - the value which caused the event by reentering the
	 *             threshold, as an "OS_Sint32"
	 *
	 */
	protocol.value.choice = MM_VALUE_CHOICE_SIGNEDINT;
	protocol.value2.choice = MM_VALUE_CHOICE_SIGNEDINT;
	protocol.value.un.signedInt = va_arg(pArgs, OS_Sint32);
	protocol.value2.un.signedInt = va_arg(pArgs, OS_Sint32);
	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "Event: Maximum Threshold Reentered (Signed): "
		  "threshold=%ld  value=%ld\n",
		  protocol.value.un.signedInt,
		  protocol.value2.un.signedInt));
	break;

    case MM_EventType_MaxThresholdReenteredUnsigned:
	/*
	 * Parameters passed to the Alert function when an event of this
	 * type is raised:
	 *
	 *     Two optional parameters are passed:
	 * 
	 *           - the threshold value, as an "OS_Uint32"
	 *           - the value which caused the event by reentering the
	 *             threshold, as an "OS_Uint32"
	 *
	 */
	protocol.value.choice = MM_VALUE_CHOICE_UNSIGNEDINT;
	protocol.value2.choice = MM_VALUE_CHOICE_UNSIGNEDINT;
	protocol.value.un.unsignedInt = va_arg(pArgs, OS_Uint32);
	protocol.value2.un.unsignedInt = va_arg(pArgs, OS_Uint32);
	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "Event: Maximum Threshold Reentered Unsigned: "
		  "threshold=%lu  value=%lu\n",
		  protocol.value.un.unsignedInt,
		  protocol.value2.un.unsignedInt));
	break;

    case MM_EventType_MinThresholdReenteredSigned:
	/*
	 * Parameters passed to the Alert function when an event of this
	 * type is raised:
	 *
	 *     Two optional parameters are passed:
	 * 
	 *           - the threshold value, as an "OS_Sint32"
	 *           - the value which caused the event by reentering the
	 *             threshold, as an "OS_Sint32"
	 *
	 */
	protocol.value.choice = MM_VALUE_CHOICE_SIGNEDINT;
	protocol.value2.choice = MM_VALUE_CHOICE_SIGNEDINT;
	protocol.value.un.signedInt = va_arg(pArgs, OS_Sint32);
	protocol.value2.un.signedInt = va_arg(pArgs, OS_Sint32);
	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "Event: Minimum Threshold Reentered (Signed): "
		  "threshold=%ld  value=%ld\n",
		  protocol.value.un.signedInt,
		  protocol.value2.un.signedInt));
	break;

    case MM_EventType_MinThresholdReenteredUnsigned:
	/*
	 * Parameters passed to the Alert function when an event of this
	 * type is raised:
	 *
	 *     Two optional parameters are passed:
	 * 
	 *           - the threshold value, as an "OS_Uint32"
	 *           - the value which caused the event by reentering the
	 *             threshold, as an "OS_Uint32"
	 *
	 */
	protocol.value.choice = MM_VALUE_CHOICE_UNSIGNEDINT;
	protocol.value2.choice = MM_VALUE_CHOICE_UNSIGNEDINT;
	protocol.value.un.unsignedInt = va_arg(pArgs, OS_Uint32);
	protocol.value2.un.unsignedInt = va_arg(pArgs, OS_Uint32);
	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "Event: Minimum Threshold Reentered Unsigned: "
		  "threshold=%lu  value=%lu\n",
		  protocol.value.un.unsignedInt,
		  protocol.value2.un.unsignedInt));
	break;

    case MM_EventType_TimerExpired:
	/*
	 * Parameters passed to the Alert function when an event of this
	 * type is raised:
	 *
	 *     No optional parameters are passed.
	 */
	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "Event: Timer expired\n"));
	break;
	
    case MM_EventType_LogMessage:
	/*
	 * Parameters passed to the Alert function when an event of this
	 * type is raised:
	 *
	 *     One optional parameter is passed:
	 *
	 *		- the log message string, as a "char *"
	 */

	protocol.value.choice = MM_VALUE_CHOICE_STRING;

	p = va_arg(pArgs, char *);
	if ((rc = STR_attachZString(p, FALSE,
				    &protocol.value.un.string)) != Success)
	{
	    goto cleanUp;
	}

	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "Event: Log Message: (%s)\n", p));
	break;
	
    case MM_EventType_ValueChangedSigned:
	/*
	 * Parameters passed to the Alert function when an event of this
	 * type is raised:
	 *
	 *     One optional parameter is passed:
	 * 
	 *           - the new value which caused the event, as an "OS_Sint32"
	 */
	protocol.value.choice = MM_VALUE_CHOICE_SIGNEDINT;
	protocol.value.un.signedInt = va_arg(pArgs, OS_Sint32);
	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "Event: Value Changed (Signed): new value=%ld\n",
		  protocol.value.un.signedInt));
	break;
	
    case MM_EventType_ValueChangedUnsigned:
	/*
	 * Parameters passed to the Alert function when an event of this
	 * type is raised:
	 *
	 *     One optional parameter is passed:
	 * 
	 *           - the new value which caused the event, as an "OS_Uint32"
	 */
	protocol.value.choice = MM_VALUE_CHOICE_UNSIGNEDINT;
	protocol.value.un.unsignedInt = va_arg(pArgs, OS_Uint32);
	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "Event: Value Changed (Unsigned): new value=%lu\n",
		  protocol.value.un.unsignedInt));
	break;

    case MM_EventType_ValueChangedString:
	/*
	 * Parameters passed to the Alert function when an event of this
	 * type is raised:
	 *
	 *     One optional parameter is passed:
	 * 
	 *           - the new value which caused the event, as a "char *"
	 */
	protocol.value.choice = MM_VALUE_CHOICE_STRING;

	p = va_arg(pArgs, char *);
	if ((rc = STR_attachZString(p, FALSE,
				    &protocol.value.un.string)) != Success)
	{
	    goto cleanUp;
	}

	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "Event: Value Changed (String): new value=(%s)\n", p));
	break;
	
    case MM_EventType_ManagableObjectChange:
	/*
	 * Parameters passed to the Alert function when an event of this
	 * type is raised:
	 *
	 * This event is only generated once per call to
	 * MM_processEvents().  This allows for a number of managable
	 * objects to be created, with only one event telling the manager
	 * application to re-read its list of managable object names.
	 *
	 *     No optional parameters are passed.
	 */
	TM_TRACE((mm_hModCB, MM_TM_ACTIVITY,
		  "Event: Managable Object List Changed\n"));
	break;
    }

    va_end(pArgs);

    /* Allocate a buffer for the Event PDU */
    if ((rc = BUF_alloc(0, &hBuf)) != Success)
    {
	goto cleanUp;
    }

    /* Format the Event PDU */
    if ((rc = formatEventPdu(MM_RemOpType_Event,
			     event,
			     &protocol, &hBuf)) != Success)
    {
	goto cleanUp;
    }

    (void) (* pfRemoteOperationSend)(&hOperation,
				     MM_RemOpType_Event,
				     event,
				     &hBuf);

  cleanUp:

    if (rc != Success)
    {
	TM_TRACE((mm_hModCB, MM_TM_ERROR,
		  "Error sending agent event to manager,"
		  "module %s, reason %s\n",
		  MODID_RCPARAMS(rc)));
    }

    /* Free the buffer */
    if (hBuf != NULL)
    {
	BUF_free(hBuf);
    }

    cleanUp(&protocol);
}


static void
agentErrorResponse(void ** phOperation,
		   OS_Uint8 operationValue,
		   ReturnCode rc)
{
    int		    i;
    void * 	    hBuf;
    OS_Uint16	    errorVal = rc;

    /* Allocate a buffer for the error PDU */
    if ((rc = BUF_alloc(0, &hBuf)) != Success)
    {
	return;
    }

    for (i=0; i<2; i++)
    {
	if ((rc = BUF_addOctet(hBuf, (OS_Uint8) (errorVal & 0xff))) != Success)
	{
	    BUF_free(hBuf);
	    return;
	}

	errorVal >>= 8;
    }

    (void) (* pfRemoteOperationSend)(phOperation,
				     MM_RemOpType_Error,
				     operationValue,
				     &hBuf);

    BUF_free(hBuf);
}



static ReturnCode
formatEventPdu(MM_RemoteOperationType operationType,
	       MM_RemoteOperationValue event,
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
	    return FAIL_RC(rc,
			   ("formatEventPdu(): could not allocate buffer"));
	}

	*phBuf = hBuf;
    }

    /* Format the PDU. */
    if ((rc = ASN_format(ASN_EncodingRules_Basic,
			 &mm_eventProtocols[MM_EVENT_DECODE(event)].eventQ,
			 hBuf,
			 p,
			 &pduLen)) != Success)
    {
	OS_free(hBuf);
	return FAIL_RC(rc, ("formatEventPdu(): ASN_format\n"));
    }

    return Success;
}


static void
cleanUp(MM_Protocol * p)
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


#ifdef TM_ENABLED

static char *
valueTypeString(MM_Value * pValue)
{
    switch(pValue->type)
    {
    case MM_ValueType_SignedInt:
	return "Signed Int";
	
    case MM_ValueType_UnsignedInt:
	return "Unsigned Int";
	
    case MM_ValueType_String:
	return "String";
    }

    return "Unknown value type";
}


static char *
valueString(MM_Value * pValue)
{
    static char 		buf[64];

    switch(pValue->type)
    {
    case MM_ValueType_SignedInt:
	sprintf(buf, "%ld", pValue->un.signedInt);
	return buf;
	
    case MM_ValueType_UnsignedInt:
	sprintf(buf, "%lu", pValue->un.unsignedInt);
	return buf;
	
    case MM_ValueType_String:
	return pValue->un.string;
    }

    return "";
}

#endif /* TM_ENABLED */
