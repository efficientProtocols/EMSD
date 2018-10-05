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

static char *rcs = "$Id: devprof.c,v 1.1.1.1 2002/10/24 19:50:03 mohsen Exp $";


#include "estd.h"
#include "addr.h"
#include "inetaddr.h"
#include "profile.h"
#include "mm.h"
#include "nnp.h"
#include "mtsua.h"
#include "mts.h"
#include "tm.h"


/*
 * Forward declarations
 */
FORWARD static ReturnCode
getProfile(MTS_DeviceProfile * pProfile,
	   char * pSource,
	   char * pData);

FORWARD static OS_Boolean
stringEqual(char * p1,
	    char * p2,
	    OS_Uint32 type);

FORWARD static OS_Boolean
emsdAuaEqual(char * pAua1,
	    char * pAua2,
	    OS_Uint32 type);

/*
 * Define the Originator/Recipient Profile attribute names and types that we
 * care about in this application.
 */
typedef struct ProfileAttributes
{
    char * 	    pAttributeName;
    OS_Uint32	    pAttributeType;
    OS_Boolean	 (* pfEqual)(char * pValue1,
			     char * pValue2,
			     OS_Uint32 type);
} ProfileAttributes;


ReturnCode
mts_initDeviceProfile(char * pSubscriberProfileFile)
{
    ReturnCode			rc;
    ProfileAttributes * 	pProfileAttributes;
    static ProfileAttributes    profileAttributes[] =
    {
	{
	    P_NAME_EMSD_AUA,
	    P_TYPE_EMSD_AUA,
	    emsdAuaEqual,
	},
	{
	    P_NAME_EMSD_AUA_NICKNAME,
	    P_TYPE_EMSD_AUA_NICKNAME,
	    emsdAuaEqual,
	},
	{
	    P_NAME_EMSD_UA_NSAP,
	    P_TYPE_EMSD_UA_NSAP,
	    stringEqual,
	},
	{
	    P_NAME_EMSD_UA_PASSWORD,
	    P_TYPE_EMSD_UA_PASSWORD,
	    stringEqual,
	},
	{
	    P_NAME_EMSD_DEVICE_TYPE,
	    P_TYPE_EMSD_DEVICE_TYPE,
	    stringEqual,
	},
	{
	    P_NAME_EMSD_DELIVERY_MODE,
	    P_TYPE_EMSD_DELIVERY_MODE,
	    stringEqual,
	},
	{
	    P_NAME_EMSD_SERIAL_MODE,
	    P_TYPE_EMSD_SERIAL_MODE,
	    stringEqual,
	},
	{
	    P_NAME_EMSD_PROTOCOL_VERSION,
	    P_TYPE_EMSD_PROTOCOL_VERSION,
	    stringEqual,
	},
	{
	    NULL,
	    NULL,
	    NULL
	},
    };
    
    /* Open the profile */
    if ((rc = PROFILE_open(pSubscriberProfileFile, &globals.hProfile)) != Success)
    {
	return rc;
    }
    
    /* Add the attribute names and types */
    for (pProfileAttributes = profileAttributes;
	 pProfileAttributes->pAttributeName != NULL;
	 pProfileAttributes++)
    {
	if ((rc =
	     PROFILE_addAttribute(globals.hProfile,
				  pProfileAttributes->pAttributeName,
				  pProfileAttributes->pAttributeType,
				  pProfileAttributes->pfEqual)) != Success)
	{
	    return rc;
	}
    }
    return Success;
}

