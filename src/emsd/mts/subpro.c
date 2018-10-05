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
 * Available Other Licenses -- Usage Of This Software In Non Free Environments:
 * 
 * License for uses of this software with GPL-incompatible software
 * (e.g., proprietary, non Free) can be obtained from Neda Communications, Inc.
 * Visit http://www.neda.com/ for more information.
 * 
 */

/*
 * 
 * History:  Written 96.11.18
 */

static char *rcs = "$Id: subpro.c,v 1.1.1.1 2002/10/24 19:50:03 mohsen Exp $";

#ifndef NULLOK
#define NULLOK(x)	((x) ? (x) : "NULL")
#endif

#include "estd.h"
#include "config.h"
#include "tm.h"
#include "eh.h"
#include "addr.h"
#include "subpro.h"
#include "mm.h"


typedef struct _ConfigFile
{
    QU_ELEMENT;

    char *	    pFileName;
    QU_Head	    sections;
    QU_Head 	    macros;
    OS_Uint32	    referenceCount;
} ConfigFile;

typedef struct _Section
{
    QU_ELEMENT;

    char *	    pSectionName;
    QU_Head	    parameters;
} Section;

#ifdef TM_ENABLED
static void *	hTM;
#endif

static OS_Boolean emsdAuaEqual        (const char *n1, const char *n2);
static OS_Boolean emsdNsapEqual       (const N_SapAddr *n1, const N_SapAddr *n2);

static ReturnCode SUB_populateDevice (ConfigFile *hConfig, OS_Uint32 index, char *pSectionName);
static ReturnCode SUB_populateEntry  (ConfigFile *hConfig, OS_Uint32 index, char *pSectionName);
static OS_Boolean SUB_isSubscriber   (const char *pSectionName);
static ReturnCode SUB_getDeviceInfo  (const char *, MTS_DeliverMode *, MTS_SerialMode *, OS_Uint8 *, OS_Uint8 *);
static OS_Boolean SUB_isDevice       (const char *pSectionName);

void SUB_reInit(int);

SUB_Subscribers   subproProfile;
SUB_Profile      *subHashIndex[NSAP_HASH_SIZE];   /* 16K entry hash table */

void
SUB_reInit(int unused)
{
  TM_TRACE((hTM, SUB_TRACE_ACTIVITY, "Re-reading subscriber information"));
  SUB_init (NULL);
}

/*
 * SUB_init (hConfig)
 *
 * Given a handle to the subscriber profile, this routine creates an array of
 * SUB_profile entries, and indices to locate desired entries.
 *
 * The structures created are not accessed directly, but are used by the support
 * routine SUB_getProfile().
 *
 * Return:  Success or Fail
 */
