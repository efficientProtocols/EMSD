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
 * Compile a Module Management operation for ASN.1 compiling
 */

#include "estd.h"
#include "asn.h"
#include "mmlocal.h"


static ReturnCode
compileValue(unsigned char * pCStruct,
	     OS_Boolean * pExists,
	     MM_ASNValue * pValue,
	     OS_Uint8 tag,
	     QU_Head * pQ);




ReturnCode
MM_compileGetValueRequest(unsigned char * pCStruct,
			  OS_Boolean * pExists,
			  MM_Protocol * p,
			  OS_Uint8 tag,
			  QU_Head * pQ)
{
    ASN_TableEntry * 	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "GetValueRequest sequence",
		  ("MM_compileGetValueRequest: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the application entity instance name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hApplicationEntityInstanceName,
		  "GetValueRequest application entity instance name",
		  ("MM_compileGetValueRequest: "
		   "application entity instance name"));
    
    /* Create a table entry for the module name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hModuleName,
		  "GetValueRequest module name",
		  ("MM_compileGetValueRequest: "
		   "module name"));
    
    /* Create a table entry for the managable object name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hManagableObjectName,
		  "GetValueRequest managable object name",
		  ("MM_compileGetValueRequest: "
		   "managable object name"));

    return Success;
}


ReturnCode
MM_compileGetValueResult(unsigned char * pCStruct,
			 OS_Boolean * pExists,
			 MM_Protocol * p,
			 OS_Uint8 tag,
			 QU_Head * pQ)
{
    ReturnCode		rc;
    ASN_TableEntry * 	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "GetValueResult sequence",
		  ("MM_compileGetValueResult: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Add the value */
    RC_CALL(rc,
	    compileValue((unsigned char *) pCStruct,
			 NULL,
			 &p->value,
			 0, pQ),
	    ("MM_compileGetValueResult: value"));
    

    return Success;
}


ReturnCode
MM_compileSetValueRequest(unsigned char * pCStruct,
			  OS_Boolean * pExists,
			  MM_Protocol * p,
			  OS_Uint8 tag,
			  QU_Head * pQ)
{
    ReturnCode		rc;
    ASN_TableEntry * 	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "SetValueRequest sequence",
		  ("MM_compileSetValueRequest: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the application entity instance name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hApplicationEntityInstanceName,
		  "SetValueRequest application entity instance name",
		  ("MM_compileSetValueRequest: "
		   "application entity instance name"));
    
    /* Create a table entry for the module name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hModuleName,
		  "SetValueRequest module name",
		  ("MM_compileSetValueRequest: "
		   "module name"));
    
    /* Create a table entry for the managable object name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hManagableObjectName,
		  "SetValueRequest managable object name",
		  ("MM_compileSetValueRequest: "
		   "managable object name"));

    /* Add the value */
    RC_CALL(rc,
	    compileValue((unsigned char *) pCStruct,
			 NULL,
			 &p->value,
			 0, pQ),
	    ("MM_compileGetValueResult: value"));
    
    return Success;
}


ReturnCode
MM_compileGetThresholdRequest(unsigned char * pCStruct,
			      OS_Boolean * pExists,
			      MM_Protocol * p,
			      OS_Uint8 tag,
			      QU_Head * pQ)
{
    ASN_TableEntry * 	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "GetThresholdRequest sequence",
		  ("MM_compileGetThresholdRequest: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the application entity instance name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hApplicationEntityInstanceName,
		  "GetThresholdRequest application entity instance name",
		  ("MM_compileGetThresholdRequest: "
		   "application entity instance name"));
    
    /* Create a table entry for the module name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hModuleName,
		  "GetThresholdRequest module name",
		  ("MM_compileGetThresholdRequest: "
		   "module name"));
    
    /* Create a table entry for the managable object name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hManagableObjectName,
		  "GetThresholdRequest managable object name",
		  ("MM_compileGetThresholdRequest: "
		   "managable object name"));

    /* Create a table entry for the threshold type */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer,
		  ASN_UniversalTag_Integer, pQ,
		  pCStruct,
		  NULL,
		  &p->thresholdType,
		  "GetThresholdRequest threshold type",
		  ("MM_compileGetThresholdRequest: "
		   "application threshold type"));

    return Success;
}


