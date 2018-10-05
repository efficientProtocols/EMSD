/*
 * 
 * Copyright (C) 1997-2002  Neda Communications, Inc.
 * 
 * This file is part of Neda-EMSD. An implementation of RFC-2524.
 * 
 * Neda-EMSD is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (GPL) as 
 * published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 * 
 * Neda-EMSD is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Neda-EMSD; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.  
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

/*
 * 
 * History:
 *
 */


#include "estd.h"
#include "tm.h"
#include "emsd.h"
#include "emsdlocal.h"

static ReturnCode
compileRecipientList(unsigned char * pCStruct,
		     OS_Boolean * pExists,
		     EMSD_RecipientList * pRecipList,
		     OS_Uint8 tag,
		     QU_Head * pQ,
		     char * pMessage);

/* --------------------------------------------------------------- */

ReturnCode
EMSD_compileProbe1_1(unsigned char * pCStruct,
		    OS_Boolean * pExists,
		    EMSD_ProbeSubmission * pProbe,
		    OS_Uint8 tag,
		    QU_Head * pQ)
{
    ReturnCode		rc;
    ASN_TableEntry * 	pTab;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  pCStruct,
		  NULL, NULL,
		  "Probe",
		  ("emsd_compileProbe: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Add the sender */
    RC_CALL(rc,
	    emsd_compileORAddress1_1((unsigned char *) pCStruct,
				    NULL,
				    &pProbe->sender,
				    0, pQ,
				    "Sender ORAddress"),
	    ("emsd_compileProbe: sender"));
    
    /* Add the recipient list */
    RC_CALL(rc,
	    compileRecipientList((unsigned char *) pCStruct,
				 NULL,
				 &pProbe->recipients,
				 ASN_UniversalTag_Sequence, pQ,
				 "Recipient List"),
	    ("emsd_compileProbe: recipients"));

    /* Create a table entry for the content type */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer,
		  ASN_UniversalTag_Integer, pQ,
		  pCStruct,
		  &pProbe->contentType.exists,
		  &pProbe->contentType.data,
		  "Content Type",
		  ("emsd_compileReportDelivery: content type"));

    /* Create a table entry for the content length */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer,
		  ASN_TAG_CONTEXT(0), pQ,
		  pCStruct,
		  &pProbe->contentLength.exists,
		  &pProbe->contentLength.data,
		  "Content Length",
		  ("emsd_compileReportDelivery: content length"));

    return Success;
}


static ReturnCode
compileRecipientList(unsigned char * pCStruct,
		     OS_Boolean * pExists,
		     EMSD_RecipientList * pRecipList,
		     OS_Uint8 tag,
		     QU_Head * pQ,
		     char * pMessage)
{
    ReturnCode		rc;
    ASN_TableEntry * 	pTab;
    
    /* Create a table entry for a sequence of */
    ASN_NEW_TABLE(pTab, ASN_Type_SequenceOf,
		  tag, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  pMessage,
		  ("compileRecipientList: sequence of"));

    /* Specify the maximum number of elements in the sequence-of */
    pTab->maxDataLength = EMSDUB_MAX_RECIPIENTS;
    
    /* Keep track of the size of each element */
    pTab->elementSize = sizeof(EMSD_RecipientList);
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Add the recipient address */
    RC_CALL(rc,
	    emsd_compileORAddress1_1((unsigned char *) pCStruct,
				    NULL,
				    pRecipList->data,
				    0, pQ,
				    "Recipient Address"),
	    ("compileRecipientData: recipient address"));

    return Success;
}
