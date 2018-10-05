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
#include "emsd.h"

char *
ua_messageIdEmsdLocalToRfc822(EMSD_LocalMessageId * pEmsdLocalMessageId,
			     char * pBuf)
{
    sprintf(pBuf, "%08lx.%08lx",
	    pEmsdLocalMessageId->submissionTime,
	    pEmsdLocalMessageId->dailyMessageNumber);

#if 0
#ifdef notyet
    sprintf(pBuf + strlen(pBuf), "@mts%04lx.emsd.org",
	    pEmsdLocalMessageId->mtsId);
#else
    strcpy(pBuf + strlen(pBuf), "@mts0023.emsd.org");
#endif
#endif

    return pBuf;
}


char *
ua_messageIdEmsdToRfc822(EMSD_MessageId * pEmsdMessageId,
			char * pBuf)
{
    /* Which format of EMSD Message Id is it? */
    switch(pEmsdMessageId->choice)
    {
    case EMSD_MESSAGEID_CHOICE_LOCAL:
	return ua_messageIdEmsdLocalToRfc822(&pEmsdMessageId->un.local, pBuf);
	
    case EMSD_MESSAGEID_CHOICE_RFC822:
	strcpy(pBuf, STR_stringStart(pEmsdMessageId->un.rfc822));
	return pBuf;

    default:
	return NULL;
    }
}


ReturnCode
ua_messageIdRfc822ToEmsdLocal(char * pRfc822MessageId,
			     EMSD_MessageId * pEmsdMessageId)
{
    ReturnCode	    rc;
    unsigned long   a;
    unsigned long   b;
    unsigned long   c;

    /*
     * If it's not in the format that we created in
     * ua_messageIdEmsdLocalToRfc822(), then we'll just give 'em back
     * the RFC822 message id.  Otherwise, we'll parse the local-format
     * message id and give it to 'em in local format.
     */
    if (sscanf(pRfc822MessageId,
	       "%lx.%lx@mts%lx.emsd.org",
	       &a, &b, &c) == 3)
    {
	/* It matched our local format. */
	pEmsdMessageId->choice = EMSD_MESSAGEID_CHOICE_LOCAL;
	pEmsdMessageId->un.local.submissionTime = a;
	pEmsdMessageId->un.local.dailyMessageNumber = b;
#ifdef notyet
	pEmsdMessageId->un.local.mtsId = c;
#endif
    }
    else
    {
	/*
	 * Just give 'em the RFC-822 format
	 */

	/* Make sure that an STR string gets allocated */
	pEmsdMessageId->un.rfc822 = NULL;

	/* Allocate an STR string and assign it the RFC-822 message id */
	if ((rc = STR_assignZString(&pEmsdMessageId->un.rfc822,
				    pRfc822MessageId)) != Success)
	{
	    return rc;
	}
    }

    return Success;
}