ReturnCode
MM_compileGetThresholdResult(unsigned char * pCStruct,
			     OS_Boolean * pExists,
			     MM_Protocol * p,
			     OS_Uint8 tag,
			     QU_Head * pQ)
{
    ReturnCode		rc;
    ASN_TableEntry * 	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "GetThresholdResult sequence",
		  ("MM_compileGetThresholdResult: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Add the value */
    RC_CALL(rc,
	    compileValue((unsigned char *) pCStruct,
			 NULL,
			 &p->value,
			 0, pQ),
	    ("MM_compileGetThresholdResult: value"));

    return Success;
}


ReturnCode
MM_compileSetThresholdRequest(unsigned char * pCStruct,
			      OS_Boolean * pExists,
			      MM_Protocol * p,
			      OS_Uint8 tag,
			      QU_Head * pQ)
{
    ASN_TableEntry * 	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "SetThresholdRequest sequence",
		  ("MM_compileSetThresholdRequest: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the application entity instance name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hApplicationEntityInstanceName,
		  "SetThresholdRequest application entity instance name",
		  ("MM_compileSetThresholdRequest: "
		   "application entity instance name"));
    
    /* Create a table entry for the module name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hModuleName,
		  "SetThresholdRequest module name",
		  ("MM_compileSetThresholdRequest: "
		   "module name"));
    
    /* Create a table entry for the managable object name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hManagableObjectName,
		  "SetThresholdRequest managable object name",
		  ("MM_compileSetThresholdRequest: "
		   "managable object name"));

    /* Create a table entry for the threshold type */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer,
		  ASN_UniversalTag_Integer, pQ,
		  pCStruct,
		  NULL,
		  &p->thresholdType,
		  "SetThresholdRequest threshold type",
		  ("MM_compileSetThresholdRequest: "
		   "threshold type"));

    /* Create a table entry for the threshold value */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer,
		  ASN_UniversalTag_Integer, pQ,
		  pCStruct,
		  NULL,
		  &p->value,
		  "SetThresholdRequest threshold value",
		  ("MM_compileSetThresholdRequest: "
		   "threshold value"));

    return Success;
}


ReturnCode
MM_compileStartTimerRequest(unsigned char * pCStruct,
			    OS_Boolean * pExists,
			    MM_Protocol * p,
			    OS_Uint8 tag,
			    QU_Head * pQ)
{
    ASN_TableEntry * 	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "StartTimerRequest sequence",
		  ("MM_compileStartTimerRequest: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the application entity instance name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hApplicationEntityInstanceName,
		  "StartTimerRequest application entity instance name",
		  ("MM_compileStartTimerRequest: "
		   "application entity instance name"));
    
    /* Create a table entry for the module name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hModuleName,
		  "StartTimerRequest module name",
		  ("MM_compileStartTimerRequest: "
		   "module name"));
    
    /* Create a table entry for the managable object name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hManagableObjectName,
		  "StartTimerRequest managable object name",
		  ("MM_compileStartTimerRequest: "
		   "managable object name"));

    /* Create a table entry for the number of milliseconds */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer,
		  ASN_UniversalTag_Integer, pQ,
		  pCStruct,
		  NULL,
		  &p->milliseconds,
		  "StartTimerRequest milliseconds",
		  ("MM_compileStartTimerRequest: "
		   "milliseconds"));

    return Success;
}


