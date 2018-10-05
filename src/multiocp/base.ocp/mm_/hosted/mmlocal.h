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
 * Module Management -- Local structures
 */

#ifndef __MMLOCAL_H__
#define	__MMLOCAL_H__

#include "estd.h"
#include "queue.h"
#include "asn.h"
#include "strfunc.h"
#include "rc.h"
#include "mm.h"

extern TM_ModuleCB *	mm_hModCB;


typedef struct ApplicationEntityInstance
{
    QU_ELEMENT;

    char * 			pName;
    QU_Head 			modules;
    QU_Head 			pendingEvents;
    QU_Head 			destinations;
} ApplicationEntityInstance;


typedef struct Module
{
    QU_ELEMENT;

    char * 			pName;
    ApplicationEntityInstance *	pApplicationEntityInstance;
    QU_Head 			managableObjects;
} Module;


typedef enum ValueType
{
    ValueType_Sint32,
    ValueType_Uint32,
    ValueType_String,
    ValueType_Handle,
    ValueType_None
} ValueType;


typedef struct ManagableObject
{
    QU_ELEMENT;

    char *			pName;
    char *			pDescription;
    OS_Uint32			notificationTypes;
    MM_ManagableObjectType	objectType;
    Module *			pModule;
    OS_Uint8			accessByName;
    void		     (* pfValueChanged)(void * hUserData,
						void * hManagableObject,
						MM_Value * pValue);
    void *			hUserData;

    ValueType			valueType;

    union
    {
	OS_Sint32		    sint32;
	OS_Uint32		    uint32;
	char *			    string;
	void *			    handle;
    } value;

    union
    {
	OS_Sint32		    sint32;
	OS_Uint32		    uint32;
    } minThreshold;

    union
    {
	OS_Sint32		    sint32;
	OS_Uint32		    uint32;
    } maxThreshold;

    union
    {
	OS_Sint32		    sint32;
	OS_Uint32		    uint32;
    } minAbsolute;

    union
    {
	OS_Sint32		    sint32;
	OS_Uint32		    uint32;
    } maxAbsolute;

} ManagableObject;


typedef struct Event
{
    QU_ELEMENT;

    char *			pModuleName;
    char *			pObjectName;
    char *			pDescription;
    OS_Uint32			notificationTypes;
    MM_EventType		eventType;
    union
    {
	OS_Sint32		    sint32;
	OS_Uint32		    uint32;
	char *			    string;
    }				param1;
    union
    {
	OS_Sint32		    sint32;
	OS_Uint32		    uint32;
	char *			    string;
    }				param2;
} Event;


typedef struct Destination
{
    QU_ELEMENT;

    void	     (* pfAlert)(char * pDestinationName,
				 char * pApplicationEntityInstanceName,
				 char * pModuleName,
				 char * pManagableObjectName,
				 char * pIdentificationMessage,
				 MM_EventType eventType,
				 ...);
    char *		pName;
    char *		pDescription;
    OS_Uint8	        accessByName;
    OS_Uint32 		notifyMask;
    OS_Uint32		eventMask;
} Destination;


typedef struct Protocols
{
    ReturnCode	    (* pfCompReq)(unsigned char * pCStruct,
				  OS_Boolean * pExists,
				  MM_Protocol * p,
				  OS_Uint8 tag,
				  QU_Head * pQ);
    ReturnCode	    (* pfCompRes)(unsigned char * pCStruct,
				  OS_Boolean * pExists,
				  MM_Protocol * p,
				  OS_Uint8 tag,
				  QU_Head * pQ);
    QU_Head	    requestQ;
    QU_Head	    resultQ;
} Protocols;


typedef struct EventProtocols
{
    ReturnCode	    (* pfComp)(unsigned char * pCStruct,
				  OS_Boolean * pExists,
				  MM_Protocol * p,
				  OS_Uint8 tag,
				  QU_Head * pQ);
    QU_Head	    eventQ;
} EventProtocols;


extern Protocols	mm_protocols[];
extern EventProtocols	mm_eventProtocols[];


ReturnCode
mm_asnInit(void);

QU_Head *
mm_getApplicationEntityInstances(void);

ApplicationEntityInstance *
mm_findApplicationEntityInstance(char * pApplicationEntityInstanceName);


Module *
mm_findModule(char * pModuleName,
	      ApplicationEntityInstance * pApplicationEntityInstance);


ManagableObject *
mm_findObject(char * pManagableObjectName,
	      Module * pModule);

Destination *
mm_findDestination(char * pDestinationName,
		   ApplicationEntityInstance * pApplicationEntityInstance);

void
mm_cleanUp(MM_Protocol * p);



#endif /* __MMLOCAL_H__ */
