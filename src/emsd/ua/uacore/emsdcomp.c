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
#include "asn.h"
#include "tm.h"
#include "emsd.h"
#include "mtsua.h"


/*
 * ASN "compiled" trees for the EMSD protocols.
 */
static QU_Head		compIpm =
			    QU_INITIALIZE(compIpm);
static QU_Head		compReportDelivery =
			    QU_INITIALIZE(compReportDelivery);
static QU_Head		compDeliverArg =
			    QU_INITIALIZE(compDeliverArg);
static QU_Head		compSubmitArg =
			    QU_INITIALIZE(compSubmitArg);
static QU_Head		compSubmitResult =
			    QU_INITIALIZE(compSubmitResult);
static QU_Head		compErrorRequest =
			    QU_INITIALIZE(compErrorRequest);
static QU_Head		compDeliveryControlArg =
			    QU_INITIALIZE(compDeliveryControlArg);
static QU_Head		compDeliveryControlResult =
			    QU_INITIALIZE(compDeliveryControlResult);
static QU_Head		compDeliveryVerifyArg =
			    QU_INITIALIZE(compDeliveryVerifyArg);
static QU_Head		compDeliveryVerifyResult =
			    QU_INITIALIZE(compDeliveryVerifyResult);
static QU_Head		compSubmissionVerifyArg =
			    QU_INITIALIZE(compSubmissionVerifyArg);
static QU_Head		compSubmissionVerifyResult =
			    QU_INITIALIZE(compSubmissionVerifyResult);




/*
 * UA_initCompileTrees()
 *
 * ASN-compile each of the EMSD protocols.
 *
 * Parameters:
 *
 *     pErrorBuf --
 *         Pointer to a buffer in which error strings are placed.
 */
ReturnCode
UA_initCompileTrees(char * pErrorBuf)
{
    ReturnCode		    rc;
    
    /*
     * Build the ASN compile trees for each of the EMSD protocols
     */
    
    /* Compile a Version 1.1 EMSD IPM Message */
    if ((rc = EMSD_compileIpm1_1((void *) 0x2,
				NULL,
				(void *) 0x2, 0,
				&compIpm)) != Success)
    {
	sprintf(pErrorBuf,
		"%s: Compiling 1.1 IPM failed, reason 0x%04x\n",
		__applicationName, rc);
	return rc;
    }
    
    /* Compile a Version 1.1 EMSD Delivery Report */
    if ((rc =
	 EMSD_compileReportDelivery1_1((void *) 0x2,
				      NULL,
				      (void *) 0x2, 0,
				      &compReportDelivery)) != Success)
    {
	sprintf(pErrorBuf,
		"%s: Compiling 1.1 delivery report failed, "
		"reason 0x%04x\n",
		__applicationName, rc);
	return rc;
    }
    
    /* Compile a Version 1.1 EMSD Deliver Argument */
    if ((rc =
	 EMSD_compileDeliverArgument1_1((void *) 0x2,
				       NULL,
				       (void *) 0x2, 0,
				       &compDeliverArg)) != Success)
    {
	sprintf(pErrorBuf,
		"%s: Compiling 1.1 deliver arg failed, "
		"reason 0x%04x\n",
		__applicationName, rc);
	return rc;
    }
    
    /* Compile a Version 1.1 EMSD Submit Argument */
    if ((rc =
	 EMSD_compileSubmitArgument1_1((void *) 0x2,
				      NULL,
				      (void *) 0x2, 0,
				      &compSubmitArg)) != Success)
    {
	sprintf(pErrorBuf,
		"%s: "
		"Compiling 1.1 submit arg failed, reason 0x%04x\n",
		__applicationName, rc);
	return rc;
    }
    
    /* Compile a Version 1.1 EMSD Submit Result */
    if ((rc =
	 EMSD_compileSubmitResult1_1((void *) 0x2,
				      NULL,
				      (void *) 0x2, 0,
				      &compSubmitResult)) != Success)
    {
	sprintf(pErrorBuf,
		"%s: "
		"Compiling 1.1 submit result failed, reason 0x%04x\n",
		__applicationName, rc);
	return rc;
    }
    
    /* Compile a Version 1.1 EMSD Error Indication */
    if ((rc =
	 EMSD_compileErrorRequest1_1((void *) 0x2,
				    NULL,
				    (void *) 0x2, 0,
				    &compErrorRequest)) != Success)
    {
	sprintf(pErrorBuf,
		"%s: "
		"Compiling 1.1 error indication failed, "
		"reason 0x%04x\n",
		__applicationName, rc);
	return rc;
    }
    
    /* Compile a Version 1.1 EMSD Delivery Control argument */
    if ((rc =
	 EMSD_compileDeliveryControlArgument1_1((void *) 0x2,
					       NULL,
					       (void *) 0x2, 0,
					       &compDeliveryControlArg)) !=
	Success)
    {
	sprintf(pErrorBuf,
		"%s: "
		"Compiling 1.1 delivery control argument failed, "
		"reason 0x%04x\n",
		__applicationName, rc);
	return rc;
    }
    
    /* Compile a Version 1.1 EMSD Delivery Control result */
    if ((rc =
	 EMSD_compileDeliveryControlResult1_1((void *) 0x2,
					       NULL,
					       (void *) 0x2, 0,
					       &compDeliveryControlResult)) !=
	Success)
    {
	sprintf(pErrorBuf,
		"%s: "
		"Compiling 1.1 delivery control result failed, "
		"reason 0x%04x\n",
		__applicationName, rc);
	return rc;
    }
    
    /* Compile a Version 1.1 EMSD Delivery Verify argument */
    if ((rc =
	 EMSD_compileDeliveryVerifyArgument1_1((void *) 0x2,
					      NULL,
					      (void *) 0x2, 0,
					      &compDeliveryVerifyArg)) !=
	Success)
    {
	sprintf(pErrorBuf,
		"%s: "
		"Compiling 1.1 delivery verify argument failed, "
		"reason 0x%04x\n",
		__applicationName, rc);
	return rc;
    }
	    
    /* Compile a Version 1.1 EMSD Delivery Verify result */
    if ((rc =
	 EMSD_compileDeliveryVerifyResult1_1((void *) 0x2,
					      NULL,
					      (void *) 0x2, 0,
					      &compDeliveryVerifyResult)) !=
	Success)
    {
	sprintf(pErrorBuf,
		"%s: "
		"Compiling 1.1 delivery verify result failed, "
		"reason 0x%04x\n",
		__applicationName, rc);
	return rc;
    }
	    
    return Success;
}
	    
	    
	    