ReturnCode
MM_compileStopTimerRequest(unsigned char * pCStruct,
			   OS_Boolean * pExists,
			   MM_Protocol * p,
			   OS_Uint8 tag,
			   QU_Head * pQ)
{
    ASN_TableEntry * 	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "StopTimerRequest sequence",
		  ("MM_compileStopTimerRequest: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the application entity instance name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hApplicationEntityInstanceName,
		  "StopTimerRequest application entity instance name",
		  ("MM_compileStopTimerRequest: "
		   "application entity instance name"));
    
    /* Create a table entry for the module name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hModuleName,
		  "StopTimerRequest module name",
		  ("MM_compileStopTimerRequest: "
		   "module name"));
    
    /* Create a table entry for the managable object name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hManagableObjectName,
		  "StopTimerRequest managable object name",
		  ("MM_compileStopTimerRequest: "
		   "managable object name"));

    return Success;
}


ReturnCode
MM_compileGetModuleListRequest(unsigned char * pCStruct,
			       OS_Boolean * pExists,
			       MM_Protocol * p,
			       OS_Uint8 tag,
			       QU_Head * pQ)
{
    ASN_TableEntry * 	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "GetModuleListRequest sequence",
		  ("MM_compileGetModuleListRequest: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the application entity instance name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hApplicationEntityInstanceName,
		  "GetModuleListRequest application entity instance name",
		  ("MM_compileGetModuleListRequest: "
		   "application entity instance name"));
    
    return Success;
}


ReturnCode
MM_compileGetApplicationEntityListResult(unsigned char * pCStruct,
					 OS_Boolean * pExists,
					 MM_Protocol * p,
					 OS_Uint8 tag,
					 QU_Head * pQ)
{
    ASN_TableEntry * 	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "GetApplicationEntityListResult sequence",
		  ("MM_compileGetApplicationEntityListResult: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the module list */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hString,
		  "GetApplicationEntityListResult module list",
		  ("MM_compileGetApplicationEntityListResult: "
		   "application entity list"));
    
    return Success;
}


ReturnCode
MM_compileGetModuleListResult(unsigned char * pCStruct,
			      OS_Boolean * pExists,
			      MM_Protocol * p,
			      OS_Uint8 tag,
			      QU_Head * pQ)
{
    ASN_TableEntry * 	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "GetModuleListResult sequence",
		  ("MM_compileGetModuleListResult: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the module list */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hString,
		  "GetModuleListResult module list",
		  ("MM_compileGetModuleListResult: "
		   "application entity module list"));
    
    return Success;
}


ReturnCode
MM_compileGetManagableObjectListRequest(unsigned char * pCStruct,
					OS_Boolean * pExists,
					MM_Protocol * p,
					OS_Uint8 tag,
					QU_Head * pQ)
{
    ASN_TableEntry * 	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "GetManagableObjectListRequest sequence",
		  ("MM_compileGetManagableObjectListRequest: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the application entity instance name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hApplicationEntityInstanceName,
		  "GetManagableObjectListRequest "
		  "application entity instance name",
		  ("MM_compileGetManagableObjectListRequest: "
		   "application entity instance name"));
    
    /* Create a table entry for the module name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hModuleName,
		  "GetManagableObjectListRequest module name",
		  ("MM_compileGetManagableObjectListRequest: module name"));
    
    return Success;
}


ReturnCode
MM_compileGetManagableObjectListResult(unsigned char * pCStruct,
				       OS_Boolean * pExists,
				       MM_Protocol * p,
				       OS_Uint8 tag,
				       QU_Head * pQ)
{
    ASN_TableEntry * 	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "GetManagableObjectListResult sequence",
		  ("MM_compileGetManagableObjectListResult: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the module list */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hString,
		  "GetManagableObjectListResult module list",
		  ("MM_compileGetManagableObjectListResult: "
		   "application entity module list"));
    
    return Success;
}



