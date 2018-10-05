/*
 *  This file is part of the Open C Platform (OCP) Library. OCP is a
 *  portable library for development of Data Communication Applications.
 * 
 *  Copyright (C) 1995 Neda Communications, Inc.
 * 	Prepared by Pean Lim (pean@neda.com)
 * 
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.  This library is
 *  distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 *  License for more details.  You should have received a copy of the GNU
 *  Library General Public License along with this library; if not, write
 *  to the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139,
 *  USA.
 * 
 */

/*+
 * File name: lic.c
 *
 * Description: License Checker
 *
 * Functions:
 *	LIC_check()	
 */

/*
 * Author: Pean Lim
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: lic.c,v 1.7 1997/03/12 18:20:18 kamran Exp $";
#endif /*}*/


#include "pf.h"
#include "eh.h"
#include "config.h"
#include "inetaddr.h"
#include "lic.h"

/* function prototypes */
static SuccFail
lic_yyyymmddToZuluDateTime(String yyyymmdd, OS_ZuluDateTime *zdt);


/* typedefs */

typedef struct LIC_Env {
    char 	*pLicensee;	/* licensee */
    char 	*pStartDate;	/* start date */
    char 	*pEndDate;	/* end date */
    char 	*pHostID;	/* machine licensed */
    char 	*pCheckDigits;	/* license integrity */
} LIC_Env_t;


static LIC_Env_t g_licEnv;

static struct LicenseParam
{
    char *	    pSectionName;
    char *	    pTypeName;
    char **	    ppValue;
} licenseParams[] = {
    {
	"License",
	"Licensee",
	&g_licEnv.pLicensee
    },
    {
	"License",
	"Start Date",
	&g_licEnv.pStartDate
    },
    {
	"License",
	"End Date",
	&g_licEnv.pEndDate
    },
    {
	"License",
	"Host ID",
	&g_licEnv.pHostID
    },
    {
	"License",
	"Check Digits",
	&g_licEnv.pCheckDigits
    }
};


/*<
 * Function:	LIC_init
 *
 * Description: 
 *
 * Arguments:	
 *
 * Returns:	
 *
 * 
>*/
PUBLIC SuccFail
LIC_check(char *pLicenseFile)
{
    Void		*hConfig;	
    struct LicenseParam	*pLicenseParam;
    ReturnCode		rc;
    char		tmpbuf[1024];

    /* Open the license configuration file. */  
    if ((rc = CONFIG_open(pLicenseFile, &hConfig)) != Success) {
	sprintf(tmpbuf,
		"LIC: Could not open license file (%.*s), "
		"reason 0x%04x\n",
		(int) (sizeof(tmpbuf) / 2), pLicenseFile, rc);
	EH_fatal(tmpbuf);
    }

    /* 
     * read the license configuration
     */

    /* Get each of the configuration parameter values */
    for (pLicenseParam = &licenseParams[0];
	 pLicenseParam < &licenseParams[sizeof(licenseParams) / sizeof(licenseParams[0])];
	 pLicenseParam++) {
	if ((rc = CONFIG_getString(hConfig,
				   pLicenseParam->pSectionName,
				   pLicenseParam->pTypeName,
				   pLicenseParam->ppValue)) != Success)
	    {
		sprintf(tmpbuf,
			"LIC: Configuration parameter\n\t%s/%s\n"
			"\tnot found, reason 0x%04x",
			pLicenseParam->pSectionName,
			pLicenseParam->pTypeName,
			rc);
		EH_fatal(tmpbuf);
	    };
    }

    /* check license file integrity */
    if ( LIC_checkHash(g_licEnv.pLicensee,
			g_licEnv.pStartDate,
			g_licEnv.pEndDate,
			g_licEnv.pHostID,
			g_licEnv.pCheckDigits) == Success ) {
#ifndef MSDOS
	printf("license file integrity check OK!\n"); /* *** NOTYET add tracing */
#endif
    }
    else {
	printf("license file integrity check failed!\n"); /* *** NOTYET add tracing */
	return Fail;
    }

    /* check start/end dates */
    {
	OS_ZuluDateTime	now;
	OS_ZuluDateTime	beg;
	OS_ZuluDateTime	end;
	OS_Uint32 nowDaysSince70;	
	OS_Uint32 begDaysSince70;	
	OS_Uint32 endDaysSince70;

	rc = OS_currentDateTime(&now, NULL);
	lic_yyyymmddToZuluDateTime(g_licEnv.pStartDate, &beg);
	lic_yyyymmddToZuluDateTime(g_licEnv.pEndDate, &end);
	
	/* we simply use (12 * 30) day years for this */ 
	nowDaysSince70 = ((now.year - 1970L) * 360L) + 
			 ((now.month - 1L) * 30L) + (now.day - 1);
	begDaysSince70 = ((beg.year - 1970L) * 360L) + 
			 ((beg.month - 1L) * 30L) + (beg.day - 1);
	endDaysSince70 = ((end.year - 1970L) * 360L) + 
			 ((end.month - 1L) * 30L) + (end.day - 1);

	if ((nowDaysSince70 < begDaysSince70) || 
	    (nowDaysSince70 > endDaysSince70)) {
	    printf("license file dates check failed!\n"); /* *** NOTYET add tracing */
	    return Fail;
	}
	else {
#ifndef MSDOS
	    printf("license file dates check OK!\n"); /* *** NOTYET add tracing */
#endif
	}
    }    

    /* check host ID */
    {
	char 		buf[128];
	SuccFail	rc;

#define DUMMY_ADDRESS "2130706433"

	rc = INET_gethostid(buf, sizeof(buf));
	if ((rc == Fail) || 
	    ( (strcmp(buf, g_licEnv.pHostID) != 0)  &&
	    (strcmp(buf, DUMMY_ADDRESS) != 0)) ) {
	    fprintf(stderr, "Host ID <%s> does not match <%s>!\n", 
		    buf, g_licEnv.pHostID); /* *** NOTYET add tracing */
	    return Fail;
	}
	else {
	    fprintf(stdout, "license file host ID check ok!\n"); /* *** NOTYET add tracing */
	}
    }

    return Success;
}


