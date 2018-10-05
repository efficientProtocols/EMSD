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
 * File: ro_prim.c  (User Shell: Remote Operations Primitives)
 *
 * Description: User shell
 *
 * Functions:
 *   lous_putAction(USQ_PrimQuInfo *primQu, LOPQ_Primitive *action)
 *   lous_addInvokeCB(ESRO_SapDesc sapDesc, ESRO_InvokeId invokeId)
 *   lous_removeInvokeCB(InvokeCB *invokeCB)
 *   ESRO_init (void)
 *   ESRO_sapBind(ESRO_SapDesc *sapDesc,  OUTgoing 
 *   		 ESRO_SapSel sapSel)
 *   ESRO_invokeReq(ESRO_InvokeId *invokeId,        	  OUTgoing 
 *   	       ESRO_UserInvokeRef userInvokeRef,
 *   	       ESRO_SapDesc locSapDesc,
 *   	       ESRO_SapSel remESROSap,
 *   	       T_SapSel *remTsap,
 *   	       N_SapAddr *remNsap,
 *   	       ESRO_OperationValue opValue,
 *   	       ESRO_EncodingType encodingType,
 *	       Int parameterLen,
 *	       Byte *parameter)
 *   ESRO_resultReq(ESRO_InvokeId invokeId,
 *   	       ESRO_UserInvokeRef userInvokeRef,
 *   	       ESRO_EncodingType encodingType,
 *	       Int parameterLen,
 *	       Byte *parameter)
 *   ESRO_errorReq(ESRO_InvokeId invokeId,
 *   	       ESRO_UserInvokeRef userInvokeRef,
 *   	       ESRO_EncodingType encodingType,
 *   	       ESRO_ErrorValue errorValue,
 *	       Int parameterLen,
 *	       Byte *parameter)
 *   ESRO_getEvent(ESRO_SapDesc sapDesc,
 *   	       ESRO_Event *event,
 *   	       Bool wait)
 *   lous_getEvent(USQ_PrimQuInfo *primQu, LOPQ_Event *event, Bool wait)
 *
-*/

/*
 * 
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: ro_prim.c,v 1.2 2002/10/25 19:37:47 mohsen Exp $";
#endif /*}*/

#include  "estd.h"
#include  "eh.h"	/* Exception Handler */
#include  "tm.h"	/* Trace Module */
#include "sf.h"		/* System facilities*/
#include "usq.h"	/* User Shell Queue */
#include "psq.h"	/* Provider Shell Queue */
#include "seq.h"	/* Sequence Module */
#include "esro.h"	/* ESROS */
#include "esro_sh.h"	/* ESROS Shell */
#include "local.h"	/* Definitions local to module */
#include "sch.h"	/* Scheduler Module */
#include "target.h"	/* Target environment */
#include "config.h"	/* Configuration Module */

#ifdef IMQ_SIMU
extern EROP_init();
extern getConf();
#endif 


STATIC LOPQ_Primitive prim;	/* ESROS priminitive */
STATIC LOPQ_Event lous_event;	/* ESROS event */

InvokeCB invokeCBMap[LOPQ_INVOKE_CBS];	/* Invoke control block map */
EsroSapCB sapCBMap[LOPQ_SAPS];		/* SAP control block map */

#ifndef MSDOS
extern int errno;
#endif

extern SuccFail G_heartBeat();
extern esro_storeEvent(USQ_PrimQuInfo *primQu);
extern esro_eventAvailable(USQ_PrimQuInfo *primQu);
extern SuccFail lous_removeInvokeCB(InvokeCB *invokeCB);

typedef struct lous_EventQuInfo {		/* Event queue */
    struct lous_EventQuInfo *next;
    struct lous_EventQuInfo *prev;
    USQ_PrimQuInfo *quInfo;			/* Primitive queue */
} lous_EventQuInfo;

typedef struct lous_EventQuInfoSeq {		/* Event queue head */
    struct lous_EventQuInfo *first;
    struct lous_EventQuInfo *last;
} lous_EventQuInfoSeq;

STATIC lous_EventQuInfoSeq eventQuInfoSeq;	/* Event queue */
STATIC SEQ_PoolDesc eventQuInfoPool;		/* Event queue pool */

USQ_PrimQuInfo *SUS_primQu;

#ifdef TM_ENABLED
LOCAL TM_ModuleCB *ESRO_modCB;
#endif

Int lous_getEvent (USQ_PrimQuInfo *primQu, LOPQ_Event *event, Bool wait);

static Int imqSimuCompileSw = 1;	/* Flag to show compile time options */

#ifdef  IMQ_SIMU
#define EROP_K_UdpSapSel 2002
#define EROP_SAPS        200
#define EROP_INVOKES     200
#define NREFS 256  
#define INVOKE_PDU_SIZE  1500

extern Int udpSapSel;
extern Int nuOfSaps;
extern Int nuOfInvokes;
extern Int invokePduSize;
#endif

typedef struct G_Env {			/* Global module: Environement */
    Bool hardReset;			/* Hard reset */
    Bool softReset;			/* Soft reset */
} G_Env;