ReturnCode
MM_compileGetManagableObjectInfoRequest(unsigned char * pCStruct,
					OS_Boolean * pExists,
					MM_Protocol * p,
					OS_Uint8 tag,
					QU_Head * pQ)
{
    ASN_TableEntry * 	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "GetManagableObjectInfoRequest sequence",
		  ("MM_compileGetManagableObjectInfoRequest: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the application entity instance name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hApplicationEntityInstanceName,
		  "GetManagableObjectInfoRequest "
		  "application entity instance name",
		  ("MM_compileGetManagableObjectInfoRequest: "
		   "application entity instance name"));
    
    /* Create a table entry for the module name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hModuleName,
		  "GetManagableObjectInfoRequest module name",
		  ("MM_compileGetManagableObjectInfoRequest: "
		   "module name"));
    
    /* Create a table entry for the managable object name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hManagableObjectName,
		  "GetManagableObjectInfoRequest managable object name",
		  ("MM_compileGetManagableObjectInfoRequest: "
		   "managable object name"));

    return Success;
}



ReturnCode
MM_compileGetManagableObjectInfoResult(unsigned char * pCStruct,
				       OS_Boolean * pExists,
				       MM_Protocol * p,
				       OS_Uint8 tag,
				       QU_Head * pQ)
{
    ASN_TableEntry * 	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "GetManagableObjectInfoRequest sequence",
		  ("MM_compileGetManagableObjectInfoRequest: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the object type */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer,
		  ASN_UniversalTag_Integer, pQ,
		  pCStruct,
		  NULL,
		  &p->objectType,
		  "GetManagableObjectInfoResult object type",
		  ("MM_compileGetManagableObjectInfoResult: object type"));
    
    /* Create a table entry for the description */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hString,
		  "GetManagableObjectInfoResult description",
		  ("MM_compileGetManagableObjectInfoResult: description"));
    
    return Success;
}



ReturnCode
MM_compileGetManagableObjectNotifyMaskRequest(unsigned char * pCStruct,
					      OS_Boolean * pExists,
					      MM_Protocol * p,
					      OS_Uint8 tag,
					      QU_Head * pQ)
{
    ASN_TableEntry * 	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "GetManagableObjectNotifyMaskRequest sequence",
		  ("MM_compileGetManagableObjectNotifyMaskRequest: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the application entity instance name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hApplicationEntityInstanceName,
		  "GetManagableObjectNotifyMaskRequest "
		  "application entity instance name",
		  ("MM_compileGetManagableObjectNotifyMaskRequest: "
		   "application entity instance name"));
    
    /* Create a table entry for the module name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hModuleName,
		  "GetManagableObjectNotifyMaskRequest module name",
		  ("MM_compileGetManagableObjectNotifyMaskRequest: "
		   "module name"));
    
    /* Create a table entry for the managable object name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hManagableObjectName,
		  "GetManagableObjectNotifyMaskRequest managable object name",
		  ("MM_compileGetManagableObjectNotifyMaskRequest: "
		   "managable object name"));

    return Success;
}



ReturnCode
MM_compileGetManagableObjectNotifyMaskResult(unsigned char * pCStruct,
					     OS_Boolean * pExists,
					     MM_Protocol * p,
					     OS_Uint8 tag,
					     QU_Head * pQ)
{
    ASN_TableEntry * 	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "GetManagableObjectNotifyMaskResult sequence",
		  ("MM_compileGetManagableObjectNotifyMaskResult: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the notify mask */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer,
		  ASN_UniversalTag_Integer, pQ,
		  pCStruct,
		  NULL,
		  &p->notifyMask,
		  "GetManagableObjectNotifyMaskResult notify mask",
		  ("MM_getManagableObjectNotifyMaskResult: notifyMask"));

    return Success;
}


ReturnCode
MM_compileSetManagableObjectNotifyMaskRequest(unsigned char * pCStruct,
					      OS_Boolean * pExists,
					      MM_Protocol * p,
					      OS_Uint8 tag,
					      QU_Head * pQ)
{
    ASN_TableEntry * 	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "SetManagableObjectNotifyMaskRequest sequence",
		  ("MM_compileSetManagableObjectNotifyMaskRequest: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the application entity instance name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hApplicationEntityInstanceName,
		  "SetManagableObjectNotifyMaskRequest "
		  "application entity instance name",
		  ("MM_compileSetManagableObjectNotifyMaskRequest: "
		   "application entity instance name"));
    
    /* Create a table entry for the module name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hModuleName,
		  "SetManagableObjectNotifyMaskRequest module name",
		  ("MM_compileSetManagableObjectNotifyMaskRequest: "
		   "module name"));
    
    /* Create a table entry for the managable object name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hManagableObjectName,
		  "SetManagableObjectNotifyMaskRequest managable object name",
		  ("MM_compileSetManagableObjectNotifyMaskRequest: "
		   "managable object name"));

    /* Create a table entry for the notify mask */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer,
		  ASN_UniversalTag_Integer, pQ,
		  pCStruct,
		  NULL,
		  &p->notifyMask,
		  "SetManagableObjectNotifyMaskRequest notify mask",
		  ("MM_compileSetManagableObjectNotifyMaskRequest: "
		   "notifyMask"));

    return Success;
}