ReturnCode
SUB_init (const char *pConfigFileName)
{
  ReturnCode          rc              = Success;
  OS_Uint32           index           = 0;
  OS_Uint32           count           = 0;
  OS_Uint32           deviceCount     = 0;
  OS_Uint32           skipped         = 0;
  OS_Uint32           hash            = 0;
  OS_Uint32           diff            = 0;
  Section            *phSection       = NULL;
  Section            *phFirstSection  = NULL;
  char               *pSectionName    = NULL;
  ConfigFile         *hConfig         = NULL;
  SUB_Profile        *pSubPro         = NULL;
  static char         configFile[256] = "";
  static OS_Boolean   initialized     = FALSE;

  if (initialized == FALSE) {
    initialized = TRUE;
    /* set up the module's trace structure */
    if (TM_OPEN(hTM, "SUB_") == NULL)
      {
	EH_problem ("Could not init the SUB_ module's trace structure\n");
	return Fail;
      }
  }
    
  TM_TRACE((hTM, SUB_TRACE_DETAIL, "SUB_init() [Hash size is %d]", sizeof subHashIndex));
  sigset (SIGUSR1, SUB_reInit);

  if (pConfigFileName && *pConfigFileName) {
    strncpy (configFile, pConfigFileName, sizeof configFile);
  } else {
    /* Use the previous file name if called with NULL, to aid in refreshing */
    if (*configFile == NULL) {
      return Fail;   /* no file name to use */
    }
  }

  /* clear the hash tables */
  memset (subHashIndex, 0, sizeof subHashIndex);

  /* reset the subscriber structure, if it has been init'd before */
  if (subproProfile.subproArray) {
    free(subproProfile.subproArray);
    subproProfile.subproArray = NULL;
  }
  if (subproProfile.deviceArray) {
    free(subproProfile.deviceArray);
    subproProfile.deviceArray = NULL;
  }
  if (subproProfile.indexSubscriberId) {
    free(subproProfile.indexSubscriberId);
    subproProfile.indexSubscriberId = NULL;
  }
  if (subproProfile.indexNsap) {
    free(subproProfile.indexNsap);
    subproProfile.indexNsap = NULL;
  }
  subproProfile.count = 0;
  subproProfile.deviceCount = 0;
  
  /* make sure CONFIG module is initialized */
  if (CONFIG_init() != Success) {
    TM_TRACE((hTM, SUB_TRACE_WARNING, "Error initializing CONFIG module; trace may not work"));
  }

  TM_TRACE((hTM, SUB_TRACE_DETAIL, "About to read configuration file (%s)", configFile));
  /* Open the profile configuration file, reads file into core */
  if ((rc = CONFIG_open(configFile, &hConfig)) != Success) {
    TM_TRACE((hTM, SUB_TRACE_WARNING, "Error reading configuration file (%s)", configFile));
    return rc;
  }

  TM_TRACE((hTM, SUB_TRACE_DETAIL, "Counting subscribers and devices"));
  /* count the number of subscribers */
  count = 0;
  phSection = NULL;
  while (CONFIG_nextSection (hConfig, &pSectionName, &phSection) == Success) {
    TM_TRACE((hTM, SUB_TRACE_COMPARE, "Looking at '%s'", NULLOK(pSectionName)));

    if (phFirstSection == NULL) {
      phFirstSection = phSection; /*** incompatibale ptrs ***/
    } else if (phFirstSection == phSection) {
      TM_TRACE((hTM, SUB_TRACE_DETAIL, "End of sections"));
      break;				       	/* break if we wrapped around */
    }
    /* test for subscriber section */
    if (SUB_isSubscriber (pSectionName) == TRUE) {
      count++;
    }
    /* test for subscriber section */
    if (SUB_isDevice (pSectionName) == TRUE) {
      deviceCount++;
    }
    pSectionName = NULL;
  }
  TM_TRACE((hTM, SUB_TRACE_ACTIVITY, "There are %d subscriber entries", count));
  TM_TRACE((hTM, SUB_TRACE_ACTIVITY, "There are %d device entries", deviceCount));

  TM_TRACE((hTM, SUB_TRACE_DETAIL, "Creating subscriber array"));
  /* allocate an array of the right size, plus some in case of untimely additions */
  subproProfile.subproArray = calloc (count + 16, sizeof (SUB_Profile));
  subproProfile.deviceArray = calloc (deviceCount + 16, sizeof (SUB_Device));

  /* populate the device array, done first as subscriber entries refer to them */
  index = 0;
  skipped = 0;
  phSection = NULL;
  while (CONFIG_nextSection (hConfig, &pSectionName, &phSection) == Success) {
    /* test for subscriber entry */
    if (SUB_isDevice (pSectionName) == FALSE) {
      pSectionName = NULL;
      if (skipped++ >= count + 100)
	break;
      else
	continue;
    }
    if (index >= deviceCount) {
      pSectionName = NULL;
      TM_TRACE((hTM, SUB_TRACE_ERROR, "Index overflow, truncating device list"));
      break;
    }
    SUB_populateDevice (&hConfig, index, pSectionName);
    pSectionName = NULL;

    index++;
  }
  subproProfile.deviceCount = index;

  /* populate the array from the configuration file */
  index = 0;
  skipped = 0;
  phSection = NULL;
  while (CONFIG_nextSection (hConfig, &pSectionName, &phSection) == Success) {
    /* test for subscriber entry */
    if (SUB_isSubscriber (pSectionName) == FALSE) {
      pSectionName = NULL;
      if (skipped++ >= count + 100)
	break;
      else
	continue;
    }
    if (index >= count) {
      pSectionName = NULL;
      TM_TRACE((hTM, SUB_TRACE_ERROR, "Index overflow, truncating subscriber list"));
      break;
    }
    SUB_populateEntry (&hConfig, index, pSectionName);
    pSectionName = NULL;

    index++;
  }
  subproProfile.count = index;

  /* sort the array by Subscriber ID */

  /* create a Subscriber ID based index -- not yet */

  /* create a NSAP based index */
  for (index = 0; index < count; index++) {
    pSubPro = &subproProfile.subproArray[index];
    hash  = pSubPro->deviceProfile.nsap.addr[2] << 8;
    hash += pSubPro->deviceProfile.nsap.addr[3];
    hash %= NSAP_HASH_SIZE;

    diff = 0;
    while (subHashIndex[hash] != NULL) {
      if (diff++ > NSAP_HASH_SIZE) {
        TM_TRACE((hTM, SUB_TRACE_ERROR, "NSAP Hash table full"));
	return Fail;
      }
      hash = ++hash % NSAP_HASH_SIZE;
    }
    subHashIndex[hash] = pSubPro;

    if (diff > 4) {
      TM_TRACE((hTM, SUB_TRACE_WARNING,
		"NSAP hash table busy (not fatal) hash=%lu [%lu]", hash, diff));
    }
  }
    
  return Success;
}

