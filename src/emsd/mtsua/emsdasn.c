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
#include "asn.h"


/*
 * ASN "compiled" trees for the EMSD protocols.  We're now supporting
 * protocol version 1.0 and 1.1, so we have separate trees for each.
 */
static QU_Head	compIpm1_0			= QU_INITIALIZE(compIpm1_0);
static QU_Head	compIpm1_1			= QU_INITIALIZE(compIpm1_1);
static QU_Head	compReportDelivery1_0		= QU_INITIALIZE(compReportDelivery1_0);
static QU_Head	compReportDelivery1_1		= QU_INITIALIZE(compReportDelivery1_1);
static QU_Head	compDeliverArg1_0		= QU_INITIALIZE(compDeliverArg1_0);
static QU_Head	compDeliverArg1_1		= QU_INITIALIZE(compDeliverArg1_1);
static QU_Head	compSubmitArg1_0		= QU_INITIALIZE(compSubmitArg1_0);
static QU_Head	compSubmitArg1_1		= QU_INITIALIZE(compSubmitArg1_1);
static QU_Head	compSubmitResult1_0		= QU_INITIALIZE(compSubmitResult1_0);
static QU_Head	compSubmitResult1_1		= QU_INITIALIZE(compSubmitResult1_1);
static QU_Head	compErrorRequest1_0		= QU_INITIALIZE(compErrorRequest1_0);
static QU_Head	compErrorRequest1_1		= QU_INITIALIZE(compErrorRequest1_1);

static QU_Head	compDeliveryControlArg1_1	= QU_INITIALIZE(compDeliveryControlArg1_1);
static QU_Head	compDeliveryControlResult1_1	= QU_INITIALIZE(compDeliveryControlResult1_1);
static QU_Head	compDeliveryVerifyArg1_1	= QU_INITIALIZE(compDeliveryVerifyArg1_1);
static QU_Head	compDeliveryVerifyResult1_1	= QU_INITIALIZE(compDeliveryVerifyResult1_1);
static QU_Head	compSubmissionVerifyArg1_1	= QU_INITIALIZE(compSubmissionVerifyArg1_1);
static QU_Head	compSubmissionVerifyResult1_1	= QU_INITIALIZE(compSubmissionVerifyResult1_1);


/* Supported protocol versions, and compile trees for them */
static struct
{
    /* Major and Minor protocol version numbers */
    OS_Uint8		    major;
    OS_Uint8		    minor;
    
    /* Pointers to the compiled ASN trees for this protocol version */
    QU_Head *		    pCompDeliverArg;
    QU_Head *		    pCompSubmitArg;
    QU_Head *		    pCompSubmitResult;
    QU_Head *		    pCompDeliveryControlArg;
    QU_Head *		    pCompDeliveryControlResult;
    QU_Head *		    pCompDeliveryVerifyArg;
    QU_Head *		    pCompDeliveryVerifyResult;
    QU_Head *		    pCompSubmissionVerifyArg;
    QU_Head *		    pCompSubmissionVerifyResult;
    
    QU_Head *		    pCompErrorRequest;
    
    QU_Head *		    pCompIpm;
    QU_Head *		    pCompReportDelivery;
} protocolVersions[] =
  {
      {						 /* Version 1.1 */
	  1, 1,
	  
	  &compDeliverArg1_1,
	  &compSubmitArg1_1,
	  &compSubmitResult1_1,
	  &compDeliveryControlArg1_1,
	  &compDeliveryControlResult1_1,
	  &compDeliveryVerifyArg1_1,
	  &compDeliveryVerifyResult1_1,
	  &compSubmissionVerifyArg1_1,
	  &compSubmissionVerifyResult1_1,
	  
	  &compErrorRequest1_1,
	  
	  &compIpm1_1,
	  &compReportDelivery1_1
      },
      {						 /* Version 1.0 */
	  1, 0,
	  
	  &compDeliverArg1_0,
	  &compSubmitArg1_0,
	  &compSubmitResult1_0,
	  NULL,
	  NULL,
	  NULL,
	  NULL,
	  NULL,
	  NULL,
	  
	  &compErrorRequest1_0,
	  
	  &compIpm1_0,
	  &compReportDelivery1_0
      }
  };