extern G_Env G_env;			/* Global module: Environement */


/*<
 * Function:    lous_putAction  (Limited Operation User Shell, Put Action)
 *
 * Description: Put action.
 *
 * Arguments:   Primitive queue, primitive.
 *
 * Returns:     Number of bytes sent if successful, otherwise a negative number.
 * 
>*/

LOCAL Int
lous_putAction(USQ_PrimQuInfo *primQu, LOPQ_Primitive *action)
{
    return USQ_putAction(primQu, (Ptr) action, sizeof(*action));
}


/*<
 * Function:    lous_addInvokeCB  (Limited Operation User Shell, Add Invoke
 *              Control Block)
 *
 * Description: Add invoke control block.
 *
 * Arguments:   SAP descriptor, invoke ID.
 *
 * Returns:     Pointer to invoke control block.
 * 
>*/

InvokeCB *
lous_addInvokeCB(ESRO_SapDesc sapDesc, ESRO_InvokeId invokeId)
{
    InvokeCB *invokeCB;				/* Invoke control block */
    EsroSapCB *sapCB;				/* SAP control block */

    invokeCB = &invokeCBMap[invokeId];
    sapCB    = &sapCBMap[sapDesc];

    if ( invokeCB->state != INACTIVE_STATE ) {
        TM_TRACE((ESRO_modCB, TM_ENTER, 
		 "lous_addInvokeCB: The invokeCB already active"));
	if (lous_removeInvokeCB(invokeCB) < 0) {
            TM_TRACE((ESRO_modCB, TM_ENTER, 
		     "lous_addInvokeCB: lous_removeInvokeCB failed."));
	    return ( (InvokeCB *)0 );
	}
    }
    invokeCB->state = ACTIVE_STATE;

    /* Associate the Tsap to the Tcep */
    invokeCB->sapCB = sapCB;

    /* If the Data Channels were to created on a per 
     * connection Bases, this is where it should be done.
     */

    /* Put this Tcep in Tsap's connection Queues */
    QU_INIT(invokeCB);
    QU_INSERT(&sapCB->invokeQuHead, invokeCB);

    return ( invokeCB );
}


/*<
 * Function:    lous_removeInvokeCB  (Limited Operation User Shell, remove
 *              Invoke Control Block)
 *
 * Description: Remove invoke control block.
 *
 * Arguments:   Invoke control block. 
 *
 * Returns:     0 on successful completion, -1 on unsuccessful completion.
 * 
>*/

SuccFail
lous_removeInvokeCB(InvokeCB *invokeCB)
{
    TM_TRACE((ESRO_modCB, TM_ENTER, 
	     "lous_removeInvokeCB: invokeCB=0x%x invokeId=%d\n", 
	     invokeCB, invokeCB - invokeCBMap));
	
    invokeCB->state = INACTIVE_STATE;

    QU_REMOVE(invokeCB);
/*    QU_INIT(invokeCB); */

    return (SUCCESS);
}


/*<
 * Function:    ESRO_init
 *
 * Description: Initialize ESROS.
 *
 * Arguments:   None.
 *
 * Returns:     0 on successful completion, a negative error value on
 *              unsuccessful completion.
 *
>*/

PUBLIC ESRO_RetVal
#ifdef IMQ_SIMU
ESRO_init (char *pConfigFile)
#else
ESRO_init ()
#endif
{
    InvokeCB *indexInvokeCB;
    EsroSapCB *indexSap;

    static Bool virgin = TRUE;
    
    if ( !virgin ) {
        EH_problem("WARNING: ESRO_init() is called "
		   "more than once (harmless)\n");
    	return ( SUCCESS );
    }
    virgin = FALSE;

#ifdef TM_ENABLED
    TM_INIT();

    if ( ! (ESRO_modCB = TM_open("ESRO_") )) {
	EH_problem("ESRO_init:      Error in TM_open ESRO_");
	return ( FAIL );
    }

    TM_TRACE((ESRO_modCB, TM_ENTER, 
	     "ESRO_init:      ESRO_ opened\n"));
#endif

    if (SCH_init(MAX_SCH_TASKS) ==  SCH_NOT_VIRGIN) { 
    	TM_TRACE((ESRO_modCB, TM_ENTER, 
		 "WARNING: ESROS application didn't initialize scheduler.\n"));
    }

    USQ_init();   	/* USQ_ initialize IMQ_ */

    for (indexInvokeCB=invokeCBMap; indexInvokeCB <= ENDOF(invokeCBMap); 
	 ++indexInvokeCB) {
	QU_INIT(indexInvokeCB);
	indexInvokeCB->state = INACTIVE_STATE;
	indexInvokeCB->sapCB = (EsroSapCB *)0;
    }

    for (indexSap=sapCBMap; indexSap <= ENDOF(sapCBMap); ++indexSap) {
	QU_INIT(&indexSap->invokeQuHead);
	QU_INIT(&indexSap->evQuHead);
	indexSap->state = INACTIVE_STATE;
    }

    eventQuInfoPool = SEQ_poolCreate(sizeof(*eventQuInfoSeq.first), 0);
    QU_INIT(&eventQuInfoSeq);

#ifdef IMQ_SIMU
    udpSapSel     = EROP_K_UdpSapSel;
    nuOfSaps      = EROP_SAPS;
    nuOfInvokes   = EROP_INVOKES;
    invokePduSize = INVOKE_PDU_SIZE;

    CONFIG_init();
    getConf(pConfigFile);	/* get configuration parameters from file */
    
    if (EROP_init(udpSapSel, nuOfSaps, nuOfInvokes, invokePduSize) == FAIL) {	
							/* Protocol Engine */
	char buf[128];
	sprintf (buf, "%s: Engine initialization (EROP_init) failed\n", 
		 "ESRO_CB_init");		/* improve: give error value */
	EH_fatal(buf);
    }
#endif

#ifdef TM_ENABLED
    TM_TRACE((ESRO_modCB, TM_ENTER, 
     	     "**** ESRO_CB_init compiled <<<WITH TM_ENABLED>>> *****\n"));
#else
    TM_TRACE((ESRO_modCB, TM_ENTER, 
             "**** ESRO_CB_init compiled <<<WITHOUT TM_ENABLED>>> *****\n"));
#endif

    return ( SUCCESS );
}


