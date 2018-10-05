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
 * File name: esros.c
 *
 * Module: Scenario Interpreter.
 *
 * Description: Functions implementing scenario interpreter features and ESROS
 * 	   	primitives.
 *
-*/

/*
 * Author: Mohsen Banan. Tweaks by Mark Mc
 * History:
 *
 */

#include "estd.h"
#include "eh.h"
#include "oe.h"
#include "tm.h"
#include "tmr.h"
#include "inetaddr.h"

#include "esro.h"
#ifndef MSDOS
#include "nmm_sh.h"
#endif

#include "esrossi.h"
#ifdef MSDOS
#include "ytab.h"
#else
#include "y.tab.h"
#endif
#include "extfuncs.h"

int debug;

extern int ignore_faults;

esrossi_Info locInfo = { {0} } ;
esrossi_Info remInfo = { {0} } ;

STATIC struct ESRO_Event ESRO_event;	/* ESROS Event */
ESRO_InvokeId invokeId;			/* Invoke Id */
ESRO_EncodingType encodingType;		/* Encoding type */

ESRO_OperationValue operationValue;	/* Operation value */
#define SHELL_CMD_OP	2

ESRO_SapSel  locSapSel = 12;		/* Local SAP selector */
ESRO_SapDesc locSapDesc;		/* Local SAP Descriptor */

ESRO_SapSel remEsroSapSel = 13;		/* Remote SAP Selector */
T_SapSel remTsapSel = {2, {0x07, 0xD2} };  	  /* UDP Port Number (2002) */
N_SapAddr remNsapAddr = { 4, {198, 62, 92, 10} }; /* Remote IP Address */

#ifdef TM_ENABLED
PUBLIC TM_ModDesc G_tmDesc;
#endif

Void
sapbind(int sap, int functionalUnit)
{
   if (ESRO_sapBind(&locSapDesc, (locSapSel = sap), functionalUnit) < 0 ) {
	EH_problem("sapbind: ESRO_SapBind failed");
	if (!ignore_faults)
		G_exit(1);
   }
}

Void
saprelease(int sap)
{
    if ( ESRO_sapUnbind((ESRO_SapSel) sap) < 0 ) {
	EH_problem("saprelease: ESRO_SapUnbind failed");
	if (!ignore_faults)
		G_exit(2);
    }
}

int
do_esros_queue()
{
    if (ESRO_getEvent(locSapDesc, &ESRO_event, TRUE) < 0) {
	return(-1);
    } 
    return ( 0 );
}


int
qcheck()
{
    return (0);
}

int
invokereq(int remoteSap, int remotePort, N_SapAddr *remoteAddr, int
	  operationVal, int encodingType, Byte *invokeParameter, 
	  unsigned long int userInvokeRef)
{
    ESRO_RetVal gotVal;

    INET_portNuToTsapSel((MdInt) remotePort, &remTsapSel);

    if ((gotVal = ESRO_invokeReq(&invokeId,
     	  		         (ESRO_UserInvokeRef)userInvokeRef,
     			         locSapDesc,
     			         (ESRO_SapSel) remoteSap,
     			         &remTsapSel, 
     			         remoteAddr, 
     			         (ESRO_OperationValue) operationVal,
     			         (ESRO_EncodingType) encodingType, 
     			         (Int) strlen((const char *)invokeParameter),
			         invokeParameter)) < 0) {
	switch(gotVal) {
	case -1:
	    EH_problem("invokereq: Invocation failed");
            return -1;
	case -2:
	    EH_problem("invokereq: Invocation failed, connectionless engine, doesn't support large data\n");
            return -1;
	default:
	    EH_problem("invokereq: Invocation failed");
            return -1;
    	}
    }

    return 0;
}

int
errorreq(int encodingType, int errorValue, Byte *value, 
	 unsigned long int userInvokeRef)
{
    ESRO_RetVal gotVal;

    if ((gotVal = ESRO_errorReq(invokeId, (ESRO_UserInvokeRef)userInvokeRef, 
			        (ESRO_EncodingType) encodingType,
			        errorValue, strlen((const char *)value), 
				value)) < 0) {
	EH_problem("errorreq: ESRO_errorReq failed.\n");
	G_exit (3);
    }

    return 0;
}

int
resultreq(int encodingType, Byte *value,
	  unsigned long int userInvokeRef)
{
    ESRO_RetVal gotVal;

    if ((gotVal = ESRO_resultReq(invokeId, (ESRO_UserInvokeRef)userInvokeRef, 
			    (ESRO_EncodingType) encodingType,
			    strlen((const char *)value), value)) < 0) {
	EH_problem("resultreq: ESRO_resultReq failed.\n");
	G_exit(4);
    }

    return 0;
}