ReturnCode
mtsua_initCompileTrees(char * pErrorBuf)
{
    ReturnCode		    rc;
    
    /*
     * Build the ASN compile trees for each of the EMSD protocols
     */
    
    /* Compile a Version 1.0 EMSD IPM Message */
    if ((rc = EMSD_compileIpm1_0((void *) 0x2,
				NULL,
				(void *) 0x2, 0,
				&compIpm1_0)) != Success)
    {
	sprintf(pErrorBuf,
		"%s: Compiling 1.0 IPM failed, reason 0x%04x\n",
		__applicationName, rc);
	return rc;
    }
    
    /* Compile a Version 1.1 EMSD IPM Message */
    if ((rc = EMSD_compileIpm1_1((void *) 0x2,
				NULL,
				(void *) 0x2, 0,
				&compIpm1_1)) != Success)
    {
	sprintf(pErrorBuf,
		"%s: Compiling 1.1 IPM failed, reason 0x%04x\n",
		__applicationName, rc);
	return rc;
    }
    
    /* Compile a Version 1.0 EMSD Delivery Report */
    if ((rc = EMSD_compileReportDelivery1_0((void *) 0x2,
					   NULL,
					   (void *) 0x2, 0,
					   &compReportDelivery1_0)) != Success)
    {
	sprintf(pErrorBuf,
		"%s: Compiling 1.0 delivery report failed, "
		"reason 0x%04x\n",
		__applicationName, rc);
	return rc;
    }
    
    /* Compile a Version 1.1 EMSD Delivery Report */
    if ((rc =
	 EMSD_compileReportDelivery1_1((void *) 0x2,
				      NULL,
				      (void *) 0x2, 0,
				      &compReportDelivery1_1)) != Success)
    {
	sprintf(pErrorBuf,
		"%s: Compiling 1.1 delivery report failed, "
		"reason 0x%04x\n",
		__applicationName, rc);
	return rc;
    }
    
    /* Compile a Version 1.0 EMSD Deliver Argument */
    if ((rc = EMSD_compileDeliverArgument1_0((void *) 0x2,
					    NULL,
					    (void *) 0x2, 0,
					    &compDeliverArg1_0)) != Success)
    {
	sprintf(pErrorBuf,
		"%s: Compiling 1.0 deliver arg failed, "
		"reason 0x%04x\n",
		__applicationName, rc);
	return rc;
    }
    
    /* Compile a Version 1.1 EMSD Deliver Argument */
    if ((rc = EMSD_compileDeliverArgument1_1((void *) 0x2,
					    NULL,
					    (void *) 0x2, 0,
					    &compDeliverArg1_1)) != Success)
    {
	sprintf(pErrorBuf,
		"%s: Compiling 1.1 deliver arg failed, "
		"reason 0x%04x\n",
		__applicationName, rc);
	return rc;
    }
    
    /* Compile a Version 1.0 EMSD Submit Argument */
    if ((rc = EMSD_compileSubmitArgument1_0((void *) 0x2,
					   NULL,
					   (void *) 0x2, 0,
					   &compSubmitArg1_0)) != Success)
    {
	sprintf(pErrorBuf,
		"%s: "
		"Compiling 1.0 submit arg failed, reason 0x%04x\n",
		__applicationName, rc);
	return rc;
    }
    
    /* Compile a Version 1.1 EMSD Submit Argument */
    if ((rc = EMSD_compileSubmitArgument1_1((void *) 0x2,
					   NULL,
					   (void *) 0x2, 0,
					   &compSubmitArg1_1)) != Success)
    {
	sprintf(pErrorBuf,
		"%s: "
		"Compiling 1.1 submit arg failed, reason 0x%04x\n",
		__applicationName, rc);
	return rc;
    }
    /* Compile a Version 1.0 EMSD Submit Result */
    if ((rc = EMSD_compileSubmitResult1_0((void *) 0x2,
					 NULL,
					 (void *) 0x2, 0,
					 &compSubmitResult1_0)) != Success)
    {
	sprintf(pErrorBuf,
		"%s: "
		"Compiling 1.0 submit result failed, reason 0x%04x\n",
		__applicationName, rc);
	return rc;
    }
    
    /* Compile a Version 1.1 EMSD Submit Result */
    if ((rc =
	 EMSD_compileSubmitResult1_1((void *) 0x2,
				      NULL,
				      (void *) 0x2, 0,
				      &compSubmitResult1_1)) != Success)
    {
	sprintf(pErrorBuf,
		"%s: "
		"Compiling 1.1 submit result failed, reason 0x%04x\n",
		__applicationName, rc);
	return rc;
    }
    
    /* Compile a Version 1.1 EMSD Submission Verify argument */
    rc = EMSD_compileSubmissionVerifyArgument1_1((void *)0x2, NULL, (void *)0x2,
						0, &compSubmissionVerifyArg1_1);
    if (rc != Success)
    {
	sprintf(pErrorBuf,
		"%s: Compiling 1.1 submission verify argument failed, "
		"reason 0x%04x\n",
		__applicationName, rc);
	return rc;
    }
	    
    /* Compile a Version 1.1 EMSD Submission Verify result */
    rc = EMSD_compileSubmissionVerifyResult1_1((void *) 0x2, NULL, (void *) 0x2,
					      0, &compSubmissionVerifyResult1_1);
    if (rc != Success)
    {
	sprintf(pErrorBuf,
		"%s: Compiling 1.1 submission verify result failed, "
		"reason 0x%04x\n",
		__applicationName, rc);
	return rc;
    }
	    
    /* Compile a Version 1.0 EMSD Error Indication */
    if ((rc =
	 EMSD_compileErrorRequest1_0((void *) 0x2,
				    NULL,
				    (void *) 0x2, 0,
				    &compErrorRequest1_0)) != Success)
    {
	sprintf(pErrorBuf,
		"%s: "
		"Compiling 1.0 error indication failed, "
		"reason 0x%04x\n",
		__applicationName, rc);
	return rc;
    }
    
    /* Compile a Version 1.1 EMSD Error Indication */
    rc = EMSD_compileErrorRequest1_1((void *) 0x2, NULL, (void *) 0x2, 0, &compErrorRequest1_1);
    if (rc != Success)
    {
	sprintf(pErrorBuf, "Compiling 1.1 error indication failed, reason 0x%04x\n", rc);
	return rc;
    }
    
    
    /* Compile a Version 1.1 EMSD Delivery Control argument */
    rc = EMSD_compileDeliveryControlArgument1_1((void *) 0x2, NULL, (void *) 0x2, 0, &compDeliveryControlArg1_1);
    if (rc != Success)
    {
	sprintf(pErrorBuf,
		"%s: "
		"Compiling 1.1 delivery control argument failed, "
		"reason 0x%04x\n",
		__applicationName, rc);
	return rc;
    }
    
    /* Compile a Version 1.1 EMSD Delivery Control result */
    rc = EMSD_compileDeliveryControlResult1_1((void *) 0x2, NULL, (void *) 0x2, 0, &compDeliveryControlResult1_1);
    if (rc != Success)
    {
	sprintf(pErrorBuf, "Compiling 1.1 delivery control result failed, reason 0x%04x\n", rc);
	return rc;
    }
    {
      ASN_TableEntry *	pTab = (ASN_TableEntry *)&compDeliveryControlResult1_1;

#if 0
      printf ("***\n%s, %d: compDeliveryControlResult1_1 = 0x%08lx\n", 
	      __FILE__, __LINE__, (long unsigned int)pTab);
      printf ("***\n%s, %d: DebugMessage = %s\n", __FILE__, __LINE__, 
	     pTab->pDebugMessage);
#endif  /* 0 */

    }

    /* Compile a Version 1.1 EMSD Delivery Verify argument */
    rc = EMSD_compileDeliveryVerifyArgument1_1((void *)0x2, NULL,
					      (void *)0x2, 0, &compDeliveryVerifyArg1_1);
    if (rc != Success)
    {
	sprintf(pErrorBuf,
		"%s: Compiling 1.1 delivery verify argument failed, "
		"reason 0x%04x\n",
		__applicationName, rc);
	return rc;
    }
	    
    /* Compile a Version 1.1 EMSD Delivery Verify result */
    rc = EMSD_compileDeliveryVerifyResult1_1((void *) 0x2, NULL, (void *) 0x2,
					    0, &compDeliveryVerifyResult1_1);
    if (rc != Success)
    {
	sprintf(pErrorBuf,
		"%s: Compiling 1.1 delivery verify result failed, "
		"reason 0x%04x\n",
		__applicationName, rc);
	return rc;
    }
	    
    return Success;
}
	    
	    
	    