/*<
 * Function:    ESRO_sapBind
 *
 * Description: ESRO SAP bind.
 *
 * Arguments:   SAP descriptor (outgoing argument), SAP selector.
 *
 * Returns:     0 on successful completion, a negative error value on 
 *              unsuccessful completion.
 * 
>*/

PUBLIC ESRO_RetVal
ESRO_sapBind(ESRO_SapDesc *sapDesc, /* OUTgoing */
	     ESRO_SapSel sapSel,
	     ESRO_FunctionalUnit functionalUnit)
{
    LOPQ_PStatus   *pStatus;	/* Primitive Status */
    USQ_PrimQuInfo *primQu;	/* Primitive Queue */

    TM_TRACE((ESRO_modCB, TM_ENTER, "ESRO_sapBind:   Entered\n"));

    if ( sapSel > ESRO_MAX_SAP_NO ) { /* no need to check min because unsigned */
	EH_problem("ESRO_sapBind: Invalid Sap number\n");
        TM_TRACE((ESRO_modCB, TM_ENTER, 
	         "               Accepted range: %d to %d\n",
		 ESRO_MIN_SAP_NO, ESRO_MAX_SAP_NO));
	return  -7; 
    }

    if (functionalUnit != ESRO_3Way  &&  functionalUnit != ESRO_2Way) {
 	EH_problem("ESRO_sapBind: Invalid functional unit (should be 2 or 3)\n");
        TM_TRACE((ESRO_modCB, TM_ENTER, 
	        "              Given value: %d,   Accepted values: %d and %d\n", 
		functionalUnit, ESRO_3Way, ESRO_2Way));
	return -6;
    }

    if ( (SUS_primQu = primQu = USQ_primQuCreate()) == NULL) {
        TM_TRACE((ESRO_modCB, TM_ENTER, 
	        "ESRO_sapBind: USQ_primQuCreate failed\n"));
	return -5;    /* Creation of primitive Qu failed */
    }

    prim.type = LOPQ_SAP_BIND; 	
    prim.un.activate.locSapSel = sapSel;
    prim.un.activate.functionalUnit = functionalUnit;

    /* Queues Keys can be derrived from chnlName */
    strcpy(prim.un.activate.chnlName, primQu->chnlName);

    if (lous_putAction(primQu, &prim) < 0) {
        TM_TRACE((ESRO_modCB, TM_ENTER, 
	        "ESRO_sapBind: lous_putAction failed\n"));
	SF_memRelease(primQu);
	return -4; 
    }

    /* Now BLOCK and Wait For the Status */

    if (lous_getEvent(primQu, &lous_event, TRUE) < 0) {
        TM_TRACE((ESRO_modCB, TM_ENTER, 
	        "ESRO_sapBind: lous_getEvent failed\n"));
	SF_memRelease(primQu);
	return -3;
    }

    pStatus = (LOPQ_PStatus *) &lous_event;
    switch (pStatus->type) {
    case LOPQ_SAP_BIND_STAT :
	if ( pStatus->status ) {
    	    TM_TRACE((ESRO_modCB, TM_ENTER,
		     "ESRO_sapBind: sapBind failed sapSel=%d\n", sapSel));
	    TM_HEXDUMP(ESRO_modCB, ESRO_TRPRIM, "sapBind status", 
                       (Byte *) pStatus, 20);

	    SF_memRelease(primQu);
	    return (pStatus->status);

	} else {
	    *sapDesc = pStatus->un.activateStat.locSapDesc;
	}
	break;

    default:
	EH_problem("ESRO_sapBind: ");
	TM_HEXDUMP(ESRO_modCB, ESRO_TRPRIM, "sapBind: bad status", 
                   (Byte *) pStatus, 20);
	SF_memRelease(primQu);
      	return (-2);
    }

    if ( *sapDesc >= DIMOF(sapCBMap)) {
	EH_problem("ESRO_sapBind: sapCBMap overflow");
	SF_memRelease(primQu);
	return (-1);
    }

    sapCBMap[*sapDesc].sapSel  = sapSel;
    sapCBMap[*sapDesc].sapDesc = *sapDesc;
    sapCBMap[*sapDesc].primQu  = primQu;
    sapCBMap[*sapDesc].state   = ACTIVE_STATE;

    TM_TRACE((ESRO_modCB, TM_ENTER,
	"ESRO_sapBind: (before ret) sapSel=%d   sapDesc=%d\n", sapSel, *sapDesc));

    return ( 0 );

} /* ESRO_sapBind() */