SuccFail
LIC_computeHash(char *licensee, /* IN */
		char *startDate, /* IN */
		char *endDate,	/* IN */
		char *hostID,	/* IN */
		char **hash)	/* OUT */
{
    int	hash_string_len;
    OS_Uint32 crc;
    char *hash_string;
    char *hash_result;

    hash_string_len = strlen(licensee) + strlen(startDate) + 
		      strlen(endDate) + strlen(hostID);
    hash_string = (char *)malloc(hash_string_len + 1);
    hash_result = (char *)malloc(hash_string_len + 1); 
    if (( NULL == hash_string ) 
	|| ( NULL == hash_result )) {
	fprintf(stderr, "out of memory!\n"); /* *** use EH_fatal(); */
	exit(1);
    }
    
    strcpy(hash_string, "");
    strcat(hash_string, licensee);
    strcat(hash_string, startDate);
    strcat(hash_string, endDate);
    strcat(hash_string, hostID);
    
    /* hashing algorithm */
    crc = PF_crc16(hash_string, hash_string_len, 0);
    sprintf(hash_result, "701%ld%ld", crc, crc ^ 0xFFFF);

#if 0
    fprintf(stdout, "Hash of <%s> is <%s>\n", hash_string, hash_result);	
#endif /* 0 */
    *hash = hash_result;
    return Success;
}



SuccFail
LIC_checkHash(char *licensee,	/* IN */
	      char *startDate,	/* IN */
	      char *endDate,	/* IN */
	      char *hostID,	/* IN */
	      char *checkDigits)/* IN */
{
    char *checkDigits_computed;

    LIC_computeHash(licensee, startDate, endDate, hostID, &checkDigits_computed);
    if ( strcmp(checkDigits, checkDigits_computed) == 0 ) {
	return Success;
    }
    else {
	return Fail;
    }
}


/*
 * helper functions 
 */
static
SuccFail
lic_yyyymmddToZuluDateTime(String yyyymmdd, /* IN */
			   OS_ZuluDateTime *zdt) /* OUT */
{
    char yyyy[5];
    char mm[3];
    char dd[3];
    int	 num;

    if ( strlen(yyyymmdd) != 8 )
	return Fail;

    yyyy[0] = '\0';
    strncat(yyyy, yyyymmdd, 4);

    mm[0] = '\0';
    strncat(mm, yyyymmdd + 4, 2);

    dd[0] = '\0';
    strncat(dd, yyyymmdd + 6, 2);

    sscanf(yyyy, "%d", &num);
    zdt->year = num;

    sscanf(mm, "%d", &num);
    zdt->month = num;

    sscanf(dd, "%d", &num);
    zdt->day = num;

    zdt->hour = 0;
    zdt->minute = 0;
    zdt->second = 0;	
    
    return Success;
}