static ReturnCode
SUB_populateEntry (ConfigFile *hConfig, OS_Uint32 index, char *pSectionName)
{
    OS_Uint32           numHits    = 0;
    SUB_Profile        *pEntry     = NULL;
    SUB_DeviceProfile  *pDev       = NULL;
    char               *pSubLabel  = NULL;
    char               *pSubData   = NULL;
    char               *pTmp       = NULL;
    void               *pParam     = NULL;
    OS_Uint32           dummy      = 0;
    OS_Uint32           a, b, c, d;

    pEntry = &subproProfile.subproArray[index];
    pDev   = &pEntry->deviceProfile;

    /* iterate through the section's entries */
    while (CONFIG_nextParameter(hConfig, pSectionName, &pSubLabel, &pSubData, &pParam) == Success) {
      TM_TRACE((hTM, SUB_TRACE_DETAIL, "SUB_populate: '%s' = '%s'", pSubLabel, pSubData));

      if (!strcmp (pSubLabel, "EMSD-Device-Type")) {
	strncpy (pDev->deviceType, pSubData, sizeof pDev->deviceType - 1);
	SUB_getDeviceInfo (pSubData, &pDev->emsdDeliveryMode, &pDev->emsdSerialMode,
			   &pDev->protocolVersionMajor,	&pDev->protocolVersionMinor);

      } else if (!strcmp (pSubLabel, "Subscriber-ID")) {
	if (strlen (pSubData) >= sizeof pEntry->subscriberId) {
	  pSubData[sizeof pEntry->subscriberId - 1] = '\0';
	}
	pTmp = pEntry->subscriberId;
	while (*pSubData) {
	  if (isdigit (*pSubData))
	    *(pTmp++) = *pSubData;
	  pSubData++;
	}
	*pTmp = '\0';
	pTmp = NULL;
      } else if (!strcmp (pSubLabel, "Subscriber-Password")) {
	strncpy (pEntry->subscriberPassword, pSubData, sizeof pEntry->subscriberPassword - 1);
      } else if (!strcmp (pSubLabel, "Subscriber-Name")) {
	strncpy (pEntry->subscriberName, pSubData, sizeof pEntry->subscriberName - 1);
      } else if (!strcmp (pSubLabel, "Subscriber-Intl-Phone")) {
	/* not yet */
      } else if (!strcmp (pSubLabel, "Subscriber-NANP-Phone")) {
	/* not yet */
      } else if (!strcmp (pSubLabel, "EMSD-AUA")) {
	strncpy (pEntry->emsdAua, pSubData, sizeof pEntry->emsdAua - 1);
      } else if (!strcmp (pSubLabel, "EMSD-AUA-Nickname 1")) {
	strncpy (pEntry->emsdNickname1, pSubData, sizeof pEntry->emsdNickname1 - 1);
      } else if (!strcmp (pSubLabel, "EMSD-AUA-Nickname 2")) {
	strncpy (pEntry->emsdNickname2, pSubData, sizeof pEntry->emsdNickname2 - 1);
      } else if (!strcmp (pSubLabel, "EMSD-From")) {
	strncpy (pEntry->emsdFrom, pSubData, sizeof pEntry->emsdFrom - 1);
      } else if (!strcmp (pSubLabel, "EMSD-Reply-To")) {
	strncpy (pEntry->emsdReplyTo, pSubData, sizeof pEntry->emsdReplyTo - 1);
      } else if (!strcmp (pSubLabel, "EMSD-UA-NSAP")) {
	/* convert the ascii form into N_Sap form */
	numHits = sscanf (pSubData, "%lu.%lu.%lu.%lu.%lu", &a, &b, &c, &d, &dummy);
	if (numHits != 4) {
	  a = b = c = d = 0;
	  TM_TRACE((hTM, SUB_TRACE_ERROR,
		  "SUB_populate(%s): Bad NSAP Address: '%s' [%d hits]",
		    pSectionName, pSubData, numHits));
	}
	pDev->nsap.len     = 4;
	pDev->nsap.addr[0] = a;
	pDev->nsap.addr[1] = b;
	pDev->nsap.addr[2] = c;
	pDev->nsap.addr[3] = d;
      } else if (!strcmp (pSubLabel, "EMSD-UA-Password")) {
	strncpy (pEntry->emsdAua, pSubData, sizeof pEntry->emsdAua - 1);
      } else if (!strcmp (pSubLabel, "IVR-Default-AUA")) {
	/* not yet */
      } else if (!strcmp (pSubLabel, "IVR-Email-AUA")) {
	/* not yet */
      } else if (!strcmp (pSubLabel, "IVR-Fax-AUA")) {
	/* not yet */
      } else if (!strcmp (pSubLabel, "IVR-EMSD-AUA")) {
	/* not yet */
      } else if (!strcmp (pSubLabel, "IVR-Pager-AUA")) {
	/* not yet */
      } else if (!strcmp (pSubLabel, "IVR-PocketNet-AUA")) {
	/* not yet */
      } else if (!strcmp (pSubLabel, "IVR-VOX-Name")) {
	/* not yet */
      } else {
	TM_TRACE((hTM, SUB_TRACE_WARNING,
		  "SUB_populate(%s): Field not recognized: '%s'", pSectionName, pSubLabel));
      }
    }

    TM_TRACE((hTM, SUB_TRACE_ACTIVITY,
	      "SUB_populate: %s: NSAP Address: %lu.%lu.%lu.%lu",
	      pEntry->subscriberId, a, b, c, d));

    /* validate the entry */
    if ((SUB_isSubscriber (pEntry->subscriberId) == FALSE)
     || (pDev->nsap.addr[0] == 0)
     || (pDev->nsap.addr[1] == 0)
     || (pDev->nsap.addr[2] == 0)
     || (pDev->nsap.addr[3] == 0)
     || (pDev->emsdSerialMode == SM_Undefined)
     || (pDev->emsdDeliveryMode == DM_Undefined)
     || (*pDev->deviceType == NULL))
      {
	TM_TRACE((hTM, SUB_TRACE_WARNING,
		  "SUB_populate(%s): Suspect entry: Sub=%s, NSAP=%lu.%lu.%lu.%lu, "
		  "Dev='%s', Name='%s', DelMode=%d, SerialMode=%d", pSectionName,
		  pEntry->subscriberId, a, b, c, d, pDev->deviceType,
		  pEntry->subscriberName, pDev->emsdDeliveryMode, pDev->emsdSerialMode));
	return Fail;
      }	
      
  return Success;
}


