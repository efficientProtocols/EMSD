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
#include "asn.h"
#include "emsd.h"
#include "emsdlocal.h"

static ReturnCode
compileNonDeliveryReport(unsigned char * pCStruct,
			 OS_Boolean * pExists,
			 EMSD_NonDeliveryReport * pNonDeliv,
			 OS_Uint8 tag,
			 QU_Head * pQ,
			 char * pMessage);

static ReturnCode
compilePerRecipReportData(unsigned char * pCStruct,
			  OS_Boolean * pExists,
			  EMSD_PerRecipReportDataList * pPerRecipData,
			  OS_Uint8 tag,
			  QU_Head * pQ,
			  char * pMessage);

static ReturnCode
compileDeliveryReport(unsigned char * pCStruct,
		      OS_Boolean * pExists,
		      EMSD_DeliveryReport * pDeliv,
		      OS_Uint8 tag,
		      QU_Head * pQ,
		      char * pMessage);


/* --------------------------------------------------------------- */

ReturnCode
EMSD_compileReportDelivery1_0(unsigned char * pCStruct,
			     OS_Boolean * pExists,
			     EMSD_ReportDelivery * pReport,
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
		  "Report Delivery",
		  ("v1.0 emsd_compileReportDelivery: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Add the message id */
    RC_CALL(rc,
	    emsd_compileMessageId1_0((unsigned char *) pCStruct,
				    NULL,
				    &pReport->msgIdOfReportedMsg,
				    0, pQ,
				    "Message Id"),
	    ("v1.0 emsd_compileReportDelivery: Message ID"));
    
    /* Add the sequence of per-recipient data */
    RC_CALL(rc,
	    compilePerRecipReportData((unsigned char *) pCStruct,
				      NULL,
				      &pReport->recipDataList,
				      ASN_UniversalTag_Sequence,
				      pQ,
				      "Recipient Data"),
	    ("v1.0 compileHeading: recipient data"));
    
    /* Create a table entry for the returned-content content type */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer,
		  ASN_UniversalTag_Integer, pQ,
		  pCStruct,
		  &pReport->returnedContent.exists,
		  &pReport->returnedContent.contentType,
		  "Returned-Content Content Type",
		  ("v1.0 emsd_compileReportDelivery: content type"))

    /* Create a table entry for the returned-content itself */
    ASN_NEW_TABLE(pTab, ASN_Type_Buffer,
		  0, pQ,
		  pCStruct,
		  &pReport->returnedContent.returnedContents.exists,
		  &pReport->returnedContent.returnedContents.data,
		  "Returned Content",
		  ("v1.0 emsd_compileReportDelivery: returned content"))

    return Success;
}