/*<
 * Function:    ESRO_sapUnbind
 *
 * Description: ESRO SAP unbind.
 *
 * Arguments:   SAP selector.
 *
 * Returns:     0 on successful completion, a negative error value on 
 *              unsuccessful completion.
 * 
>*/

PUBLIC  ESRO_RetVal
ESRO_sapUnbind(ESRO_SapSel sapSel)
{
    Int sapDesc;			/* SAP descriptor */
    EsroSapCB *indexSap;		/* Index to SAP */
    InvokeCB *invokeCB;			/* Invoke control block */
    InvokeCB *invokeCBNext;		/* Next element of invoke control blk */

    if ( sapSel > ESRO_MAX_SAP_NO ) { /* no need to check min 'cause unsigned */
	EH_problem("ESRO_sapBind: Invalid Sap number\n");
        TM_TRACE((ESRO_modCB, TM_ENTER, 
	         "               Accepted range: %d to %d\n",
		 ESRO_MIN_SAP_NO, ESRO_MAX_SAP_NO));
	return (FAIL); 
    }

    for (indexSap=sapCBMap; indexSap <= ENDOF(sapCBMap); ++indexSap) {
	if ( indexSap->sapSel ==  sapSel && indexSap->state == ACTIVE_STATE ) {
	    /* So, the sap is active */
	    break;
	}
    }

    if ( indexSap > ENDOF(sapCBMap) ) {
	/* So the SAP is not active */
	return (SUCCESS);
    }

    sapDesc = indexSap - sapCBMap;

    TM_TRACE((ESRO_modCB, TM_ENTER,
	    "ESRO_sapUnbind: sapDesc=%d\n",  sapDesc));

    prim.type = LOPQ_SAP_UNBIND;
    prim.un.deActivate.locSapDesc = sapDesc;

    if (lous_putAction(sapCBMap[sapDesc].primQu, &prim) < 0) {
	EH_problem("ESRO_sapUnbind: lous_putAction failed\n");
	return (FAIL);
    }

    /* See if there are any remaining invokations associated with this
     * SAP, issue disconnect requests on them.
     */

    for(invokeCB = QU_FIRST(&sapCBMap[sapDesc].invokeQuHead);
	! QU_EQUAL(invokeCB, &sapCBMap[sapDesc].invokeQuHead);
	invokeCB = invokeCBNext) {

	invokeCBNext = QU_NEXT(invokeCB);
	if (lous_removeInvokeCB(invokeCB) < 0) {
	    EH_problem("ESRO_sapUnbind: lous_removeInvokeCB failed\n");
	}
    }

    /* See if there are any events queued up for this SAP,
     * dump them.
     */

    sapCBMap[sapDesc].sapSel = 0;
    sapCBMap[sapDesc].state = INACTIVE_STATE;

    TM_TRACE((ESRO_modCB, TM_ENTER,
	     "ESRO_sapUnbind: (before ret) sapSel=%d   sapDesc=%d\n", 
	     sapSel, sapDesc));

#ifndef IMQ_SIMU
    if (imqSimuCompileSw) {
      	TM_TRACE((ESRO_modCB, TM_ENTER, 
        	 "**** user_shell compiled <<<WITHOUT IMQ_SIMU>>> *****\n"));
	imqSimuCompileSw = 0;
    }
    IMQ_blockQuRemove(sapCBMap[sapDesc].primQu->actionQu);
    IMQ_blockQuRemove(sapCBMap[sapDesc].primQu->eventQu);

/*    PSQ_primQuRemove((PSQ_PrimQuInfo *)sapCBMap[sapDesc].primQu); */
    close((int)sapCBMap[sapDesc].primQu->actionQu);
    close((int)sapCBMap[sapDesc].primQu->eventQu);
#endif

    return ( SUCCESS );

} /* ESRO_sapUnbind() */


/*<
 * Function:    ESRO_invokeReq
 *
 * Description: ESRO invoke request processing.
 *
 * Arguments:   Invoke ID, SAP descriptor, SAP selector, Transport SAP selector,
 *              Network SAP selector, operation value, encoding type, 
 *              parameter length, parameter data.
 *
 * Returns:     0 on successful completion, a negative error value on 
 *              unsuccessful completion.
 * 
>*/