static ReturnCode
SUB_populateDevice (ConfigFile *hConfig, OS_Uint32 index, char *pSectionName)
{
    OS_Uint32          major       = 0;
    OS_Uint32          minor       = 0;
    SUB_Device         *pEntry     = NULL;
    char               *pSubLabel  = NULL;
    char               *pSubData   = NULL;
    void               *pParam     = NULL;

    TM_TRACE((hTM, SUB_TRACE_DETAIL, "SUB_populateDevice '%s'", pSectionName));

    pEntry = &subproProfile.deviceArray[index];

    pEntry->emsdDeliveryMode = DM_Complete;
    pEntry->emsdSerialMode = SM_Serial;
    pEntry->protocolVersionMajor = 1;
    pEntry->protocolVersionMinor = 1;

    /* iterate through the section's entries */
    while (CONFIG_nextParameter(hConfig, pSectionName, &pSubLabel, &pSubData, &pParam) == Success) {
      TM_TRACE((hTM, SUB_TRACE_DETAIL, "SUB_populateDevice: '%s' = '%s'", pSubLabel, pSubData));
      if (!strcmp (pSubLabel, "EMSD-Device-Type")) {
	strncpy (pEntry->deviceType, pSubData, sizeof pEntry->deviceType - 1);

      } else if (!strcmp (pSubLabel, "EMSD-Delivery-Mode")) {
	pEntry->emsdDeliveryMode = strncmp (pSubData, "Efficient", 5) ? DM_Complete : DM_Efficient;

      } else if (!strcmp (pSubLabel, "EMSD-Serialized-Messages")) {
	pEntry->emsdSerialMode = strcmp (pSubData, "No") ? SM_Parallel : SM_Serial;

      } else if (!strcmp (pSubLabel, "EMSD-Protocol-Version")) {
	if (sscanf (pSubData, "%ld.%ld", &major, &minor) == 2) {
	  pEntry->protocolVersionMajor = major;
	  pEntry->protocolVersionMinor = minor;
	} else {
	  TM_TRACE((hTM, SUB_TRACE_ERROR, "Invalid protocol format: '%s'", pSubData));
	}
      } else {
	printf ("Should not get here\n");
      }
    }

    TM_TRACE((hTM, SUB_TRACE_ACTIVITY, "SUB_populateDevice: %s: %-8.8s %s",
	      pEntry->deviceType,
	      pEntry->emsdSerialMode == SM_Serial ? "Serial" : "Parallel",
	      pEntry->emsdDeliveryMode == DM_Complete ? "Complete" : "Efficient"));
      
    return Success;
}