ReturnCode
MM_compileGetDestinationListRequest(unsigned char * pCStruct,
				    OS_Boolean * pExists,
				    MM_Protocol * p,
				    OS_Uint8 tag,
				    QU_Head * pQ)
{
    ASN_TableEntry * 	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "GetDestinationListRequest sequence",
		  ("MM_compileGetDestinationListRequest: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the application entity instance name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hApplicationEntityInstanceName,
		  "GetDestinationListRequest application entity instance name",
		  ("MM_compileGetDestinationListRequest: "
		   "application entity instance name"));
    
    return Success;
}


ReturnCode
MM_compileGetDestinationListResult(unsigned char * pCStruct,
			      OS_Boolean * pExists,
			      MM_Protocol * p,
			      OS_Uint8 tag,
			      QU_Head * pQ)
{
    ASN_TableEntry * 	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "GetDestinationListResult sequence",
		  ("MM_compileGetDestinationListResult: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the destination list */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hString,
		  "GetDestinationListResult destination list",
		  ("MM_compileGetDestinationListResult: "
		   "application entity destination list"));
    
    return Success;
}


ReturnCode
MM_compileGetDestinationMasksRequest(unsigned char * pCStruct,
				     OS_Boolean * pExists,
				     MM_Protocol * p,
				     OS_Uint8 tag,
				     QU_Head * pQ)
{
    ASN_TableEntry * 	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "GetDestinationMasksRequest sequence",
		  ("MM_compileGetDestinationMasksRequest: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the application entity instance name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hApplicationEntityInstanceName,
		  "GetDestinationMasksRequest "
		  "application entity instance name",
		  ("MM_compileGetDestinationMasksRequest: "
		   "application entity instance name"));
    
    /* Create a table entry for the destination name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hDestinationName,
		  "GetDestinationMasksRequest destination name",
		  ("MM_compileGetDestinationMasksRequest: "
		   "destination name"));
    
    return Success;
}


ReturnCode
MM_compileGetDestinationMasksResult(unsigned char * pCStruct,
				    OS_Boolean * pExists,
				    MM_Protocol * p,
				    OS_Uint8 tag,
				    QU_Head * pQ)
{
    ASN_TableEntry * 	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "GetDestinationMasksResult sequence",
		  ("MM_compileGetDestinationMasksResult: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the notify mask */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer,
		  ASN_UniversalTag_Integer, pQ,
		  pCStruct,
		  NULL,
		  &p->notifyMask,
		  "GetDestinationMasksResult notify mask",
		  ("MM_compileGetDestinationMasksResult: notifyMask"));

    /* Create a table entry for the event mask */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer,
		  ASN_UniversalTag_Integer, pQ,
		  pCStruct,
		  NULL,
		  &p->eventMask,
		  "GetDestinationMasksResult event mask",
		  ("MM_compileGetDestinationMasksResult: eventMask"));

    return Success;
}