ReturnCode
mts_getDeviceProfileByNsap(N_SapAddr * pRemoteNsap,
			   MTS_DeviceProfile ** ppProfile)
{
    int				i;
    char * 			p;
    char			nsapBuf[32];
    ReturnCode			rc;
    MTS_DeviceProfile * 	pProfile;
    
    /* Allocate a Profile structure */
    if ((pProfile = OS_alloc(sizeof(MTS_DeviceProfile))) == NULL)
    {
	return ResourceError;
    }
    
    /*
     * Find the user profile for which the remote address matches.
     */
    
    /* Write the NSAP in a format which can be compared */
    p = nsapBuf;
    for (i = 0; i < pRemoteNsap->len; i++)
    {
	sprintf(p, "%s%d", (i == 0) ? "" : ".", pRemoteNsap->addr[i]);
	p += strlen(p);
    }
    TM_TRACE((globals.hTM, MTS_TRACE_PROFILE, 
	     "Getting profile for NSAP %s", nsapBuf));
    
    /* Now, go find the profile which uses this NSAP */
    if (PROFILE_searchByType(globals.hProfile,
			     P_TYPE_EMSD_UA_NSAP,
			     nsapBuf,
			     &pProfile->hProfile) != Success)
    {
	/* Don't operate on behalf of an unknown user */
	MM_logMessage(globals.hMMLog,
		      globals.tmpbuf,
		      "Access attempt by unrecognized user: NSAP [%s]\n",
		      nsapBuf);
	
	OS_free(pProfile);
	return MTS_RC_DeviceProfileNotFound;
    }
    
    /* Get the profile data */
    if ((rc = getProfile(pProfile, "NSAP", nsapBuf)) != Success)
    {
	OS_free(pProfile);
	return rc;
    }
    
    /* Give 'em what they came for */
    *ppProfile = pProfile;
    
    return Success;
}


ReturnCode
mts_getDeviceProfileByAua(char *                pAua,
			  MTS_DeviceProfile **  ppProfile)
{
    ReturnCode			rc;
    MTS_DeviceProfile * 	pProfile;
    
    TM_TRACE((globals.hTM, MTS_TRACE_PROFILE, 
	     "Getting profile for AUA %s", pAua));

    /* Allocate a Profile structure */
    if ((pProfile = OS_alloc(sizeof(MTS_DeviceProfile))) == NULL)
    {
	return ResourceError;
    }
    
    /*
     * Find the user profile where the EMSD-AUA or one of the
     * EMSD-AUA Nicknames match this address
     */
    if (PROFILE_searchByType(globals.hProfile,
			     P_TYPE_EMSD_AUA,
			     pAua,
			     &pProfile->hProfile) != Success)
    {
	/* Don't operate on behalf of an unknown user */
	MM_logMessage(globals.hMMLog,
		      globals.tmpbuf,
		      "Delivery attempt to unrecognized user: AUA [%s]\n",
		      pAua);
	
	OS_free(pProfile);
	return MTS_RC_DeviceProfileNotFound;
    }
    
    /* Get the profile data */
    if ((rc = getProfile(pProfile, "AUA", pAua)) != Success)
    {
        printf ("+++  getProfile failed for %s\n", pAua);
	OS_free(pProfile);
	return rc;
    }
    
    /* Give 'em what they came for */
    *ppProfile = pProfile;
    
    return Success;
}