PUBLIC  ESRO_RetVal
ESRO_invokeReq(ESRO_InvokeId *invokeId,        	 /* OUTgoing */
    	       ESRO_UserInvokeRef userInvokeRef,    /* User invoke ref */
	       ESRO_SapDesc 	locSapDesc,	    /* Local SAP Descriptor */

	       ESRO_SapSel 	remESROSap,	    /* Remote ESRO SAP */
	       T_SapSel *	remTsap,	    /* Remote Transport SAP */
	       N_SapAddr *	remNsap,	    /* Remote Network SAP */
	       ESRO_OperationValue opValue,	    /* Operation value */
	       ESRO_EncodingType   encodingType,    /* Encoding type */
	       Int  parameterLen,		    /* Parameter length */
	       Byte *parameter)			    /* Parameter */
{

    LOPQ_PStatus *pStatus;	/* Primitive Status */

    if (remTsap == NULL  ||  remNsap == NULL) {
	return -1;
    }

    /* Such comparison as 'if (parameterLen > invokePduSize)' is done
	in ESROS engine and provider returns failure code. */

    TM_TRACE((ESRO_modCB, TM_ENTER,
	    "ESRO_invokeReq: sapDesc=%d, parameterLen=%d\n",
	    locSapDesc, parameterLen));

    prim.type = LOPQ_INVOKE_REQ;
    prim.un.invokeReq.userInvokeRef= userInvokeRef;
    prim.un.invokeReq.locSapDesc   = locSapDesc;
    prim.un.invokeReq.remESROSap   = remESROSap;
    prim.un.invokeReq.remTsap      = *remTsap;
    prim.un.invokeReq.remNsap      = *remNsap;
    prim.un.invokeReq.opValue      = opValue;
    prim.un.invokeReq.encodingType = encodingType;
    prim.un.invokeReq.parameterLen = parameterLen;
    OS_copy(prim.un.invokeReq.parameter, parameter, parameterLen);

    if (lous_putAction(sapCBMap[locSapDesc].primQu, &prim) < 0) {
	EH_problem("ESRO_invokeReq: lous_putAction failed\n");
	return -3;
    }

    /* Check the Status: BLOCK and wait */
    /* If the coming event is not a response to invoke request, then invoke
       request fails, and the event is dropped. */

    if (lous_getEvent(sapCBMap[locSapDesc].primQu, &lous_event, TRUE) < 0) {
	EH_problem("ESRO_invokeReq: lous_getEvent failed\n");
	return -4;
    }
 
    pStatus = (LOPQ_PStatus *)&lous_event;

    switch (pStatus->type) {
    case LOPQ_INVOKE_REQ_STAT :
	if ( pStatus->status ) {
	    TM_HEXDUMP(ESRO_modCB, ESRO_TRPRIM, 
		       "invokeReq status",  (Byte *) pStatus, 20);
            TM_TRACE((ESRO_modCB, TM_ENTER,
	    	     "ESRO_invokeReq: status indicates that invoke failed"));
/***            lous_removeInvokeCB(invokeCB); ***/
	    return -5;
	} 
	*invokeId = pStatus->un.invokeReqStat.invokeId;
	break;

    default:
	TM_HEXDUMP(ESRO_modCB, ESRO_TRPRIM, "invokeReq: Bad Status",  
		   (Byte *) pStatus, 20);
	EH_problem("ESRO_invokeReq: WARNING: Bad status is received for "
		   "invoke.request. Some event(s) might be lost\n");
	return -6;
    }

    if ( invokeId == (ESRO_InvokeId *) 0 ) {
	EH_problem("ESRO_invokeReq: Cannot assign invoke ID");
	return  -7;
    }

    TM_TRACE((ESRO_modCB, TM_ENTER,
	    "ESRO_invokeReq: sapDesc=%d, invokeId=%d\n",
	    locSapDesc, *invokeId));

    if (lous_addInvokeCB(locSapDesc, *invokeId) == 0) {
    	TM_TRACE((ESRO_modCB, TM_ENTER,
		 "ESRO_invokeReq: invokeCB entry already active"));
	return  -8;
    }

    return (0);		/* SUCCESS */

} /* ESRO_invokeReq() */



/*<
 * Function:    ESRO_resultReq
 *
 * Description: ESRO RESULT.request processing.
 *
 * Arguments:   Invoke ID, encoding type, parameter length, parameter data.
 *
 * Returns:     0 on successful completion, otherwise a negative error value.
 * 
>*/

PUBLIC  ESRO_RetVal
ESRO_resultReq(ESRO_InvokeId invokeId,			/* Invoke ID */
    	       ESRO_UserInvokeRef userInvokeRef,	/* User invoke ref */
	       ESRO_EncodingType encodingType, 		/* Encoding type */
	       Int parameterLen,			/* Parameter length */
	       Byte *parameter)				/* Parameter */
{
    EsroSapCB *sapCB;		/* ESRO SAP Control Block */

    TM_TRACE((ESRO_modCB, TM_ENTER, 
	     "ESRO_resultReq: invokeId=%d, len=%d\n", 
	     invokeId, parameterLen));

    prim.type = LOPQ_RESULT_REQ;

    prim.un.resultReq.locSapDesc   = 0;  /* NOTYET, I don't think is necessary */
    prim.un.resultReq.invokeId     = invokeId;
    prim.un.resultReq.userInvokeRef= userInvokeRef;
    prim.un.resultReq.encodingType = encodingType;
    prim.un.resultReq.parameterLen = parameterLen;

    if (parameter) {
    	OS_copy(prim.un.resultReq.parameter, parameter, parameterLen);
    }

    if ((sapCB = invokeCBMap[invokeId].sapCB) == 0) {
	EH_problem("ESRO_resultReq: No SAP entry for the invoke id\n"); 
    	TM_TRACE((ESRO_modCB, TM_ENTER, 
		 "                invokeId=%d, sapCB=%lu\n", 
		 invokeId, (unsigned long)sapCB));
	return (-1);	
    }

    if (lous_putAction(sapCB->primQu, &prim) < 0) {
	EH_problem("ESRO_resultReq: lous_putAction failed\n");
	return (-2);	
    }

    return (SUCCESS);

} /* ESRO_resultReq() */