static ReturnCode
SUB_getDeviceInfo (const char *pDeviceName,
		   MTS_DeliverMode  *delivery,
		   MTS_SerialMode   *serial,
		   OS_Uint8         *major,
		   OS_Uint8         *minor)
{
  OS_Uint32    i = 0;

  for (i = 0; i < subproProfile.deviceCount; i++) {
    if (!strcmp (pDeviceName, subproProfile.deviceArray[i].deviceType)) {
      *delivery = subproProfile.deviceArray[i].emsdDeliveryMode;
      *serial   = subproProfile.deviceArray[i].emsdSerialMode;
      *major    = subproProfile.deviceArray[i].protocolVersionMajor;
      *minor    = subproProfile.deviceArray[i].protocolVersionMinor;

      TM_TRACE((hTM, SUB_TRACE_COMPARE, "Found device: '%s'", pDeviceName));
      return (Success);
    }
  }
  /* default values if no device is found */
  TM_TRACE((hTM, SUB_TRACE_ERROR, "Could not find device: '%s'", pDeviceName));

  *delivery = DM_Complete;
  *serial   = SM_Serial;
  *major    = 1;
  *minor    = 1;

  return (Fail);
}

SUB_Profile *
SUB_getEntry (const char *subscriberId, const N_SapAddr *pNsap)
{
  OS_Uint32   i      = 0;
  OS_Uint32   hash   = 0;
  OS_Boolean  found  = FALSE;

  if (subproProfile.subproArray == NULL) {
    printf ("SUB_init() has not been sucessfully called\n");
    return (NULL);
  }
  if (subscriberId && pNsap) {
    for (i = 0; i < subproProfile.count; i++) {
      if (emsdAuaEqual(subproProfile.subproArray[i].subscriberId, subscriberId)) {
	if (emsdNsapEqual(&subproProfile.subproArray[i].deviceProfile.nsap, pNsap)) {
	  found = TRUE;
	  break;
	}
      }
    }
  } else if (subscriberId) {
    for (i = 0; i < subproProfile.count; i++) {
      if (emsdAuaEqual(subproProfile.subproArray[i].subscriberId, subscriberId)) {
	found = TRUE;
	break;
      }
    }
  } else if (pNsap) {
    hash = ((pNsap->addr[2] << 8) + pNsap->addr[3]) % NSAP_HASH_SIZE;

    for (i = 0; i < subproProfile.count; i++) {
      if (subHashIndex[hash] == NULL) {
	TM_TRACE((hTM, SUB_TRACE_WARNING, "NSAP=%d.%d.%d.%d  hash=0x%04x",
		  pNsap->addr[0], pNsap->addr[1],  pNsap->addr[2], pNsap->addr[3], hash));
	TM_TRACE((hTM, SUB_TRACE_WARNING, "NSAP not in hash table: %lu [%lu]", hash, i));
	return (NULL);
      }
      if (emsdNsapEqual(&subHashIndex[hash]->deviceProfile.nsap, pNsap)) {
	TM_TRACE((hTM, SUB_TRACE_DETAIL, "NSAP found: %lu [%lu]", hash, i));
	return (subHashIndex[hash]);
      }
      hash = ++hash % NSAP_HASH_SIZE;
    }
  }

  return (found ? &subproProfile.subproArray[i] : NULL);
}