/*
 * UA_getProtocolree()
 *
 * Get a pointer to the ASN-compiled tree for the specified protocol.
 *
 * Parameters:
 *
 *     protocol --
 *         Protocol identifier for which a pointer to the ASN-compiled tree is
 *         desired.
 *
 * Returns:
 *     Queue head for desired ASN-compiled tree.
 */
QU_Head *
UA_getProtocolTree(MTSUA_ProtocolType protocol)
{
    /*
     * See which protocol they're looking for.  Give 'em the compiled
     * tree pointer for that protocol.
     */
    switch(protocol)
    {
    case MTSUA_Protocol_Ipm:
	return &compIpm;
			    
    case MTSUA_Protocol_ReportDelivery:
	return &compReportDelivery;
			    
    case MTSUA_Protocol_DeliverArg:
	return &compDeliverArg;
			    
    case MTSUA_Protocol_SubmitArg:
	return &compSubmitArg;
			    
    case MTSUA_Protocol_SubmitResult:
	return &compSubmitResult;
			    
    case MTSUA_Protocol_ErrorRequest:
	return &compErrorRequest;

    case MTSUA_Protocol_DeliveryControlArg:
	return &compDeliveryControlArg;

    case MTSUA_Protocol_DeliveryControlRes:
	return &compDeliveryControlResult;

    case MTSUA_Protocol_DeliveryVerifyArg:
	return &compDeliveryVerifyArg;

    case MTSUA_Protocol_DeliveryVerifyRes:
	return &compDeliveryVerifyResult;

    case MTSUA_Protocol_SubmissionVerifyArg:
	return &compSubmissionVerifyArg;

    case MTSUA_Protocol_SubmissionVerifyRes:
	return &compSubmissionVerifyResult;
    }
		
    /* Unsupported protocol version. */
    return NULL;
}