QU_Head *
mtsua_getProtocolTree(MTSUA_ProtocolType protocol,
		      OS_Uint8 protocolVersionMajor,
		      OS_Uint8 protocolVersionMinor)
{
    int		    i;
		
    /* For each supported protocol version... */
    for (i = 0; i < DIMOF(protocolVersions); i++)
    {
	/* ... Is this the one requested? */
	if (protocolVersionMajor == protocolVersions[i].major &&
	    protocolVersionMinor == protocolVersions[i].minor)
	{
	    /*
	     * Yup.  See which protocol they're looking for.  Give 'em
	     * the compiled tree pointer for that protocol.
	     */
	    switch(protocol)
	    {
	    case MTSUA_Protocol_Ipm:
		return protocolVersions[i].pCompIpm;
			    
	    case MTSUA_Protocol_ReportDelivery:
		return protocolVersions[i].pCompReportDelivery;
			    
	    case MTSUA_Protocol_DeliverArg:
		return protocolVersions[i].pCompDeliverArg;
			    
	    case MTSUA_Protocol_SubmitArg:
		return protocolVersions[i].pCompSubmitArg;
			    
	    case MTSUA_Protocol_SubmitResult:
		return protocolVersions[i].pCompSubmitResult;
			    
	    case MTSUA_Protocol_ErrorRequest:
		return protocolVersions[i].pCompErrorRequest;

	    case MTSUA_Protocol_DeliveryControlArg:
		return protocolVersions[i].pCompDeliveryControlArg;

	    case MTSUA_Protocol_DeliveryControlRes:
		return protocolVersions[i].pCompDeliveryControlResult;

	    case MTSUA_Protocol_DeliveryVerifyArg:
		return protocolVersions[i].pCompDeliveryVerifyArg;

	    case MTSUA_Protocol_DeliveryVerifyRes:
		return protocolVersions[i].pCompDeliveryVerifyResult;

	    case MTSUA_Protocol_SubmissionVerifyArg:
		return protocolVersions[i].pCompSubmissionVerifyArg;

	    case MTSUA_Protocol_SubmissionVerifyRes:
		return protocolVersions[i].pCompSubmissionVerifyResult;

	    default:
	      printf ("%s, %d: Unsupported protocol selection %d: v%d.%d\n", __FILE__, __LINE__,
		      protocol, protocolVersionMajor, protocolVersionMinor);
	      
	    }
	}
    }
		
    /* Unsupported protocol version. */
    printf ("%s, %d: Unsupported protocol version: v%d.%d\n", __FILE__, __LINE__,
	    protocolVersionMajor, protocolVersionMinor);
    return NULL;
}
