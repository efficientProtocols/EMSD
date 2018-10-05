/*
 *  Copyright (C) 1995,1999  Neda Communications, Inc. All rights reserved.
 * 
 *  This software is furnished under a license and use, duplication,
 *  disclosure and all other uses are restricted to the rights specified
 *  in the written license between the licensee and copyright holders.
 * 
 */

/*+
 * File name: udp_po.c
 *
 * Module: UDP_IF point of observation.
 *
 * Description: Functions for initialization, and writting the PDU log file.
 *
 * Functions:
 * 
 * PUBLIC SuccFail UDP_PO_init(String errFile, String logFile)
 * PUBLIC SuccFail UDP_PO_logIn(Byte *pdu, Int size, N_SapAddr *remNsapAddr)
 * PUBLIC SuccFail UDP_PO_logOut(Byte *pdu, Int size, N_SapAddr *remNsapAddr)
 * PUBLIC SuccFail UDP_PO_errIn(Byte *pdu, Int size) 
 * PUBLIC SuccFail UDP_PO_errOut(Byte *pdu, Int size)
 *
-*/

/*
 * Author: Mohsen Banan, Hugh Shane
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: udp_po.c,v 1.10 1999/11/14 04:56:24 mohsen Exp $";
#endif /*}*/

#include "estd.h"
#include "eh.h"
#include "os.h"
#include "tm.h"
#include "du.h"
#include "udp_po.h"
#include "local.h"

#ifdef UDP_PO_	/*{*/

#ifdef TM_ENABLED
TM_ModuleCB *UDP_PO_modCB;
#endif

Bool UDP_noLogSw = 1;
#if defined(OS_VARIANT_WinCE)
STATIC FILE *udp_outLogFp;
STATIC FILE *udp_errLogFp;
#else
STATIC Int fdOutLog, fdErrLog;
#endif
STATIC OS_Uint32 start_time = 0;
STATIC OS_Uint32 absoluteTime = 1;


/*<
 * Function:    UDP_PO_init
 *
 * Description: Initialize UDP logging
 *
 * Arguments:   Error log pointer, log file pointer
 *
 * Returns:     SuccFail
 * 
>*/
PUBLIC SuccFail 
UDP_PO_init(String errFile, String logFile)
{
    static Bool virgin = TRUE;
    FILE *errLogFp, *outLogFp;

    if ( !virgin ) {
	return FAIL;
    }
    virgin = FALSE;

    if ((TM_OPEN(UDP_PO_modCB, "UDP_PO_")) == NULL) {
    	EH_fatal("UDP_PO_init:    Error in TM_open UDP_PO_");
    }

    if ( (outLogFp = fopen (logFile, "w")) != NULL) {
#if defined(OS_VARIANT_WinCE)
	udp_outLogFp = outLogFp;
#else
       	fdOutLog= fileno(outLogFp);
#endif
    }
    else {
	perror("UDP_PO_init");
        EH_fatal("UDP_PO_init:    Cannot open output log file");
    }

    if ( (errLogFp = fopen (errFile, "w")) != NULL) {
#if defined(OS_VARIANT_WinCE)
    	udp_errLogFp = errLogFp;
#else
    	fdErrLog = fileno(errLogFp);
#endif
    } else {
	perror("UDP_PO_init");
        EH_fatal("UDP_PO_init:    Cannot open error log file");
	exit(1);
    }

    if (absoluteTime) {
        start_time = 0;
    } else {
        start_time = OS_timeHourMinSec();
    }


    return SUCCESS;

} /* UDP_PO_init() */


/*<
 * Function:    UDP_PO_logit
 *
 * Description: Write a record in the log file.
 *
 * Arguments:   Code of the log record, data of record, size of the data.
 *
 * Returns:     0 if successful
 *
>*/

LOCAL SuccFail
udp_po_logit(Int direction, char *dp, Int siz, N_SapAddr *remNsapAddr)
{
    UDP_PO_LogRecord tm;

    if (UDP_noLogSw) {
	return (SUCCESS);
    }

    if (siz < 0 /* || siz > NOTYET */ ) {
	return (FAIL);
    }

    tm.magic = 0x4711;

    tm.tmx   = OS_timeHourMinSec() - start_time; /* NOTYET - deal with wraparound? */
        
    tm.size  = siz;
    tm.code  = direction;

#if defined(OS_VARIANT_WinCE)
    if (fwrite(&tm, sizeof(tm), 1, udp_outLogFp) < 1) {
#else
    if (write(fdOutLog, &tm, sizeof(tm)) < 0) {
#endif
	perror("UDP_PO_logit: ");
	EH_problem("UDP_PO_logit: LOGGING DEACTIVATED "
		   "(writing to log file failed)");
	UDP_noLogSw = 1;
	return (FAIL);
    }

    if (siz) { 
#if defined(OS_VARIANT_WinCE)
	if (fwrite(dp, siz, 1, udp_outLogFp) < 1) {
#else
	if (write(fdOutLog, dp, siz) < 0) {
#endif
	    perror("UDP_PO_logit: ");
	    EH_problem("UDP_PO_logit: LOGGING DEACTIVATED "
		       "(writing to log file failed)");
	    UDP_noLogSw = 1;
	    return (FAIL);
	}
    }

    if (remNsapAddr) { 
#if defined(OS_VARIANT_WinCE)
	if (fwrite(remNsapAddr, sizeof(*remNsapAddr), 1, udp_outLogFp) < 1) {
#else
	if (write(fdOutLog, remNsapAddr, sizeof(*remNsapAddr)) < 0) {
#endif
	    perror("UDP_PO_logit: ");
	    EH_problem("UDP_PO_logit: LOGGING DEACTIVATED "
		       "(writing to log file failed)");
	    UDP_noLogSw = 1;
	    return (FAIL);
	}
    }

    return (SUCCESS);

} /* udp_po_logit() */


/*<
 * Function:    UDP_PO_logIn(Byte *pdu, Int size, N_SapAddr *remNsapAddr)
 *              UDP_PO_logOut(Byte *pdu, Int size, N_SapAddr *remNsapAddr)
 *
 * Description: Write a record in the log file.
 *
 * Arguments:   N-1 PDU pointer, PDU size.
 *
 * Returns:     
 *
>*/

PUBLIC SuccFail 
UDP_PO_logIn(Byte *pdu, Int size, N_SapAddr *remNsapAddr)
{
  return udp_po_logit(UDP_PO_PDU_IN, pdu, size, remNsapAddr);
}


/*<
 * Function:    UDP_PO_logOut
 *
 * Description: Log pdu.
 *
 * Arguments:	PDU, size.
 *
 * Returns: 
 *
>*/

PUBLIC SuccFail 
UDP_PO_logOut(Byte *pdu, Int size, N_SapAddr *remNsapAddr)
{
  return udp_po_logit(UDP_PO_PDU_OUT, pdu, size, remNsapAddr);
}

#endif /*}*/

