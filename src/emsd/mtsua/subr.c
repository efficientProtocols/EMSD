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
#include "emsdmail.h"
#include "emsd822.h"


void
mtsua_osFree(void * p)
{
    /* OS_free() may be a macro.  This function provides a function address */
    OS_free(p);
}


void
mtsua_getPrintableRfc822Address(char * pBuf,
				ADDRESS * pAddr)
{
    const char *rspecials =  "()<>@,;:\\\"[].";
				/* full RFC-822 specials */

    /* Start off with an empty address */
    *pBuf = '\0';

    if (pAddr->mailbox != NULL && pAddr->host == NULL)
    {
	/* yes, write group name */
	rfc822_cat(pBuf, pAddr->mailbox, rspecials);
	strcat(pBuf, ": ");	/* write group identifier */
    }
    else
    {
	if (pAddr->host == NULL)
	{
	    /* end of group */
	    strcat(pBuf,";");
	}
	else if (pAddr->personal == NULL && pAddr->adl == NULL)
	{
	    /* simple case */
	    rfc822_address(pBuf, pAddr);
	}
	else
	{
	    /* must use phrase <route-addr> form */
	    if (pAddr->personal != NULL)
	    {
		rfc822_cat(pBuf, pAddr->personal, rspecials);
	    }

	    strcat(pBuf, " <");	/* write address delimiter */
	    rfc822_address(pBuf, pAddr); /* write address */
	    strcat(pBuf, ">");	/* closing delimiter */
	}
    }
}