/*<
 * Function:    ESRO_errorReq
 *
 * Description: ESRO ERROR.request processing.
 *
 * Arguments:   Invoke ID, encoding type, parameter length, parameter data.
 *
 * Returns:     0 on successful completion, otherwise a negative error value.
 * 
>*/

PUBLIC  ESRO_RetVal
ESRO_errorReq(ESRO_InvokeId invokeId,			/* Invoke ID */
    	      ESRO_UserInvokeRef userInvokeRef,		/* User invoke ref */
	      ESRO_EncodingType encodingType,		/* Encoding type */
	      ESRO_ErrorValue errorValue,		/* Error value */
	       Int parameterLen,			/* Parameter length */
	       Byte *parameter)				/* Parameter */
{
    EsroSapCB *sapCB;			/* ESRO SAP Control Block */

    TM_TRACE((ESRO_modCB, TM_ENTER, 
	"ESRO_errorReq:  invokeId=%d, len=%d\n",
	invokeId, parameterLen));

    prim.type = LOPQ_ERROR_REQ;

    prim.un.errorReq.locSapDesc   = 0;  /* NOTYET, I don't think is necessary */
    prim.un.errorReq.invokeId     = invokeId;
    prim.un.errorReq.userInvokeRef= userInvokeRef;
    prim.un.errorReq.encodingType = encodingType;
    prim.un.errorReq.errorValue   = errorValue;
    prim.un.errorReq.parameterLen = parameterLen;

    if (parameter) {
    	OS_copy(prim.un.errorReq.parameter, parameter, parameterLen);
    }

    if ((sapCB = invokeCBMap[invokeId].sapCB) == 0) {
	EH_problem("ESRO_errorReq: No SAP entry for the invoke id\n"); 
    	TM_TRACE((ESRO_modCB, TM_ENTER, 
		 "                invokeId=%d, sapCB=%lu\n", 
	 	 invokeId, (unsigned long)sapCB));
	return (-1);	
    }

    if (lous_putAction(sapCB->primQu, &prim) < 0) {
	EH_problem("ESRO_errorReq: lous_putAction failed\n");
	return (-2);
    }

    return (SUCCESS);

} /* ESRO_errorReq() */


/*<
 * Function:    ESRO_getEvent
 * 
 * Description: Get event. (Currently multiple Saps are not supported.)
 *
 * Arguments:   SAP descriptor, event, wait.
 *
 * Returns:     0 on successful completion, otherwise a negative error value.
 * 
>*/