static ReturnCode
getProfile(MTS_DeviceProfile * pProfile,
	   char *              pSource,
	   char *              pData)
{
    int                  rc           = 0;
    char * 	         p            = NULL;
    unsigned int         a, b, c, d;		/* will hold NSAP address octets */

    TM_TRACE((globals.hTM, MTS_TRACE_PROFILE, 
	     "\n\nGetting profile [%s] [%s]", pSource, pData));

    /* Get the NSAP for this user */
    rc = PROFILE_getAttributeValue(globals.hProfile, pProfile->hProfile, P_NAME_EMSD_UA_NSAP, &pProfile->pNsap);
    if (rc != Success)
    {
	MM_logMessage(globals.hMMLog, globals.tmpbuf,
		      "UA NSAP not found for user with %s (%s)\n", pSource, pData);
	return MTS_RC_MissingProfileAttribute;
    }
    
    /* Convert the NSAP into its binary form */
    pProfile->nsap.len = sscanf(pProfile->pNsap, "%u.%u.%u.%u", &a, &b, &c, &d);
    if (pProfile->nsap.len < 2)
    {
	MM_logMessage(globals.hMMLog,
		      globals.tmpbuf,
		      "UA NSAP is illegal format for user with "
		      "%s (%s)\n",
		      pSource, pData);
	return MTS_RC_IllegalProfileAttribute;
    }

    /* Fill in the NSAP structure */
    pProfile->nsap.addr[0] = a;
    pProfile->nsap.addr[1] = b;
    pProfile->nsap.addr[2] = c;
    pProfile->nsap.addr[3] = d;

    /* Get the EMSD AUA for this user */
    if (PROFILE_getAttributeValue(globals.hProfile,
				  pProfile->hProfile,
				  P_NAME_EMSD_AUA,
				  &pProfile->pEmsdAua) != Success)
    {
	MM_logMessage(globals.hMMLog,
		      globals.tmpbuf,
		      "EMSD AUA not found for user with "
		      "%s (%s)\n",
		      pSource, pData);
	return MTS_RC_MissingProfileAttribute;
    }
    
    /* Get the EMSD UA Password for this user */
    if (PROFILE_getAttributeValue(globals.hProfile,
				  pProfile->hProfile,
				  P_NAME_EMSD_UA_PASSWORD,
				  &pProfile->pEmsdUaPassword) != Success)
    {
	MM_logMessage(globals.hMMLog,
		      globals.tmpbuf,
		      "EMSD UA Password not found for user with %s (%s)\n",
		      pSource, pData);
	return MTS_RC_MissingProfileAttribute;
    }

    /* Get the EMSD UA Device Type for this user */
    if (PROFILE_getAttributeValue(globals.hProfile,
				  pProfile->hProfile,
				  P_NAME_EMSD_DEVICE_TYPE,
				  &pProfile->pDeviceType) != Success)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_PROFILE, 
		 "*** EMSD Device Type not found"));
	MM_logMessage(globals.hMMLog,
		      globals.tmpbuf,
		      "Device type not found for user with %s (%s)\n",
		      pSource, pData);
	return MTS_RC_MissingProfileAttribute;
    }

    /*
     * Find the device profile for this subscriber
     */
    TM_TRACE((globals.hTM, MTS_TRACE_PROFILE, 
	     "Getting device profile for '%s'", pProfile->pDeviceType));

    pProfile->hProfile = NULL;
    if (PROFILE_searchByType(globals.hProfile,
			     P_TYPE_EMSD_DEVICE_TYPE,
			     pProfile->pDeviceType,
			     &pProfile->hProfile) != Success)
    {
	/* Don't operate on behalf of an unknown device */
        TM_TRACE((globals.hTM, MTS_TRACE_PROFILE, 
		 "*** Device '%s' not found", pProfile->pDeviceType));

	MM_logMessage(globals.hMMLog,
		      globals.tmpbuf,
		      "Delivery attempt to unrecognized device: Device Type [%s]\n",
		      &pProfile->pDeviceType);
	
	OS_free(pProfile);
	return MTS_RC_DeviceProfileNotFound;
    }

    /* go to the section for the device type */
    if (PROFILE_getAttributeValue(globals.hProfile,
				  pProfile->hProfile,
				  P_NAME_EMSD_DEVICE_TYPE,
				  &pProfile->pDeviceType) != Success)
    {
        TM_TRACE((globals.hTM, MTS_TRACE_PROFILE, "Device profile not found: %s", pProfile->pDeviceType));
	MM_logMessage(globals.hMMLog,
		      globals.tmpbuf,
		      "Device profile not found for user with %s (%s)\n",
		      pSource, pData);
	return MTS_RC_MissingProfileAttribute;
    }
    
    /* Get the Protocol Version for this user */
    if (PROFILE_getAttributeValue(globals.hProfile,
				  pProfile->hProfile,
				  P_NAME_EMSD_PROTOCOL_VERSION,
				  (char **) &p) != Success)
    {
	MM_logMessage(globals.hMMLog,
		      globals.tmpbuf,
		      "Protocol version number not found for user with %s (%s)",
		      pSource, pData);
	return MTS_RC_UnrecognizedProtocolVersion;;
    }
    TM_TRACE((globals.hTM, MTS_TRACE_PROFILE, "Protocol Version = %s", p));
    
    /* Parse and validate the protocol version */
    if (sscanf(p, "%u.%u", &a, &b) != 2 || a != 1 || b > 1)
    {
	MM_logMessage(globals.hMMLog,
		      globals.tmpbuf + 32,
		      "Invalid version number for user with %s (%s)\n",
		      pSource, pData);
	return MTS_RC_UnrecognizedProtocolVersion;;
    }
    
    /* Add 'em to the device profile */
    pProfile->protocolVersionMajor = a;
    pProfile->protocolVersionMinor = b;

    /* Get the EMSD Deliver Mode, if we're not using protocol version 1.0 */
    if (pProfile->protocolVersionMajor == 1 &&
	pProfile->protocolVersionMinor == 0)
    {
	/* Deliver mode not used for version 1.0 protocol */
	pProfile->pDeliverMode = NULL;
    }
    else
    {
	/* Get the Deliver Mode value */
	if (PROFILE_getAttributeValue(globals.hProfile,
				      pProfile->hProfile,
				      P_NAME_EMSD_DELIVERY_MODE,
				      &pProfile->pDeliverMode) != Success)
	{
	    MM_logMessage(globals.hMMLog, globals.tmpbuf,
			  "EMSD Deliver Mode not found for user with %s (%s)\n",
			  pSource, pData);
	    return MTS_RC_MissingProfileAttribute;
	}
	TM_TRACE((globals.hTM, MTS_TRACE_PROFILE, 
		 "Delivery Mode = %s", pProfile->pDeliverMode));
	
	/* Make sure it's a legal value */
	if (OS_strcasecmp(pProfile->pDeliverMode, "Efficient") != 0 &&
	    OS_strcasecmp(pProfile->pDeliverMode, "Complete") != 0)
	{
	    MM_logMessage(globals.hMMLog,
			  globals.tmpbuf,
			  "EMSD Deliver Mode value illegal for user with %s (%s)",
			  pSource, pData);
	    return MTS_RC_MissingProfileAttribute;
	}
    }
    
    TM_TRACE ((globals.hTM, MTS_TRACE_DETAIL,
	       "%s (%s): v%d.%d NSAP=%d.%d.%d.%d %s",
	       pSource, pData,
	       pProfile->protocolVersionMajor, pProfile->protocolVersionMinor, 
	       pProfile->nsap.addr[0], pProfile->nsap.addr[1],
	       pProfile->nsap.addr[2], pProfile->nsap.addr[3], 
	       pProfile->pDeliverMode ? pProfile->pDeliverMode : "DeliverMode=NULL"));

    return Success;
}


static OS_Boolean
stringEqual(char * p1,
	    char * p2,
	    OS_Uint32 type)
{
    return (OS_strcasecmp(p1, p2) == 0);
}


static OS_Boolean
emsdAuaEqual(char * pAua1,
	    char * pAua2,
	    OS_Uint32 type)
{
    OS_Boolean	    rc;
    char *	    pAt1;
    char *	    pAt2;

    /* If there's a domain part in AUA 1, strip it off for now */
    if ((pAt1 = strchr(pAua1, '@')) != NULL)
    {
	/* There is.  Null terminate at this point. */
	*pAt1 = '\0';
    }

    /* If there's a domain part in AUA 2, strip it off for now */
    if ((pAt2 = strchr(pAua2, '@')) != NULL)
    {
	/* There is.  Null terminate at this point. */
	*pAt2 = '\0';
    }

    /* Now, are the two the same? */
    rc = (OS_strcasecmp(pAua1, pAua2) == 0);

    /* Restore the atsigns */
    if (pAt1 != NULL)
    {
	*pAt1 = '@';
    }

    if (pAt2 != NULL)
    {
	*pAt2 = '@';
    }

    return rc;
}