ReturnCode
MM_compileSetDestinationMasksRequest(unsigned char * pCStruct,
				     OS_Boolean * pExists,
				     MM_Protocol * p,
				     OS_Uint8 tag,
				     QU_Head * pQ)
{
    ASN_TableEntry * 	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "SetDestinationMasksRequest sequence",
		  ("MM_compileSetDestinationMasksRequest: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the application entity instance name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hApplicationEntityInstanceName,
		  "SetDestinationMasksRequest "
		  "application entity instance name",
		  ("MM_compileSetDestinationMasksRequest: "
		   "application entity instance name"));
    
    /* Create a table entry for the destination name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hDestinationName,
		  "SetDestinationMasksRequest destination name",
		  ("MM_compileSetDestinationMasksRequest: "
		   "destination name"));
    
    /* Create a table entry for the notify mask */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer,
		  ASN_UniversalTag_Integer, pQ,
		  pCStruct,
		  NULL,
		  &p->notifyMask,
		  "SetDestinationMasksRequest notify mask",
		  ("MM_compileSetDestinationMasksRequest: notifyMask"));

    /* Create a table entry for the event mask */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer,
		  ASN_UniversalTag_Integer, pQ,
		  pCStruct,
		  NULL,
		  &p->eventMask,
		  "SetDestinationMasksRequest event mask",
		  ("MM_compileSetDestinationMasksRequest: eventMask"));

    return Success;
}


ReturnCode
MM_compileThresholdEvent(unsigned char * pCStruct,
			 OS_Boolean * pExists,
			 MM_Protocol * p,
			 OS_Uint8 tag,
			 QU_Head * pQ)
{
    ReturnCode		rc;
    ASN_TableEntry * 	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "ThresholdEvent sequence",
		  ("MM_compileThresholdEvent: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the application entity instance name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hApplicationEntityInstanceName,
		  "ThresholdEvent "
		  "application entity instance name",
		  ("MM_compileThresholdEvent: "
		   "application entity instance name"));
    
    /* Create a table entry for the module name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hModuleName,
		  "ThresholdEvent module name",
		  ("MM_compileThresholdEvent: "
		   "module name"));
    
    /* Create a table entry for the managable object name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hManagableObjectName,
		  "ThresholdEvent managable object name",
		  ("MM_compileThresholdEvent: "
		   "managable object name"));

    /* Add the exceeded or reentered threshold value */
    RC_CALL(rc,
	    compileValue((unsigned char *) pCStruct,
			 NULL,
			 &p->value,
			 0, pQ),
	    ("MM_compileThresholdEvent: value"));
    
    /* Add the new value */
    RC_CALL(rc,
	    compileValue((unsigned char *) pCStruct,
			 NULL,
			 &p->value2,
			 0, pQ),
	    ("MM_compileThresholdEvent: value"));

    return Success;
}


ReturnCode
MM_compileLogMessageEvent(unsigned char * pCStruct,
			  OS_Boolean * pExists,
			  MM_Protocol * p,
			  OS_Uint8 tag,
			  QU_Head * pQ)
{
    ASN_TableEntry * 	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "LogMessageEvent sequence",
		  ("MM_compileLogMessageEvent: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the application entity instance name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hApplicationEntityInstanceName,
		  "LogMessageEvent "
		  "application entity instance name",
		  ("MM_compileLogMessageEvent: "
		   "application entity instance name"));
    
    /* Create a table entry for the module name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hModuleName,
		  "LogMessageEvent module name",
		  ("MM_compileLogMessageEvent: "
		   "module name"));
    
    /* Create a table entry for the managable object name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hManagableObjectName,
		  "LogMessageEvent managable object name",
		  ("MM_compileLogMessageEvent: "
		   "managable object name"));

    /* Create a table entry for the log message string */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hString,
		  "LogMessageEvent log message string",
		  ("MM_compileLogMessageEvent: "
		   "log message string"));

    return Success;
}


