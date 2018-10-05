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
#include "eh.h"
#include "mtsua.h"
#include "mts.h"
#include "tm.h"
#include "msgstore.h"

void
mts_deliveryNonDel(struct DQ_ControlData * pMsg, OS_Uint32 error)
{
    void *	                hSendmail;
    
    if (pMsg == NULL) {
      TM_TRACE((globals.hTM, MTS_TRACE_ERROR, "Non-Delivery called with NULL argument"));
      return;
    }
    /* Update the message status */
    verify_setStatus (pMsg->messageId,
		      EMSD_DeliveryVerifyStatus_NonDeliveryReportIsSentOut);

    /* Create a command line to sendmail */
    sprintf(globals.tmpbuf, "%s '%s'", globals.config.pSendmailPath, pMsg->originator);
    
    TM_TRACE((globals.hTM, MTS_TRACE_STATE, "Non-Delivery '%s'", globals.tmpbuf));

    /* Open a pipe to sendmail */
    if ((hSendmail = popen(globals.tmpbuf, "w")) == NULL)
    {
	EH_fatal("Could not open pipe to sendmail for non-delivery.\n");
    }
    
#ifdef RFC1894
    /*
     * Generate a non-delivery message in the format specified in
     * RFC-1894, Delivery Status Notifications.
     */
#else
    
    /* Create the header of message */
    fprintf(hSendmail, "From: Postmaster\n");
    fprintf(hSendmail, "To: %s\n", pMsg->originator);
    fprintf(hSendmail, "Subject: Non-Delivery report\n");
    
    /* Separate the header from the body */
    fprintf(hSendmail, "\n");
    
    /* Create the body of the message */
    fprintf(hSendmail, "Your message %s\nCould not be delivered.\n",
	    pMsg->messageId);
    
    fclose(hSendmail);
#endif
    
}


void
mts_submissionNonDel(MTS_Operation * pOperation,
		     OS_Uint32 error)
{
    printf("mts_submissionNonDel() not yet implemented\n");

    /* delete the control and data files */
}


void
mts_rfc822SubmitNonDel(MTSUA_ControlFileData * pMsg,
		       ReturnCode rc)
{
    printf("mts_rfc822SubmitNonDel() not yet implemented\n");
}
