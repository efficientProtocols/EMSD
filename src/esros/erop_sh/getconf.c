/*
 * 
 * Copyright (C) 1997-2002  Neda Communications, Inc.
 * 
 * This file is part of ESRO. An implementation of RFC-2188.
 * 
 * ESRO is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (GPL) as 
 * published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 * 
 * ESRO is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with ESRO; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.  
 * 
 */

/*
 *  
 *  Available Other Licenses -- Usage Of This Software In Non Free Environments:
 * 
 *  License for uses of this software with GPL-incompatible software
 *  (e.g., proprietary, non Free) can be obtained from Neda Communications, Inc.
 *  Visit http://www.neda.com/ for more information.
 * 
 */

/*+
 * File name: getconf.c
 *
 * Description: Get configuration parameters of ESROS engine from the 
 *		configuration file
 *
 * Functions:
 *   int getConf(char *configFile)
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: getconf.c,v 1.2 2002/10/25 19:37:33 mohsen Exp $";
#endif /*}*/

#include "estd.h"
#include "config.h"
#include "esro.h"
#include "tm.h"
#include "eh.h"

extern char *__applicationName;

extern int ESRO_setConf(Int param, Int value);	 /* Get Configuration */
extern int ESRO_getConf(Int param, Int *value);	 /* Set Configuration */

char errbuf[1024];		/* Error buffer */

#ifdef TM_ENABLED
LOCAL TM_ModuleCB *EROP_SH_modCB;	/* Trace Module Control Block */
#endif

typedef struct EsroConfMap	/* ESROS' Configuration Map */
{
    Int       esroParam;	/* ESROS Parameter number */
    OS_Uint32 value;		/* Parameter's value */
} EsroConfMap;

static struct 		/* ESROS' Configuration Parameters and Timers */
{
    struct				/* ESROS' Configuration parameters */
    {
	EsroConfMap portNumber;		/* Port number */
	EsroConfMap maxSAPs;		/* Maximum number of SAPs */
	EsroConfMap maxInvokes;		/* Maximum concurrent invocations */
	EsroConfMap maxReferenceNumbers;/* Maximum number of reference no's */
	EsroConfMap pduSize;		/* Maximum PDU size */
    } params;
    struct				/* ESROS' timers */
    {
	EsroConfMap acknowledgement;	/* Acknowledgement delay */
	EsroConfMap roundtrip;		/* Round trip delay */
	EsroConfMap retransmit;		/* Retransmission interval */
	EsroConfMap maxNSDULifeTime;	/* Max NSDU life time */
	EsroConfMap retransmitCount;	/* Maximum number of retransmission */
    } timers;
} config =
    {
	{
	 { ESRO_PortNumber,     0},
	 { ESRO_MaxSAPs,        0},
	 { ESRO_MaxInvokes,     0},
	 { ESRO_MaxReference,   0},
	 { ESRO_PduSize,        0}
	}, {
	 { ESRO_Acknowledgement,0},
	 { ESRO_Roundtrip,      0},
	 { ESRO_Retransmit,     0},
	 { ESRO_MaxNSDULifeTime,0},
	 { ESRO_RetransmitCount,0}
	}
    };


static struct ConfigParams {			/* Configuration parameters */
    char 	*pSectionName;			/* Section name */
    char 	*pTypeName;			/* Type name */
    OS_Uint32  	*ppValue;			/* Value */
} configParams[] = {
    { 	"Params",
  	"Port Number",
	&config.params.portNumber.value
    },
    { 	"Params",
	"Max SAPs",
	&config.params.maxSAPs.value
    },
    { 	"Params",
	"Max Invokes",
	&config.params.maxInvokes.value
    },
    { 	"Params",
	"Max Reference Numbers",
	&config.params.maxReferenceNumbers.value
    },
    { 	"params",
	"Pdu Size",
	&config.params.pduSize.value
    },
    { 	"Timer Values",
	"Acknowledgement",
	&config.timers.acknowledgement.value
    },
    {   "Timer Values",
	"Roundtrip",
	&config.timers.roundtrip.value
    },
    {   "Timer Values",
	"Retransmit",
	&config.timers.retransmit.value
    },
    {   "Timer Values",
	"Max NSDU Life Time",
	&config.timers.maxNSDULifeTime.value
    },
    {   "Timer Values",
	"Retransmit Count",
	&config.timers.retransmitCount.value
    }
};

