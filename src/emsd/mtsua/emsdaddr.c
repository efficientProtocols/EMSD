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
#include "buf.h"
#include "emsdmail.h"
#include "emsd.h"


static ReturnCode (* pfLookupEmsdAddress)(char * p,
					 OS_Uint16 len,
					 OS_Uint32 * pEmsdAddress,
					 ADDRESS * pAddr) = NULL;

static ReturnCode (* pfLookupRfc822Address)(char * pBuf,
					    OS_Uint32 bufLen) = NULL;


void
mtsua_setLookupEmsdAddress(ReturnCode (* pfLookup)(char * p,
						  OS_Uint16 len,
						  OS_Uint32 * pEmsdAddress,
						  ADDRESS * pAddr))
{
    pfLookupEmsdAddress = pfLookup;
}


void
mtsua_setLookupRfc822Address(ReturnCode (* pfLookup)(char * pBuf,
						     OS_Uint32 bufLen))
{
    pfLookupRfc822Address = pfLookup;
}


void
mtsua_createRfc822AddrFromEmsd(EMSD_ORAddress * pORAddr,
			      char * pLocalDomainAndHost,
			      char * pBuf,
			      OS_Uint32 bufLen)
{
    int		    len;

    if (pORAddr->choice == EMSD_ORADDRESS_CHOICE_LOCAL)
    {
	/*
	 * Create an RFC-822 address from the local-format address.
	 *
	 * The format that we use is as follows:
	 *
	 *     A personal name (if it exists), followed by a valid
	 *     address:
	 *
	 *     The User-name portion is composed of:
	 *         The local address value, split into two values separated by
	 *         dots.  The first value is the local address divided by
	 *         1000; the second is the local address modulo 1000.
	 *
	 *     The Domain-name portion is composed of one component:
	 *         The local host name (sub-domain)
	 */
	sprintf(pBuf, "%.*s <%03ld.%03ld%s%s>",
		(pORAddr->un.local.emsdName.exists ?
		 STR_getStringLength(pORAddr->un.local.emsdName.data) :
		 0),
		(pORAddr->un.local.emsdName.exists ?
		 (char *)
		 STR_stringStart(pORAddr->un.local.emsdName.data) :
		 ""),
		pORAddr->un.local.emsdAddress / 1000,
		pORAddr->un.local.emsdAddress % 1000,
		pLocalDomainAndHost == NULL ? "" : "@",
		pLocalDomainAndHost == NULL ? "" : pLocalDomainAndHost);
	printf ("$$$$ Local: %s\n", pBuf);
    }
    else
    {
	/* RFC-822 format */
	len = STR_getStringLength(pORAddr->un.rfc822);

	if (len > bufLen - 1)
	{
	    len = bufLen - 1;
	}

	strncpy(pBuf,
		(char *) STR_stringStart(pORAddr->un.rfc822),
		len);

	pBuf[len] = '\0';
        printf ("$$$$ RFC-822: %s\n", pBuf);
    }

    /* Now it's time for address rewriting. */
    if (pfLookupRfc822Address != NULL)
    {
	(void) (* pfLookupRfc822Address)(pBuf, bufLen);
        printf ("$$$$ Rewritten: %s\n", pBuf);
    }
}



ReturnCode
mtsua_getEmsdAddrFromRfc822(char *       pRfc822Addr,
			   OS_Uint32 *  pEmsdAddress,
			   ADDRESS *    pAddr)
{
    ReturnCode      rc  = Fail;
    char *	    p;
    char *	    pEnd;

    /* Skip the comment portion of the address */
    if ((p = strchr(pRfc822Addr, '<')) == NULL)
    {
	p = pRfc822Addr;
    }
    else
    {
	++p;
    }

    /* Look up the address to see if we can find an EMSD address for it */
    if (pfLookupEmsdAddress != NULL)
    {
	if ((pEnd = strchr(p, '>')) == NULL)
	{
	    pEnd = p + strlen(p);
	}

	/* Call the registered function to look it up. */
	rc = (* pfLookupEmsdAddress)(p, pEnd - p, pEmsdAddress, pAddr);
	if (rc == Fail)
	  printf ("mtsua:pfLookupEmsdAddress failed for '%s'\n", pRfc822Addr);
	return rc;
    }

    printf ("mtsuaLookupEmsdAddress failed for '%s'\n", pRfc822Addr);
    return Fail;
}