PUBLIC  ESRO_RetVal
ESRO_getEvent(ESRO_SapDesc sapDesc, 		/* ESRO SAP Descriptor */
	      ESRO_Event *event, 		/* ESROS Event */
	      Bool wait)			/* Blocking/non-blocking */
{
    int retVal = 0;

    if ( sapDesc < 0 ) {
	EH_problem("ESRO_getEvent: Invalid SAP descriptor");
	return (-1);
    }

#ifdef IMQ_SIMU
    if (imqSimuCompileSw) {
    	TM_TRACE((ESRO_modCB, TM_ENTER, 
        	 "**** user_shell compiled <<<WITH IMQ_SIMU>>> *****\n"));
	imqSimuCompileSw = 0;
    }
    G_heartBeat();
#else
    if (imqSimuCompileSw) {
    	TM_TRACE((ESRO_modCB, TM_ENTER, 
        	 "**** user_shell compiled <<<WITHOUT IMQ_SIMU>>> *****\n"));
	imqSimuCompileSw = 0;
    }
#endif

    if ( (retVal = lous_getEvent(sapCBMap[sapDesc].primQu, &lous_event, wait))
	 < 0 ) {
	if ( retVal == -2 ) {
	    return -2;
	}
	if ( errno == EINTR ) {
    	    TM_TRACE((ESRO_modCB, TM_ENTER, 
        	     "ESRO_getEvent: lous_getEvent called, then EINTR\n"));
	    return ( -3 );
	}
	return ( -4 );
    }

    TM_TRACE((ESRO_modCB, TM_ENTER, "ESRO_getEvent:  sapDesc=%d\n", sapDesc));

    switch ( lous_event.type ) { 
    case LOPQ_INVOKE_IND :
        {
	    LOPQ_InvokeInd * invokeInd;

	    invokeInd = &(lous_event.un.invokeInd);

	    TM_TRACE((ESRO_modCB, TM_ENTER, 
		     "ESRO_getEvent: InvokeInd: sapDesc=0x%x invokeId=%d\n", 
		     invokeInd->locSapDesc, invokeInd->invokeId));

	    if (!lous_addInvokeCB(invokeInd->locSapDesc, invokeInd->invokeId)) {
	    	TM_TRACE((ESRO_modCB, TM_ENTER, 
	    		"ESRO_getEvent: Invoke indication: no invokeCB"));
	    	return ( -5 );
	    }

	    event->type         = lous_event.type;
	    event->un.invokeInd = *invokeInd;

	    return ( SUCCESS );
    	}

    case LOPQ_RESULT_IND :
    case LOPQ_ERROR_IND :
    	{
	    LOPQ_ResultInd * resultInd;
    	    EsroSapCB *sapCB;

	    resultInd = &(lous_event.un.resultInd);

	    if ((sapCB = invokeCBMap[resultInd->invokeId].sapCB) == 0) {
	      	EH_problem("ESRO_getEvent: Invalid sap CB for result/error "
			   "indication\n"); 
    		TM_TRACE((ESRO_modCB, TM_ENTER, 
			 "               sapCB=%lu\n",
			 (unsigned long)sapCB));
		return (-6);	
	    }

	    TM_TRACE((ESRO_modCB, TM_ENTER, 
		     "ESRO_getEvent: ResultInd: sapDesc=0x%x  invokeId=%d\n", 
		      invokeCBMap[resultInd->invokeId].sapCB->sapDesc,
		      resultInd->invokeId));
	
	    event->type = lous_event.type;
	    event->un.resultInd = *resultInd;

	    if (lous_removeInvokeCB(&invokeCBMap[resultInd->invokeId]) == FAIL) {
	      	EH_problem("ESRO_getEvent: RESULT/ERROR.indication: "
			   "lous_removeInvokeCB failed\n"); 
		return -7;
	    }

	    return ( 0 );	/* SUCCESS */
    	}

    case LOPQ_RESULT_CNF :
    case LOPQ_ERROR_CNF :
    	{
	    LOPQ_ResultCnf * resultCnf;
            EsroSapCB *sapCB;

	    resultCnf = &(lous_event.un.resultCnf);

	    if ((sapCB = invokeCBMap[resultCnf->invokeId].sapCB) == 0) {
	      	EH_problem("ESRO_getEvent: Invalid sap CB for "
			   "result/error confirmation\n"); 
    		TM_TRACE((ESRO_modCB, TM_ENTER, 
			 "               sapCB=%lu\n",
			 (unsigned long)sapCB));
		return (-8);	
	    }

	    TM_TRACE((ESRO_modCB, TM_ENTER, 
		     "ESRO_getEvent: ResultCnf: sapDesc=0x%x invokeId=%d\n", 
	    	     invokeCBMap[resultCnf->invokeId].sapCB->sapDesc,
		     resultCnf->invokeId));

	    event->type         = lous_event.type;
	    event->un.resultCnf = *resultCnf;

	    if (lous_removeInvokeCB(&invokeCBMap[resultCnf->invokeId]) == FAIL) {
	      	EH_problem("ESRO_getEvent: RESULT/ERROR.confirm: "
			   "lous_removeInvokeCB failed\n"); 
		return (-9);
	    }

	    return ( 0 );	/* SUCCESS */
    	}

    case LOPQ_FAILURE_IND :
        {
	    LOPQ_FailureInd *failureInd;
            EsroSapCB *sapCB;

	    failureInd = &(lous_event.un.failureInd);

	    if ((sapCB = invokeCBMap[failureInd->invokeId].sapCB) == 0) {
	      	EH_problem("ESRO_getEvent: Invalid sap CB for "
			   "failureIndication\n"); 
    		TM_TRACE((ESRO_modCB, TM_ENTER, 
			 "               sapCB=%lu\n", (unsigned long)sapCB));
		return (-10);	
	    }

	    TM_TRACE((ESRO_modCB, TM_ENTER, 
		     "ESRO_getEvent: FailureInd: sapDesc=0x%x\n",
	    	     invokeCBMap[failureInd->invokeId].sapCB->sapDesc));

	    event->type          = lous_event.type;
	    event->un.failureInd = *failureInd;

	    if (lous_removeInvokeCB(&invokeCBMap[failureInd->invokeId]) == FAIL) {
	      	EH_problem("ESRO_getEvent: FAILURE.indication: "
			   "lous_removeInvokeCB failed\n"); 
		return (-11);
	    }
	
	    return ( 0 );	/* SUCCESS */
	}

    default:
	EH_problem("ESRO_getEvent: Invalid event");

    	TM_HEXDUMP(ESRO_modCB, ESRO_TRPRIM, 
	       	   "getEvent: Bad Event", (Byte *)  &lous_event, 20);

	event->type = -1;
	return ( -12 );
    }

} /* ESRO_getEvent() */