static void *hConfig;

/*+
 * Function: getConf()
 *
 * Description: Get configuration
 *
 * Arguments: Config file name
 *
 * Returns: 0 if successful, -1 otherwise.
 *
-*/

int
getConf(char *configFile)
{
    ReturnCode		rc;
    struct ConfigParams *pConfigParam;
    int 		i;
    char *pConfigFile = NULL;

/*    static Bool virgin = TRUE;
    if ( !virgin ) {
	return 0;
    }
    virgin = FALSE;
*/
#ifdef TM_ENABLED
    static Bool virgin = TRUE;
    if ( virgin ) {
    virgin = FALSE;

    TM_INIT();

    if (TM_OPEN(EROP_SH_modCB, "EROP_SH") == NULL) {
    	EH_problem("EROP_init: TM_open EROP_SH failed");
	return ( FAIL );
    }
    }
#endif

#if !defined(OS_VARIANT_WinCE)
    /* NOTYET:  getenv() is non-portable ... */

    /* See if there's a configuration direction in the environment */
    pConfigFile = getenv("EROP_CONFIG_FILE");
#endif 

    if (configFile != NULL) {
	pConfigFile = configFile;
    }

    /*
     * Read the configuration file
     */

    /* Make sure we have a configuration file specified */
    if (pConfigFile == NULL)
    {
	sprintf(errbuf,
		"%s: No configuration file specified.\n"
		"    Either set environment variable EROP_CONFIG_FILE or\n"
		"    specify it with the '-c' option on the command line.",
		__applicationName);
	EH_fatal(errbuf);
    }

    TM_TRACE ((EROP_SH_modCB, TM_ENTER,
	      "Config file name: %s\n", pConfigFile));

    /* First, get a handle to the configuration data */
    if ((rc = CONFIG_open(pConfigFile, &hConfig)) != Success)
    {
        sprintf(errbuf, "esros: Could not open configuration file %s\n", 
		pConfigFile); 
	EH_fatal(errbuf);
    }

    i = 0;

    /* Get each of the configuration parameter values */
    for (pConfigParam = &configParams[0];
	 pConfigParam < &configParams[sizeof(configParams) /
				     sizeof(configParams[0])];
	 pConfigParam++)
    {
	if ((rc = CONFIG_getNumber(hConfig,
				   pConfigParam->pSectionName,
				   pConfigParam->pTypeName,
				   pConfigParam->ppValue)) != Success)
	{
	    sprintf(errbuf,
		    "%s: Configuration parameter\n\t%s/%s\n"
		    "\tnot found, reason 0x%04x",
		    __applicationName,
		    pConfigParam->pSectionName,
		    pConfigParam->pTypeName,
		    rc);
	    EH_problem(errbuf);
	    continue;
	}
#if 0
	if (ESRO_setConf(pConfigParam->pTypeName, 
#else
	if (ESRO_setConf((Int) i++, 
#endif
			   (Int) *pConfigParam->ppValue) != Success) {
	    sprintf(errbuf,
		    "%s: ESRO_setConf failed for parameter\n\t%s/%s\n",
		    __applicationName,
		    pConfigParam->pSectionName,
		    pConfigParam->pTypeName);
	    EH_problem(errbuf);
	}
    }

    return 0;

} /* getConf() */


#ifdef MTEST  	/* Module Test */
main()
{
    getConf();

    for (pConfigParam = &configParams[0];
	 pConfigParam < &configParams[sizeof(configParams) /
				     sizeof(configParams[0])];
	 pConfigParam++)
    {
	printf("\n%s %s %d", pConfigParam->pSectionName,
			     pConfigParam->pTypeName,
			     pConfigParam->ppValue);
    }
}
#endif