Void
rawevent(int n)
{
	char name[100];
	sprintf(name,"Raw event %d\n",n);
    while (do_esros_queue() < 0) {
	;
    }
    if (ESRO_event.type != n) {
	badevent(&ESRO_event, name);
    }
	
}

int
invokeind(int operationValue, int encodingType, char *value)
{
    Int retVal = SUCCESS;

    char *name = "INVOKE.indication";

    while (do_esros_queue() < 0) {
	;
    }
    if (ESRO_event.type != ESRO_INVOKEIND) {
	badevent(&ESRO_event, name);
	retVal = (FAIL);
    }

    if (ESRO_event.un.invokeInd.operationValue != operationValue) {
	badOpValue(name,ESRO_event.un.invokeInd.operationValue, operationValue);
	retVal = (FAIL);
    }
    if (ESRO_event.un.invokeInd.encodingType != encodingType) {
	badEncodingType(name,ESRO_event.un.invokeInd.encodingType,encodingType );
	retVal = (FAIL);
    }

    if (strncmp(value, (const char*)ESRO_event.un.invokeInd.data,
		ESRO_event.un.invokeInd.len) != 0) {
	badData(name, (char*)ESRO_event.un.invokeInd.data,
		ESRO_event.un.invokeInd.len, value);
	retVal = (FAIL);
    }

    invokeId = ESRO_event.un.invokeInd.invokeId;

    return retVal;
}

int
errorconf(unsigned long int userInvokeRef)
{
    Int retVal = SUCCESS;
    char *name = "ERROR.confirmation";

    while (do_esros_queue() < 0) 
		;

    if (ESRO_event.type != ESRO_ERRORCNF) {
	badevent(&ESRO_event, name);
	retVal = (FAIL);
    }

    if (ESRO_event.un.errorCnf.userInvokeRef != 
	(ESRO_UserInvokeRef)userInvokeRef) {
	badUserInvokeRef(name, ESRO_event.un.errorCnf.userInvokeRef,
			 (ESRO_UserInvokeRef)userInvokeRef );
	retVal = FAIL;
    }


    return (retVal);
}

int
resultconf(unsigned long int userInvokeRef)
{
    Int retVal = SUCCESS;

    char *name = "RESULT.confirmation";

    while (do_esros_queue() < 0) 
		;

    if (ESRO_event.type != ESRO_RESULTCNF) {
	badevent(&ESRO_event, name);
	retVal = (FAIL);
    }

    if (ESRO_event.un.resultCnf.userInvokeRef != 
	(ESRO_UserInvokeRef)userInvokeRef) {
	badUserInvokeRef(name, ESRO_event.un.resultCnf.userInvokeRef,
			 (ESRO_UserInvokeRef)userInvokeRef );
	retVal = FAIL;
    }


    return (retVal);
}

int
resultind(int encodingType, char *value, unsigned long int userInvokeRef)
{
    Int retVal = SUCCESS;

    char *name = "RESULT.indication";

    while (do_esros_queue() < 0) {
	;
    }

#ifdef TM_ENABLED
     	TM_trace(G_tmDesc, TM_ENTER,
		 "resultInd: invokeId=%d, paramter=%s\n",
		 ESRO_event.un.resultInd.invokeId,
		 ESRO_event.un.resultInd.data);
#endif

    if (ESRO_event.type != ESRO_RESULTIND) {
	badevent(&ESRO_event, name);
	retVal = FAIL;
    }
    if (ESRO_event.un.resultInd.encodingType != encodingType) {
	badEncodingType(name,ESRO_event.un.resultInd.encodingType,encodingType );
	retVal = FAIL;
    }

    if (strncmp(value, (const char *)ESRO_event.un.resultInd.data,
		ESRO_event.un.resultInd.len) != 0) {
	badData(name, (char *)ESRO_event.un.resultInd.data,
		ESRO_event.un.resultInd.len, value);
	retVal = FAIL;
    }

    if (ESRO_event.un.resultInd.userInvokeRef != 
	(ESRO_UserInvokeRef)userInvokeRef) {
	badUserInvokeRef(name, ESRO_event.un.resultInd.userInvokeRef,
			 (ESRO_UserInvokeRef)userInvokeRef );
	retVal = FAIL;
    }

    return (retVal);
}