/*<
 * Function:    lous_getEvent
 *
 * Description: User shell get event.
 *
 * Arguments:   Primitive queue, event, waiting time.
 *
 * Returns:     0 on successful completion, otherwise a negative error value.
 * 
>*/

Int
lous_getEvent(USQ_PrimQuInfo *primQu, 		/* Primitive queue */
	      LOPQ_Event *event, 		/* ESROS Event */
	      Bool wait)			/* blocking/non-blocking */
{
    Int size = 0;

#ifdef TM_ENABLED
    static char taskName[OS_MAX_FILENAME_LEN+50] = "lous get event";
#endif

    if (primQu == 0) {
	return (-1);
    }

#ifdef IMQ_SIMU
    if (imqSimuCompileSw) {
    	TM_TRACE((ESRO_modCB, TM_ENTER, 
        	 "**** user_shell compiled <<<WITH IMQ_SIMU>>> *****\n"));
	imqSimuCompileSw = 0;
    }
    wait = 0;
#else
    if (imqSimuCompileSw) {
    	TM_TRACE((ESRO_modCB, TM_ENTER, 
        	 "**** user_shell compiled <<<WITHOUT IMQ_SIMU>>> *****\n"));
	imqSimuCompileSw = 0;
    }
#endif

    if (!wait) {
	if ((size = USQ_getEvent(primQu, (Ptr)event, sizeof(*event))) > 0) {

	    TM_TRACE((ESRO_modCB, TM_ENTER, 
		     "lous_getEvent:  event=%d  action=%d   event->type=%d "
		     "size=%ld\n",
		     primQu->eventQu, primQu->actionQu, event->type, 
		     (long int) size));

	    return ( size );
	} else if (errno == EINTR) {
    	    TM_TRACE((ESRO_modCB, TM_ENTER, 
        	     "lous_getEvent: USQ_getEvent called, then EINTR\n"));
	    return ( size );

	} else {
	    return ( size );

	}
    } else {
#ifdef TM_ENABLED
        strcpy (taskName, "lous get event ");
 	SCH_submit((Void *)esro_storeEvent, (Ptr)&primQu, 
		   (SCH_Event)primQu->eventQu, 
		   (String) strcat(taskName, TM_here()));
#else
 	SCH_submit((Void *)esro_storeEvent, (Ptr)&primQu, 
		   (SCH_Event)primQu->eventQu);
#endif
	
	G_env.hardReset = G_env.softReset = FALSE;

	while (!G_env.softReset) {
	    G_heartBeat();

	    if (esro_eventAvailable(primQu)) {
	    	if ((size = USQ_getEvent(primQu, (Ptr)event, sizeof(*event))) 
		     > 0) {
	    	    return ( size );
		} else {
		    if (size == -2) {
			return -2;
		    }
	    	    TM_TRACE((ESRO_modCB, TM_ENTER, 
		     	     "lous_getEvent:  WARNING: Event in queue, "
			     "but USQ_getEvent failed\n"
			     "                         eventQu=%d  actionQu=%d\n",
		     	     primQu->eventQu, primQu->actionQu));
		}
    	    }
    	}
    }
    return (size);

} /* lous_getEvent() */


/*<
 * Function:    lous_getSchEvent
 *
 * Description: get scheduler event.
 *
 * Arguments:   SAP Descriptor.
 *
 * Returns:     Scheduler event
 * 
>*/

SCH_Event *
eSRO_getSchEvent(ESRO_SapDesc sapDesc)		/* ESRO SAP Descriptor */
{
    return ((SCH_Event *)&(sapCBMap[sapDesc].primQu->eventQu));
}


/*<
 * Function:    lous_storeEvent
 *
 * Description: Store event
 *
 * Arguments:   Queue descriptor
 *
 * Returns:     0 if successful, -1 otherwise.
 * 
>*/

Int
esro_storeEvent(USQ_PrimQuInfo *primQu)		/* Primitive queue */
{
    lous_EventQuInfo *eventQuInfo;

    if (!(eventQuInfo = (lous_EventQuInfo *)SEQ_elemObtain(eventQuInfoPool))) {
	EH_problem("esro_storeEvent: SEQ_elemObtain failed");
	return (FAIL);
    }

    eventQuInfo->quInfo = primQu;
    QU_INIT(eventQuInfo);
    QU_INSERT (&eventQuInfoSeq, eventQuInfo);

    return (SUCCESS);	
}


/*<
 * Function:    esro_eventAvailable
 *
 * Description: Check if event available on the given queue.
 *
 * Arguments:   Queue descriptor.
 *
 * Returns:     0 if successful, -1 otherwise.
 * 
>*/

Int
esro_eventAvailable(USQ_PrimQuInfo *primQu)	/* Primitive queue */
{
    lous_EventQuInfo *eventQuInfo;
    
    for (eventQuInfo = eventQuInfoSeq.first;  
	 eventQuInfo != (lous_EventQuInfo *)&eventQuInfoSeq;
	 eventQuInfo = eventQuInfo->next) {
	if (primQu == eventQuInfo->quInfo) {
	    return (SUCCESS);
	}
    }

    return (FAIL);	
}