static ReturnCode
compilePerRecipReportData(unsigned char * pCStruct,
			  OS_Boolean * pExists,
			  EMSD_PerRecipReportDataList * pPerRecipData,
			  OS_Uint8 tag,
			  QU_Head * pQ,
			  char * pMessage)
{
    ReturnCode 			rc;
    ASN_TableEntry * 		pTab;
    EMSD_DeliveryReport * 	pDeliv;
    EMSD_NonDeliveryReport *	pNonDeliv;
    EMSD_PerRecipReportData *	pPerRecipFields =
	pPerRecipData->data;
    
    /* Create a table entry for a sequence of */
    ASN_NEW_TABLE(pTab, ASN_Type_SequenceOf, tag, pQ,
		  pCStruct,
		  pExists,
		  &pPerRecipData->count,
		  pMessage,
		  ("v1.0 compilePerRecipReportData: sequence of"));

    /* Specify the maximum number of elements in the sequence-of */
    pTab->maxDataLength = EMSDUB_MAX_RECIPIENTS;
    
    /* Keep track of the size of each element */
    pTab->elementSize = sizeof(EMSD_PerRecipReportData);
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence,
		  ASN_UniversalTag_Sequence, pQ,
		  NULL,
		  NULL,
		  NULL,
		  "Per-recip Report Data",
		  ("v1.0 compilePerRecipReportData: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Add the recipient address */
    RC_CALL(rc,
	    emsd_compileORAddress1_0((unsigned char *) pCStruct,
				    NULL,
				    &pPerRecipFields->recipient,
				    0, pQ,
				    "Recipient Address"),
	    ("v1.0 compilePerRecipReportData: recipient address"));
    
    /* Create a table entry for the choice */
    ASN_NEW_TABLE(pTab, ASN_Type_Choice, 0, pQ,
		  pCStruct,
		  NULL,
		  &pPerRecipFields->report.choice,
		  "Per-recip Report Data",
		  ("v1.0 compilePerRecipReportData: choice"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Add a delivery report */
    pDeliv = &pPerRecipFields->report.un.deliveryReport;
    RC_CALL(rc,
	    compileDeliveryReport((unsigned char *) pCStruct,
				  NULL,
				  pDeliv,
				  ASN_TAG_CONTEXT(0), pQ,
				  "Delivery Report"),
	    ("v1.0 compilePerRecipReportData: delivery report"));

    /* Add a non-delivery report */
    pNonDeliv = &pPerRecipFields->report.un.nonDeliveryReport;
    RC_CALL(rc,
	    compileNonDeliveryReport((unsigned char *) pCStruct,
				     NULL,
				     pNonDeliv,
				     ASN_TAG_CONTEXT(1), pQ,
				     "Non-Delivery Report"),
	    ("v1.0 compilePerRecipData: non-delivery report"));

    return Success;
}


static ReturnCode
compileDeliveryReport(unsigned char * pCStruct,
		      OS_Boolean * pExists,
		      EMSD_DeliveryReport * pDeliv,
		      OS_Uint8 tag,
		      QU_Head * pQ,
		      char * pMessage)
{
    ASN_TableEntry * 		pTab;

    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence, tag, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  pMessage,
		  ("v1.0 compileDeliveryReport: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;
    
    /* Create a table entry for message delivery time */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer,
		  ASN_UniversalTag_Integer, pQ,
		  pCStruct,
		  NULL,
		  &pDeliv->messageDeliveryTime,
		  "Message Delivery Time",
		  ("v1.0 compileDeliveryReport: message delivery time"));

    return Success;
}


static ReturnCode
compileNonDeliveryReport(unsigned char * pCStruct,
			 OS_Boolean * pExists,
			 EMSD_NonDeliveryReport * pNonDeliv,
			 OS_Uint8 tag,
			 QU_Head * pQ,
			 char * pMessage)
{
    ASN_TableEntry * 		pTab;

    /* Create a table entry for the sequence */
    ASN_NEW_TABLE(pTab, ASN_Type_Sequence, tag, pQ,
		  pCStruct,
		  pExists,
		  NULL,
		  pMessage,
		  ("v1.0 compileNonDeliveryReport: sequence"));
    
    /* Future items get inserted onto this guy's list */
    pQ = (QU_Head *) &pTab->tableList;

    /* Create a table entry for the non-delivery reason */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer,
		  ASN_UniversalTag_Integer, pQ,
		  pCStruct,
		  NULL,
		  &pNonDeliv->nonDeliveryReason,
		  "Non-Delivery Reason",
		  ("v1.0 compileNonDeliveryReport: reason"));

    /* Create a table entry for the non-delivery diagnostic */
    ASN_NEW_TABLE(pTab, ASN_Type_Integer,
		  ASN_TAG_CONTEXT(0), pQ,
		  pCStruct,
		  &pNonDeliv->nonDeliveryDiagnostic.exists,
		  &pNonDeliv->nonDeliveryDiagnostic.data,
		  "Non-Delivery Diagnostic",
		  ("v1.0 compileNonDeliveryReport: diagnostic"));
    
    /* Create a table entry for the non-delivery explanation */
    ASN_NEW_TABLE(pTab, ASN_Type_OctetString,
		  ASN_TAG_CONTEXT(1), pQ,
		  pCStruct,
		  &pNonDeliv->explanation.exists,
		  &pNonDeliv->explanation.data,
		  "Explanation",
		  ("v1.0 compileNonDeliveryReport: explanation"));

    return Success;
}