ReturnCode
MM_compileValueChangedEvent(unsigned char * pCStruct,
			    OS_Boolean * pExists,
			    MM_Protocol * p,
			    OS_Uint8 tag,
			    QU_Head * pQ)
{
    ReturnCode		rc;
    ASN_TableEntry * 	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "ValueChangedEvent sequence",
		  ("MM_compileValueChangedEvent: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the application entity instance name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hApplicationEntityInstanceName,
		  "ValueChangedEvent "
		  "application entity instance name",
		  ("MM_compileValueChangedEvent: "
		   "application entity instance name"));
    
    /* Create a table entry for the module name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hModuleName,
		  "ValueChangedEvent module name",
		  ("MM_compileValueChangedEvent: "
		   "module name"));
    
    /* Create a table entry for the managable object name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hManagableObjectName,
		  "ValueChangedEvent managable object name",
		  ("MM_compileValueChangedEvent: "
		   "managable object name"));

    /* Add the new value */
    RC_CALL(rc,
	    compileValue((unsigned char *) pCStruct,
			 NULL,
			 &p->value,
			 0, pQ),
	    ("MM_compileValueChangedEvent: value"));

    return Success;
}


ReturnCode
MM_compileTimerExpiredEvent(unsigned char * pCStruct,
			    OS_Boolean * pExists,
			    MM_Protocol * p,
			    OS_Uint8 tag,
			    QU_Head * pQ)
{
    ASN_TableEntry * 	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "TimerExpiredEvent sequence",
		  ("MM_compileTimerExpiredEvent: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the application entity instance name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hApplicationEntityInstanceName,
		  "TimerExpiredEvent "
		  "application entity instance name",
		  ("MM_compileTimerExpiredEvent: "
		   "application entity instance name"));
    
    /* Create a table entry for the module name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hModuleName,
		  "TimerExpiredEvent module name",
		  ("MM_compileTimerExpiredEvent: "
		   "module name"));
    
    /* Create a table entry for the managable object name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hManagableObjectName,
		  "TimerExpiredEvent managable object name",
		  ("MM_compileTimerExpiredEvent: "
		   "managable object name"));

    return Success;
}


ReturnCode
MM_compileManagableObjectChangedEvent(unsigned char * pCStruct,
				      OS_Boolean * pExists,
				      MM_Protocol * p,
				      OS_Uint8 tag,
				      QU_Head * pQ)
{
    ASN_TableEntry * 	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  "ManagableObjectChangedEvent sequence",
		  ("MM_compileManagableObjectChangedEvent: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the application entity instance name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hApplicationEntityInstanceName,
		  "ManagableObjectChangedEvent "
		  "application entity instance name",
		  ("MM_compileManagableObjectChangedEvent: "
		   "application entity instance name"));
    
    /* Create a table entry for the module name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hModuleName,
		  "ManagableObjectChangedEvent module name",
		  ("MM_compileManagableObjectChangedEvent: "
		   "module name"));
    
    /* Create a table entry for the managable object name */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_UniversalTag_OctetString, pQ,
		  pCStruct,
		  NULL,
		  &p->hManagableObjectName,
		  "ManagableObjectChangedEvent managable object name",
		  ("MM_compileManagableObjectChangedEvent: "
		   "managable object name"));

    return Success;
}


static ReturnCode
compileValue(unsigned char * pCStruct,
	     OS_Boolean * pExists,
	     MM_ASNValue * pValue,
	     OS_Uint8 tag,
	     QU_Head * pQ)
{
    ASN_TableEntry *	pTab;
    
    ASN_NEW_TABLE(pTab, ASN_Type_Choice, tag, pQ,
		  pCStruct,
		  pExists,
		  &pValue->choice,
		  "MM_ASNValue",
		  ("compileValue: choice"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* create a table entry for the signed integer choice. */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer,
		  ASN_TAG_APPL(0), pQ,
		  pCStruct,
		  NULL,
		  &pValue->un.signedInt,
		  "signed integer",
		  ("compileValue: signed int"));

    /* create a table entry for the signed integer choice. */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer,
		  ASN_TAG_APPL(1), pQ,
		  pCStruct,
		  NULL,
		  &pValue->un.unsignedInt,
		  "unsigned integer",
		  ("compileValue: unsigned int"));

    /* create a table entry for the signed integer choice. */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_TAG_APPL(2), pQ,
		  pCStruct,
		  NULL,
		  &pValue->un.string,
		  "string",
		  ("compileValue: string"));

    return Success;
}