static OS_Boolean
emsdAuaEqual (const char *s1, const char *s2)
{
    OS_Uint32       numFields	= 0;
    OS_Uint32       hi		= 0;
    OS_Uint32       lo		= 0;
    OS_Uint32       dummy	= 0;
    OS_Uint32       num1	= 0;
    OS_Uint32       num2	= 0;

    numFields = sscanf(s1, "%lu.%lu.%lu", &hi, &lo, &dummy);
    switch (numFields) {
    case 1:
      num1 = hi;
      break;
    case 2:
      num1 = (hi * 1000) + lo;
      break;
    default:
      return (FALSE);
    }

    numFields = sscanf(s2, "%lu.%lu.%lu", &hi, &lo, &dummy);
    switch (numFields) {
    case 1:
      num2 = hi;
      break;
    case 2:
      num2 = (hi * 1000) + lo;
      break;
    default:
      return (FALSE);
    }

    return (num1 == num2 ? TRUE : FALSE);
}

static OS_Boolean
emsdNsapEqual (const N_SapAddr *n1, const N_SapAddr *n2)
{
  if (n1 == NULL || n2 == NULL)
    return (FALSE);

  if ((n1->addr[0]  == n2->addr[0])
   && (n1->addr[1]  == n2->addr[1])
   && (n1->addr[2]  == n2->addr[2])
   && (n1->addr[3]  == n2->addr[3])) {
    return (TRUE);
  }
  return (FALSE);
}

OS_Boolean
SUB_isSubscriber (const char *pSectionName)
{
  int          numFields        = 0;
  int          i1, i2, i3;

  TM_TRACE((hTM, SUB_TRACE_COMPARE, "SUB_isSubscriber(%s)", NULLOK(pSectionName)));

  if (pSectionName == NULL) {
    TM_TRACE((hTM, SUB_TRACE_COMPARE, "SUB_isSubscriber(NULL), no match"));
    return (FALSE);
  }
  if (strncmp (pSectionName, "Subscriber", 10) == 0) {
    TM_TRACE((hTM, SUB_TRACE_COMPARE, "SUB_isSubscriber(), label match"));
    return (TRUE);
  }
  numFields = sscanf (pSectionName, "%d.%d.%d", &i1, &i2, &i3);
  if (numFields == 1 || numFields == 2) {
    TM_TRACE((hTM, SUB_TRACE_COMPARE, "SUB_isSubscriber(), numeric match"));
    return (TRUE);
  }

  TM_TRACE((hTM, SUB_TRACE_COMPARE, "SUB_isSubscriber(), no match"));
  return (FALSE);
}


static OS_Boolean
SUB_isDevice (const char *pSectionName)
{
  TM_TRACE((hTM, SUB_TRACE_COMPARE, "SUB_isDevice(%s)", NULLOK(pSectionName)));

  if (pSectionName == NULL) {
    TM_TRACE((hTM, SUB_TRACE_COMPARE, "SUB_isDevice(NULL), no match"));
    return (FALSE);
  }
  if (strncmp (pSectionName, "Device ", 7) == 0) {
    TM_TRACE((hTM, SUB_TRACE_COMPARE, "SUB_isDevice(), label match"));
    return (TRUE);
  }

  return (FALSE);
}