int
errorind(int encodingType, int errorValue, char *value, 
	 unsigned long int userInvokeRef)
{
    Int retVal = (SUCCESS);

    char *name = "ERROR.indication";

    while (do_esros_queue() < 0) {
	;
    }

    if (ESRO_event.type != ESRO_ERRORIND) {
	badevent(&ESRO_event, name);
	retVal = (FAIL);
    }
    if (ESRO_event.un.errorInd.encodingType != encodingType) {
	badEncodingType(name,ESRO_event.un.errorInd.encodingType,encodingType );
	retVal = (FAIL);
    }

    if (strncmp(value, (const char *)ESRO_event.un.errorInd.data,
		ESRO_event.un.errorInd.len) != 0) {
	badData(name, (char *)ESRO_event.un.errorInd.data,
		ESRO_event.un.errorInd.len, value);
	retVal = (FAIL);
    }

    if (ESRO_event.un.errorInd.errorValue != errorValue) {
	badErrorValue(name, ESRO_event.un.errorInd.errorValue, errorValue);
	return (FAIL);
    }

    if (ESRO_event.un.errorInd.userInvokeRef != 
	(ESRO_UserInvokeRef)userInvokeRef) {
	badUserInvokeRef(name, ESRO_event.un.errorInd.userInvokeRef,
			 (ESRO_UserInvokeRef)userInvokeRef );
	retVal = FAIL;
    }

    return retVal;
}

int
failureind(int failureValue, unsigned long int userInvokeRef)
{
    char *name = "FAILURE.indication";
    Int retVal = SUCCESS;

    while (do_esros_queue() < 0) {
	;
    }

    if (ESRO_event.type != ESRO_FAILUREIND) {
	badevent(&ESRO_event, name);
    	retVal = (FAIL);
    }

    if (ESRO_event.un.failureInd.failureValue != failureValue) {
	badFailureValue(name, ESRO_event.un.failureInd.failureValue,
			failureValue);
	retVal = (FAIL);
    }

    if (ESRO_event.un.failureInd.userInvokeRef != 
	(ESRO_UserInvokeRef)userInvokeRef) {
	badUserInvokeRef(name, ESRO_event.un.failureInd.userInvokeRef,
			 (ESRO_UserInvokeRef)userInvokeRef );
	retVal = (FAIL);
    }


    return (retVal);
}

/*
 *  Log unexpected event.
 */

Void
badevent(ESRO_Event *ESRO_event, char *name)
{
    char *got;

    switch (ESRO_event->type) {
    case ESRO_INVOKEIND:
	    got = "INVOKE.indication";
	    break;
    case ESRO_FAILUREIND:
	    got = "INVOKE.failed";
	    break;
    case ESRO_RESULTIND:
	    got = "RESULT.indication";
	    break;
    case ESRO_ERRORIND:
	    got = "ERROR.indication";
	    break;
    case ESRO_RESULTCNF:
 	    got = "RESULT.confirmation";
	    break;
    default:
	    got = "Bad event code";
	    break;
    }
    log("%s: Unexpected event (%x)", got, ESRO_event->type);
    log("Expected event: %s", name);
    }

Void
badOpValue(char *eventName, int actual, int expected)
{
    log("%s: Bad operation value (%x)",eventName,actual);
    log("Expected value: %x", expected);
}

Void
badEncodingType(char *eventName, int actual, int expected)
{
    log("%s: Bad encoding type (%x)",eventName,actual);
    log("Expected value: %x", expected);
}

Void
badData(char *eventName,  char *actualData, int actualLen, char *expected)
{
    log("%s: Bad data (%.*s)",eventName, actualLen, actualData);
    log("Expected value: %s",expected);
}

Void
badFailureValue(char *eventName, int actual, int expected)
{
    log("%s: Bad failure value (%x)", eventName, actual);
    log("Expected value: %x", expected);
}


Void
badErrorValue(char *eventName, int actual, int expected)
{
    log("%s: Bad error value (%x)", eventName, actual);
    log("Expected value: %x", expected);
}

Void
badUserInvokeRef(char *eventName, 
		 ESRO_UserInvokeRef actual, 
		 ESRO_UserInvokeRef expected)
{
    log("%s: Bad User Invoke Ref value (%x)", eventName, actual);
    log("Expected value: %x", expected);
}

/* inhibit the next N PDUs */
Void 
udp_pc_inhibit( Int direction, Int next )
{
    log("udp_pc_inhibit: direction = %d. next = %d", direction, next);
    /* NMRO_udp_pc_inhibit( direction, next ); */
}

/* drop some percent of all PDUs */
Void 
udp_pc_lossy( Int direction, Int percent )
{
    log("udp_pc_inhibit: direction = %d. percent = %d", direction, percent);
    /* NMRO_udp_pc_lossy( direction, percent ); */
}

