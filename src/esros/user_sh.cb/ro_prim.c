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
 *   ESRO_CB_init (void)
 *   ESRO_CB_sapBind(ESRO_SapDesc *sapDesc,  OUTgoing 
 *   		     ESRO_SapSel sapSel, ESRO_FunctionalUnit functionalUnit,
 *	             int (*invokeInd) (ESRO_SapDesc		locSapDesc,
 *			               ESRO_SapSel 	   	remESROSap,
 *			               T_SapSel		*remTsap,
 *			               N_SapAddr	   	*remNsap,
 *				       ESRO_InvokeId     	invokeId,
 *			               ESRO_OperationValue 	opValue,
 *			               ESRO_EncodingType   	encodingType,
 *			               DU_View	   	parameter),
 *	             int (*resultInd) (ESRO_InvokeId   	invokeId,
 *				       ESRO_UserInvokeRef userInvokeRef,
 *			               ESRO_EncodingType 	encodingType,
 *			               DU_View		parameter),
 *	             int (*errorInd)  (ESRO_InvokeId   	invokeId,
 *				       ESRO_UserInvokeRef userInvokeRef,
 *			               ESRO_EncodingType	encodingType,
 *			               ESRO_ErrorValue	errorValue,
 *			               DU_View 		parameter),
 *	             int (*resultCnf) (ESRO_InvokeId     invokeId,
 *				       ESRO_UserInvokeRef userInvokeRef),
 *	             int (*errorCnf)  (ESRO_InvokeId     invokeId,
 *				       ESRO_UserInvokeRef userInvokeRef),
 *	             int (*failureInd)(ESRO_InvokeId     invokeId,
 *				       ESRO_UserInvokeRef userInvokeRef,
 *				       ESRO_FailureValue failureValue))
 *
 *   ESRO_CB_invokeReq(ESRO_InvokeId *invokeId,        	  OUTgoing 
 *                     ESRO_UserIvokeRef userInvokeRef,
 *                     ESRO_SapDesc locSapDesc,
 *   	               ESRO_SapSel remESROSap,
 *   	               T_SapSel *remTsap,
 *   	               N_SapAddr *remNsap,
 *   	               ESRO_OperationValue opValue,
 *   	               DU_View parameter)
 *
 *   ESRO_CB_invokeReq(ESRO_InvokeId       *invokeId,  	  OUTgoing 
 *                     ESRO_UserIvokeRef userInvokeRef,
 *   	               ESRO_SapDesc        locSapDesc,
 *   	               ESRO_SapSel         remESROSap,
 *   	               T_SapSel            *remTsap,
 *   	               N_SapAddr           *remNsap,
 *   	               ESRO_OperationValue opValue,
 *   	               ESRO_EncodingType   encodingType,
 *   	               DU_View             parameter)
 *   ESRO_CB_errorReq(ESRO_InvokeId     invokeId,
 *                    ESRO_UserIvokeRef userInvokeRef,
 *   	              ESRO_EncodingType encodingType,
 *   	              DU_View           parameter)
 *   esro_cb_getEvent(ESRO_SapDesc sapDesc)
 *   lous_cb_getEvent(USQ_PrimQuInfo *primQu, LOPQ_Event *event)
 *   lous_addUSInvoke();
 *   lous_removeUSInvokeId();
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: ro_prim.c,v 1.1.1.1 2002/10/08 18:57:32 mohsen Exp $";
#endif /*}*/

#include  "oe.h"	/* Operating Environment */
#include  "estd.h"	/* Extended standard header */
#include  "eh.h"	/* Exception Handler */
#include  "tm.h"	/* Trace Module */
#include  "du.h"	/* Data Unit module */
#include "usq.h"	/* User Shell Queue */
#include "psq.h"	/* Provider Shell Queue */
#include "seq.h"	/* Sequence module */
#include "esro_sh.h"	/* ESRO Shell */
#include "sch.h"	/* Scheduler */
#include "target.h"	/* Target environment */
#include "esro_cb.h"	/* ESROS with call-back API */
#include "local.h"	/* Local definitions */

#ifdef TM_ENABLED
#define DU_MALLOC TM_BIT10		/* Data Unit module malloc tracing */
extern TM_ModuleCB *DU_modCB;		/* Data Unit module trace handle */

LOCAL  TM_ModuleCB *ESRO_modCB;		/* ESROS module trace handle */
#endif

STATIC  DU_View lous_cpDataToDU(unsigned char *buffer, int length);

STATIC LOPQ_Primitive prim;		/* ESROS Primitive */
STATIC LOPQ_Event lous_event;		/* ESROS event */

InvokeCB invokeCBMap[LOPQ_INVOKE_CBS];	/* Invoke control block */
EsroSapCB sapCBMap[LOPQ_SAPS];		/* SAP control block */

UTOPInvokeIdMap uToPInvokeIdMap[LOPQ_INVOKE_CBS];

#ifndef MSDOS
extern int errno;
#endif

typedef struct lous_EventQuInfo {		/* Event queue node */
    struct lous_EventQuInfo *next;
    struct lous_EventQuInfo *prev;
    USQ_PrimQuInfo *quInfo;
} lous_EventQuInfo;

typedef struct lous_EventQuInfoSeq {		/* Event queue head */
    struct lous_EventQuInfo *first;
    struct lous_EventQuInfo *last;
} lous_EventQuInfoSeq;

USQ_PrimQuInfo *SUS_primQu;
SCH_Event *schEvent;

STATIC lous_EventQuInfoSeq eventQuInfoSeq;
STATIC SEQ_PoolDesc eventQuInfoPool;

extern esro_storeEvent(USQ_PrimQuInfo *primQu);
extern esro_eventAvailable(USQ_PrimQuInfo *primQu);
extern Int lous_addUSInvoke();
extern Void lous_removeUSInvokeId();

Int lous_cb_getEvent (USQ_PrimQuInfo *primQu, LOPQ_Event *event);
PUBLIC ESRO_RetVal esro_cb_getEvent(ESRO_SapDesc *sapDesc);
SCH_Event *ESRO_CB_getSchEvent(ESRO_SapDesc sapDesc);
Int lous_usInvokeId(ESRO_InvokeId invokeId);


/*<
 * Function:    lous_putAction  (Limited Operation User Shell, Put Action)
 *
 * Description: Put action.
 *
 * Arguments:   Primitive queue, primitive.
 *
 * Returns:     None.
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
lous_addInvokeCB(ESRO_SapDesc sapDesc, 		/* ESRO SAP Descriptor */
		 ESRO_InvokeId invokeId)	/* ESRO Invoke ID */
{
    InvokeCB *invokeCB;
    EsroSapCB *sapCB;

    if ( invokeId < 0 ) {
	EH_problem("lous_addInvokeCB: Invalid invoke Id");
        TM_TRACE((ESRO_modCB, TM_ENTER, 
	 	 "lous_addInvokeCB: Invalid invokeId=%d\n", invokeId));
	return ( (InvokeCB *)0 );
    }
    if ( sapDesc < 0 ) {
	EH_problem("lous_addInvokeCB: Invalid SAP descriptor");
        TM_TRACE((ESRO_modCB, TM_ENTER, 
	 	 "lous_addInvokeCB: Invalid sapDesc=%d\n", sapDesc));
	return ( (InvokeCB *)0 );
    }

    invokeCB = &invokeCBMap[invokeId];
    sapCB    = &sapCBMap[sapDesc];

    if ( invokeCB->state != INACTIVE_STATE ) {
	EH_problem("lous_addInvokeCB: The invokeCB already active");
	return ( (InvokeCB *)0 );
    }
    invokeCB->state = ACTIVE_STATE;

    /* Associate the Tsap to the Tcep */
    invokeCB->sapCB = sapCB;

    /* If the Data Channels were to be created on a per 
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
    
    return (SUCCESS);
}


/*<
 * Function:    ESRO_CB_init
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
ESRO_CB_init (char *pConfigFile)
{
    InvokeCB *indexInvokeCB;
    EsroSapCB *indexSap;

    static Bool virgin = TRUE;
    
    if ( !virgin ) {
        EH_problem("WARNING: ESRO_CB_init() is called "
		   "more than once (harmless)\n");
	return (SUCCESS);
    }
    virgin = FALSE;

#ifdef TM_ENABLED
    TM_INIT();

    if ( ! (ESRO_modCB = TM_open("ESRO_") )) {
	EH_problem("ESRO_CB_init:      Error in TM_open ESRO_");
	return ( FAIL );
    }
    TM_TRACE((ESRO_modCB, TM_ENTER, "ESRO_CB_init(CB):  ESRO_ opened\n"));
#endif

    if (SCH_init(MAX_SCH_TASKS) ==  SCH_NOT_VIRGIN) { 
    	TM_TRACE((ESRO_modCB, TM_ENTER, 
		 "WARNING: ESROS application didn't initialize scheduler.\n"));
    }

    USQ_init();   	/* USQ_ initialize IMQ_ */

    for (indexInvokeCB=invokeCBMap; 
	 indexInvokeCB <= ENDOF(invokeCBMap); 
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

    {
    	UTOPInvokeIdMap *idMap, *lastId = uToPInvokeIdMap + LOPQ_INVOKE_CBS;

    	for (idMap = uToPInvokeIdMap; idMap < lastId; idMap++) {
	    idMap->state    = INACTIVE_STATE;
	    idMap->invokeId = -1;
    	}
    }

    return ( SUCCESS );

} /* ESRO_CB_init() */
 


/*<
 * Function:    ESRO_CB_sapBind
 *
 * Description: ESRO SAP bind.
 *
 * Arguments:   SAP descriptor (outgoing argument), SAP selector,
 *		functional unit (2-way or 3-way handshake).
 *
 * Returns:     0 on successful completion, a negative error value on 
 *              unsuccessful completion.
 * 
>*/

PUBLIC ESRO_RetVal
ESRO_CB_sapBind(ESRO_SapDesc *sapDesc, /* OUTgoing */
	        ESRO_SapSel sapSel,
	        ESRO_FunctionalUnit functionalUnit,
	        int (*invokeInd) (ESRO_SapDesc		locSapDesc,
			          ESRO_SapSel 	   	remESROSap,
			          T_SapSel		*remTsap,
			          N_SapAddr	   	*remNsap,
				  ESRO_InvokeId     	invokeId,
			          ESRO_OperationValue 	opValue,
			          ESRO_EncodingType   	encodingType,
			          DU_View	   	parameter),
	        int (*resultInd) (ESRO_InvokeId   	invokeId,
				  ESRO_UserInvokeRef   	userInvokeRef,
			          ESRO_EncodingType 	encodingType,
			          DU_View		parameter),
	        int (*errorInd)  (ESRO_InvokeId   	invokeId,
				  ESRO_UserInvokeRef   	userInvokeRef,
			          ESRO_EncodingType	encodingType,
			          ESRO_ErrorValue	errorValue,
			          DU_View 		parameter),
	        int (*resultCnf) (ESRO_InvokeId     invokeId,
				  ESRO_UserInvokeRef   	userInvokeRef),
	        int (*errorCnf)  (ESRO_InvokeId     invokeId,
				  ESRO_UserInvokeRef   	userInvokeRef),
	        int (*failureInd)(ESRO_InvokeId     invokeId,
				  ESRO_UserInvokeRef   	userInvokeRef,
				  ESRO_FailureValue failureValue))
{
    LOPQ_PStatus   *pStatus;	/* Primitive Status */
    USQ_PrimQuInfo *primQu;	/* Primitive Queue */

#ifdef TM_ENABLED
    static char taskName[100] = "esro_cb_getEvent (by sapBind) (CB)";
#endif

    TM_TRACE((ESRO_modCB, TM_ENTER, "ESRO_CB_sapBind:   Entered\n"));

    if ( sapSel > ESRO_MAX_SAP_NO ) { /* no need to check min because unsigned */
	char buf[256];
	sprintf(buf, "EROP_CB_sapBind: Invalid Sap number: "
		"Accepted range: %d to %d\n",
		ESRO_MIN_SAP_NO, ESRO_MAX_SAP_NO);
	EH_problem(buf);
	return  -1; 
    }

    if (functionalUnit != ESRO_3Way  &&  functionalUnit != ESRO_2Way) {
	char buf[256];
 	sprintf(buf, "ESRO_sapBind: Invalid functional unit: "
		" Given value: %d,   Accepted values: %d and %d\n", 
		functionalUnit, ESRO_3Way, ESRO_2Way);
	EH_problem(buf);
	return -2;
    }

    if ( (SUS_primQu = primQu = USQ_primQuCreate()) == NULL) {
 	EH_problem("ESRO_sapBind: USQ_primQuCreate failed\n");
	return -3;
    }

    prim.type = LOPQ_SAP_BIND; 	
    prim.un.activate.locSapSel = sapSel;
    prim.un.activate.functionalUnit = functionalUnit;

    /* Queues Keys can be derrived from chnlName */
    strcpy(prim.un.activate.chnlName, primQu->chnlName);

    if (USQ_putAction(primQu, (Ptr) &prim, sizeof(prim)) == FAIL) {
        TM_TRACE((ESRO_modCB, TM_ENTER, 
	        "ESRO_sapBind: USQ_putAction failed\n"));
	return -4; 
    }

    /* Block and wait for bind */
    while (lous_cb_getEvent(primQu, &lous_event) < 0) {
    }

    pStatus = (LOPQ_PStatus *) &lous_event;

    switch (pStatus->type) {
    case LOPQ_SAP_BIND_STAT:
    	if ( pStatus->status != SUCCESS) {
    	    EH_problem("ESRO_CB_sapBind: status says sapBind failed");
	    TM_HEXDUMP(ESRO_modCB, ESRO_TRPRIM, "sapBind status", 
                       (Byte *) pStatus, 20);
	    return (pStatus->status);
	} 
	*sapDesc = pStatus->un.activateStat.locSapDesc;
	break;

    default: 
    	EH_problem("ESRO_CB_sapBind: invalid event type");
	TM_HEXDUMP(ESRO_modCB, ESRO_TRPRIM, "sapBind: bad type", 
                       (Byte *) pStatus, 20);
	return (-5);
    }

    if ( *sapDesc >= DIMOF(sapCBMap)) {
	EH_problem("ESRO_CB_sapBind: sapCBMap overflow");
	return ( -6 );
    }

    sapCBMap[*sapDesc].sapSel  = sapSel;
    sapCBMap[*sapDesc].functionalUnit  = functionalUnit;
    sapCBMap[*sapDesc].sapDesc = *sapDesc;
    sapCBMap[*sapDesc].primQu  = primQu;
    sapCBMap[*sapDesc].state   = ACTIVE_STATE;

    sapCBMap[*sapDesc].invokeInd  = invokeInd;
    sapCBMap[*sapDesc].resultInd  = resultInd;
    sapCBMap[*sapDesc].errorInd   = errorInd;
    sapCBMap[*sapDesc].resultCnf  = resultCnf;
    sapCBMap[*sapDesc].errorCnf   = errorCnf;
    sapCBMap[*sapDesc].failureInd = failureInd;

    TM_TRACE((ESRO_modCB, TM_ENTER,
	"ESRO_CB_sapBind: (before ret) sapSel=%d   sapDesc=%d    funcUnit=%d\n",
	 sapSel, *sapDesc, functionalUnit));

#ifdef TM_ENABLED
    strcpy(taskName, "esro_cb_getEvent (by sapBind) (CB)");
    SCH_submit ((Void *)esro_cb_getEvent, (Ptr) &sapCBMap[*sapDesc].sapDesc,
		(SCH_Event) sapCBMap[*sapDesc].primQu->eventQu, 
		(String) strcat(taskName, TM_here()));
#else
    SCH_submit ((Void *)esro_cb_getEvent, (Ptr) &sapCBMap[*sapDesc].sapDesc,
		(SCH_Event) sapCBMap[*sapDesc].primQu->eventQu);
#endif

    return ( 0 );

} /* ESRO_CB_sapBind() */


/*<
 * Function:    ESRO_CB_sapUnbind
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
ESRO_CB_sapUnbind(ESRO_SapSel sapSel)
{
    Int sapDesc;
    EsroSapCB *indexSap;

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
	    "ESRO_CB_sapUnbind: sapSel=%d    sapDesc=%d\n", sapSel, sapDesc));

    prim.type = LOPQ_SAP_UNBIND;
    prim.un.deActivate.locSapDesc = sapDesc;

    if (lous_putAction(sapCBMap[sapDesc].primQu, &prim) == FAIL) {
	EH_problem("ESRO_sapUnbind: lous_putAction failed\n");
	return (FAIL);
    }

    /* See if there are any remaining invokations associated with this
     * SAP, issue disconnect requests on them.
     */

    /* See if there are any events queued up for this SAP,
     * dump them.
     */

    sapCBMap[sapDesc].sapSel = 0;
    sapCBMap[sapDesc].state = INACTIVE_STATE;

    TM_TRACE((ESRO_modCB, TM_ENTER,
	"ESRO_CB_sapUnbind: (before ret) sapSel=%d   sapDesc=%d    funcUnit=%d\n",
	 sapSel, sapDesc, sapCBMap[sapDesc].functionalUnit));

    /* If user calls sapUnbind while it has operations in progress,
       dump all its scheduled tasks */

    SCH_allTaskDelete(sapCBMap[sapDesc].primQu->eventQu, NULL, NULL);
    SCH_allTaskDelete(SCH_PSEUDO_EVENT, (Void *)esro_cb_getEvent, 
		      sapCBMap[sapDesc].primQu);

    IMQ_blockQuRemove(sapCBMap[sapDesc].primQu->actionQu);
    IMQ_blockQuRemove(sapCBMap[sapDesc].primQu->eventQu);

/*    PSQ_primQuRemove((PSQ_PrimQuInfo *)sapCBMap[sapDesc].primQu); */
    close(sapCBMap[sapDesc].primQu->actionQu);
    close(sapCBMap[sapDesc].primQu->eventQu);

    return ( SUCCESS );

} /* ESRO_CB_sapUnbind() */


/*<
 * Function:    ESRO_CB_invokeReq
 *
 * Description: ESRO invoke request processing.
 *
 * Arguments:   Invoke ID, SAP descriptor, SAP selector, Transport SAP selector,
 *              Network SAP selector, operation value, encoding type, data unit.
 *
 * Returns:     0 on successful completion, a negative error value on 
 *              unsuccessful completion.
 * 
>*/

PUBLIC  ESRO_RetVal
ESRO_CB_invokeReq(ESRO_InvokeId *invokeId,        	 /* OUTgoing */
	          ESRO_UserInvokeRef userInvokeRef,
	          ESRO_SapDesc locSapDesc,
	          ESRO_SapSel remESROSap,
	          T_SapSel *remTsap,
	          N_SapAddr *remNsap,
	          ESRO_OperationValue opValue,
	          ESRO_EncodingType encodingType,
     	          DU_View	    parameter)
{
    prim.type = LOPQ_INVOKE_REQ;
    prim.un.invokeReq.userInvokeRef= userInvokeRef;
    prim.un.invokeReq.locSapDesc   = locSapDesc;
    prim.un.invokeReq.remESROSap   = remESROSap;
    prim.un.invokeReq.remTsap      = *remTsap;
    prim.un.invokeReq.remNsap      = *remNsap;
    prim.un.invokeReq.opValue      = opValue;
    prim.un.invokeReq.encodingType = encodingType;
    prim.un.invokeReq.parameterLen = DU_size(parameter);
    OS_copy(prim.un.invokeReq.parameter, 
	    DU_data(parameter), DU_size(parameter));

    if((*invokeId = prim.un.invokeReq.userShellInvokeId = lous_addUSInvoke()) 
       == -1) {
	EH_problem("ESRO_CB_invokeReq: lous_addUSInvoke failed\n"); 
	return (FAIL);
    }

    TM_TRACE((ESRO_modCB, TM_ENTER, "ESRO_CB_invokeReq: usInvokeId=%d\n", 
	     *invokeId));

    if (lous_putAction(sapCBMap[locSapDesc].primQu, &prim) < 0) {
	EH_problem("ESRO_CB_invokeReq: lous_putAction failed\n"); 
        lous_removeUSInvokeId(*invokeId);
	return (FAIL);
    }
    
    return (SUCCESS);

} /* ESRO_invokeReq() */



/*<
 * Function:    ESRO_CB_resultReq
 *
 * Description: ESRO RESULT.request processing.
 *
 * Arguments:   Invoke ID, encoding type, data unit.
 *
 * Returns:     0 on successful completion, otherwise a negative error value.
 * 
>*/

PUBLIC  ESRO_RetVal
ESRO_CB_resultReq(ESRO_InvokeId usInvokeId,
	          ESRO_UserInvokeRef userInvokeRef,
	          ESRO_EncodingType encodingType, 
 	          DU_View	    parameter)
{
    EsroSapCB *sapCB;
    ESRO_InvokeId psInvokeId; 

    if (usInvokeId < 0  ||  encodingType > 5 ) {
	EH_problem("ESRO_CB_resultReq: invalid encoding type (or invoke id)\n"); 
	return FAIL;
    }

    psInvokeId = uToPInvokeIdMap[usInvokeId].invokeId;

    TM_TRACE((ESRO_modCB, TM_ENTER, 
	     "ESRO_CB_resultReq: usInvokeId=%d  psInvokeId=%d\n", 
	     usInvokeId, psInvokeId));

    prim.type = LOPQ_RESULT_REQ;

    prim.un.resultReq.locSapDesc   = 0; /* NOTYET, I don't think is necessary */
    prim.un.resultReq.invokeId     = psInvokeId;
    prim.un.resultReq.userInvokeRef= userInvokeRef;
    prim.un.resultReq.encodingType = encodingType;
    prim.un.resultReq.parameterLen = DU_size(parameter);

    if (parameter) {
     	OS_copy(prim.un.resultReq.parameter, 
		DU_data(parameter), 
		DU_size(parameter));
    }

    if ((sapCB = invokeCBMap[psInvokeId].sapCB) == 0) {
	EH_problem("ESRO_CB_resultReq: No SAP control block for the "
		   "given invoke id\n");
	return FAIL;
    }

    if (lous_putAction(sapCB->primQu, &prim) < 0) {
	EH_problem("ESRO_CB_resultReq: lous_putAction failed\n");
	return (FAIL);
    }

    return ( SUCCESS ); 	/* change */

} /* ESRO_CB_resultReq() */


/*<
 * Function:    ESRO_CB_errorReq
 *
 * Description: ESRO ERROR.request processing.
 *
 * Arguments:   Invoke ID, encoding type, error value, and data unit.
 *
 * Returns:     0 on successful completion, otherwise a negative error value.
 * 
>*/

PUBLIC  ESRO_RetVal
ESRO_CB_errorReq(ESRO_InvokeId     usInvokeId,
	         ESRO_UserInvokeRef userInvokeRef,
	         ESRO_EncodingType encodingType,
 	         ESRO_ErrorValue   errorValue,
	         DU_View	   parameter)
{
    EsroSapCB *sapCB;
    ESRO_InvokeId psInvokeId; 

    if (usInvokeId < 0  ||  encodingType > 5) {
	return FAIL;
    }

    psInvokeId = uToPInvokeIdMap[usInvokeId].invokeId;

    TM_TRACE((ESRO_modCB, TM_ENTER, 
	"ESRO_CB_errorReq:  usInvokeId=%d   psInvokeId=\n",
	usInvokeId, psInvokeId));

    prim.type = LOPQ_ERROR_REQ;

    prim.un.errorReq.locSapDesc   = 0;  /* NOTYET, I don't think is necessary */
    prim.un.errorReq.invokeId     = psInvokeId;
    prim.un.errorReq.userInvokeRef= userInvokeRef;
    prim.un.errorReq.encodingType = encodingType;
    prim.un.errorReq.errorValue   = errorValue;
    prim.un.errorReq.parameterLen = DU_size(parameter);

    if (parameter) {
    	OS_copy(prim.un.errorReq.parameter, 
		DU_data(parameter), 
		DU_size(parameter));
    }

    if ((sapCB = invokeCBMap[psInvokeId].sapCB) == 0) {
	EH_problem("ESRO_CB_resultReq: No SAP control block for "
		   "the given invoke id\n");
	return FAIL;
    }

    if (lous_putAction(sapCB->primQu, &prim) < 0) {
	EH_problem("ESRO_CB_resultReq: lous_putAction failed.");
	return (FAIL);
    }

    return ( SUCCESS );

} /* ESRO_CB_errorReq() */


/*<
 * Function:    esro_cb_getEvent
 * 
 * Description: Get event. (Currently multiple Saps are not supported.)
 *
 * Arguments:   SAP descriptor, event, wait.
 *
 * Returns:     0 on successful completion, otherwise a negative error value.
 * 
>*/

PUBLIC  ESRO_RetVal
esro_cb_getEvent(ESRO_SapDesc *sapDesc)
{
    Int retVal = SUCCESS;

    ESRO_Event events, *event = &events;

#ifdef TM_ENABLED
    static char taskName[100] = "esro_cb_getEvent, ";
#endif

 
    retVal = FAIL;
    if ( *sapDesc < 0 ) {
	EH_problem("esro_cb_getEvent: ***WATCH-OUT***: Invalid SAP descriptor");
	return ( FAIL );
    }

    if ( lous_cb_getEvent(sapCBMap[*sapDesc].primQu, &lous_event) == FAIL ) {

#ifdef TM_ENABLED
    TM_TRACE((ESRO_modCB, TM_ENTER, 
	     "esro_cb_getEvent: getEvent scheduled\n"));
    strcpy(taskName, "esro_cb_getEvent (CB)");
    SCH_submit ((Void *)esro_cb_getEvent, (Ptr) sapDesc, 
		(SCH_Event) sapCBMap[*sapDesc].primQu->eventQu, 
		(String) strcat(taskName, TM_here()));
#else
    SCH_submit ((Void *)esro_cb_getEvent, (Ptr) sapDesc, 
		(SCH_Event) sapCBMap[*sapDesc].primQu->eventQu);
#endif

	if ( errno == EINTR ) {
            TM_TRACE((ESRO_modCB, TM_ENTER, 
		     "esro_cb_getEvent: failed sapDesc=%d\n", *sapDesc));
	    return ( FAIL );
	}
        TM_TRACE((ESRO_modCB, TM_ENTER, 
		 "esro_cb_getEvent: failed sapDesc=%d\n", *sapDesc));
	return ( FAIL );
    }

#ifdef TM_ENABLED
	        TM_TRACE((ESRO_modCB, TM_ENTER, 
		     	 "esro_cb_getEvent: InvokeInd: getEvent scheduled\n"));

    		strcpy(taskName, "esro_cb_getEvent (by invokeInd) (CB)");
    		SCH_submit ((Void *)esro_cb_getEvent, 
			    (Ptr) sapDesc, 
			    (SCH_Event) SCH_PSEUDO_EVENT,
		            (String) strcat(taskName, TM_here()));
#else
    		SCH_submit ((Void *)esro_cb_getEvent, 
			    (Ptr) sapDesc, 
			    (SCH_Event) SCH_PSEUDO_EVENT);
#endif

    TM_TRACE((ESRO_modCB, TM_ENTER, "esro_cb_getEvent:   sapDesc=%d\n", 
	      *sapDesc));

    switch ( lous_event.type ) { 
    case LOPQ_INVOKE_IND :
        {
	    LOPQ_InvokeInd * invokeInd;
	    Int evSapDesc;
	    DU_View parameter;
    	    ESRO_InvokeId  userShellInvokeId;	/* User Shell Invoke Id */

    	    TM_TRACE((ESRO_modCB, TM_ENTER, 
		     "esro_cb_getEvent:  INVOKE.indication received\n"));

	    invokeInd = &(lous_event.un.invokeInd);

	    evSapDesc = invokeInd->locSapDesc;

	    TM_TRACE((ESRO_modCB, TM_ENTER, 
		     "esro_cb_getEvent: InvokeInd: sapDesc=0x%x invokeId=%d\n",
	             evSapDesc, invokeInd->invokeId));

	    if (!lous_addInvokeCB(invokeInd->locSapDesc, invokeInd->invokeId)) {
	    	EH_problem("esro_cb_getEvent: Invoke Indication: "
			   "lous_addInvokeCB failed. No invokeCB\n");
	    	return ( FAIL );
	    }

    	    if((userShellInvokeId = lous_addUSInvoke()) == -1) {
	    	EH_problem("esro_cb_getEvent: lous_addUSInvoke failed\n");
	        TM_TRACE((ESRO_modCB, TM_ENTER, 
		         "esro_cb_getEvent: lous_addUSInvoke failed\n"));
	        return (FAIL);
     	    }

            uToPInvokeIdMap[userShellInvokeId].state    = ACTIVE_STATE;
            uToPInvokeIdMap[userShellInvokeId].invokeId = invokeInd->invokeId;

	    TM_TRACE((ESRO_modCB, TM_ENTER, 
		     "esro_cb_getEvent: InvokeInd: usInvokeId=%d  psInvokeId=%d\n",
	             userShellInvokeId, invokeInd->invokeId));

	    event->type = lous_event.type;
	    event->un.invokeInd = *invokeInd;

	    if (invokeInd->len > 0) {
	       if ((parameter = lous_cpDataToDU (invokeInd->data, invokeInd->len))
		   == NULL) {
            	   EH_problem("esro_cb_getEvent: lous_cpDataToDU failed\n");
	       }
	    } else {
		parameter = NULL;
	    }

            (*sapCBMap[evSapDesc].invokeInd)
	                         (invokeInd->locSapDesc,
			          invokeInd->remESROSap,
			          &(invokeInd->remTsap),
			          &(invokeInd->remNsap),
				  lous_usInvokeId(invokeInd->invokeId),
			          invokeInd->operationValue,
			          invokeInd->encodingType,
			          parameter);

	    if (parameter) {
	    	DU_free(parameter);
	    }

	    retVal = SUCCESS;

	    break;
    	}

    case LOPQ_RESULT_IND :
    	{
	    LOPQ_ResultInd * resultInd;
	    Int evSapDesc;
	    DU_View parameter;

	    resultInd = &(lous_event.un.resultInd);

	    if (resultInd->invokeId >= LOPQ_INVOKE_CBS) {
		EH_problem("esro_cb_getEvent: Result Indication: "
		 	   "Invalid invoke Id: %ld!\n");
	        retVal = FAIL;
		break;
	    }

            TM_TRACE((ESRO_modCB, TM_ENTER, 
                    "esro_cb_getEvent:   RESULT.indication received, "
		    "psInvokeId=%d usInvokeId=%d\n", 
                    resultInd->invokeId, lous_usInvokeId(resultInd->invokeId)));

	    if (invokeCBMap[resultInd->invokeId].sapCB == 0) {
		EH_problem("esro_cb_getEvent: No sap control block found "
			   "for invokeId\n");
		retVal = (FAIL);
		break;
	    }

	    evSapDesc = invokeCBMap[resultInd->invokeId].sapCB->sapDesc;

	    TM_TRACE((ESRO_modCB, TM_ENTER, 
		     "esro_cb_getEvent: ResultInd: sapDesc=0x%x  psInvokeId=%d\n", 
		     evSapDesc, resultInd->invokeId));
	
	    if (resultInd->len > 0) {
	        if ((parameter = lous_cpDataToDU (resultInd->data, resultInd->len))
		    == NULL) {
            	    EH_problem("esro_cb_getEvent: lous_cpDataToDU failed\n");
	        }
	    } else {
		parameter = NULL;
	    }

            (*sapCBMap[evSapDesc].resultInd) (
			lous_usInvokeId(resultInd->invokeId), 
			resultInd->userInvokeRef,
		  	resultInd->encodingType,
		     	parameter);

	    lous_removeInvokeCB(&invokeCBMap[resultInd->invokeId]);
    	    lous_removeUSInvokeId(lous_usInvokeId(resultInd->invokeId));

	    if (parameter) {
	    	DU_free(parameter);
	    }

	    retVal = SUCCESS;
	    break;
    	}

    case LOPQ_ERROR_IND :
    	{
	    LOPQ_ErrorInd *errorInd;
	    Int evSapDesc;
	    DU_View parameter = NULL;

	    errorInd = &(lous_event.un.errorInd);

	    if (errorInd->invokeId >= LOPQ_INVOKE_CBS) {
		EH_problem("esro_cb_getEvent: Error Indication: "
		 	   "Invalid invoke Id: %ld!\n");
	        retVal = FAIL;
		break;
	    }

            TM_TRACE((ESRO_modCB, TM_ENTER, 
                    "esro_cb_getEvent:   ERROR.indication received "
		    "invokeId=%d usInvokeId=%d\n", 
                    errorInd->invokeId, lous_usInvokeId(errorInd->invokeId)));

	    if (invokeCBMap[errorInd->invokeId].sapCB == 0) {
		EH_problem("esro_cb_getEvent: No sap control block found "
			   "for invokeId\n");
		retVal = (FAIL);
		break;
	    }

	    evSapDesc = invokeCBMap[errorInd->invokeId].sapCB->sapDesc;

	    TM_TRACE((ESRO_modCB, TM_ENTER, 
		     "esro_cb_getEvent: ErrorInd: sapDesc=0x%x  "
		     "psInvokeId=%d  errorValue=%d\n", 
		     evSapDesc, errorInd->invokeId, errorInd->errorValue));
	
	    if (errorInd->len > 0) {
	    	if ((parameter = lous_cpDataToDU (errorInd->data, errorInd->len))
		    == NULL) {
            	    EH_problem("esro_cb_getEvent: lous_cpDataToDU failed\n");
	    	}
	    } else {
		parameter = NULL;
	    }

            (*sapCBMap[evSapDesc].errorInd) (
			lous_usInvokeId(errorInd->invokeId), 
			errorInd->userInvokeRef,
		  	errorInd->encodingType,
		  	errorInd->errorValue,
		     	parameter);

	    lous_removeInvokeCB(&invokeCBMap[errorInd->invokeId]);
    	    lous_removeUSInvokeId(lous_usInvokeId(errorInd->invokeId));

	    if (parameter) {
	    	DU_free(parameter);
	    }

	    retVal = SUCCESS;
	    break;
    	}

    case LOPQ_RESULT_CNF :
    	{
	    LOPQ_ResultCnf * resultCnf;
	    Int evSapDesc;

	    resultCnf = &(lous_event.un.resultCnf);

	    if (resultCnf->invokeId >= LOPQ_INVOKE_CBS) {
		EH_problem("esro_cb_getEvent: Result Confirm: "
		 	   "Invalid invoke Id: %ld!\n");
	        retVal = FAIL;
		break;
	    }

            TM_TRACE((ESRO_modCB, TM_ENTER, 
                    "esro_cb_getEvent:   RESULT Confirm received, "
		    "psInvokeId=%d usInvokeId=%d\n", 
                    resultCnf->invokeId, lous_usInvokeId(resultCnf->invokeId)));

	    evSapDesc = invokeCBMap[resultCnf->invokeId].sapCB->sapDesc;

	    TM_TRACE((ESRO_modCB, TM_ENTER, 
		     "esro_cb_getEvent: ResultCnf: sapDesc=0x%x invokeId=%d "
		     "usInvokeId=%d\n", 
		     evSapDesc, resultCnf->invokeId, 
		     lous_usInvokeId(resultCnf->invokeId)));

	    event->type = lous_event.type;
	    event->un.resultCnf = *resultCnf;

            (*sapCBMap[evSapDesc].resultCnf) (
			lous_usInvokeId(resultCnf->invokeId),
			resultCnf->userInvokeRef);

	    lous_removeInvokeCB(&invokeCBMap[resultCnf->invokeId]);
    	    lous_removeUSInvokeId(lous_usInvokeId(resultCnf->invokeId));

	    retVal = SUCCESS;
	    break;
    	}

    case LOPQ_ERROR_CNF :
    	{
	    LOPQ_ErrorCnf * errorCnf;
	    Int evSapDesc;

	    errorCnf = &(lous_event.un.errorCnf);

	    if (errorCnf->invokeId >= LOPQ_INVOKE_CBS) {
		EH_problem("esro_cb_getEvent: Error Confirm: "
		 	   "Invalid invoke Id: %ld!\n");
	        retVal = FAIL;
		break;
	    }

            TM_TRACE((ESRO_modCB, TM_ENTER, 
                    "esro_cb_getEvent:   Error Confirm received, "
		    "psInvokeId=%d usInvokeId=%d\n", 
                    errorCnf->invokeId, lous_usInvokeId(errorCnf->invokeId)));

	    evSapDesc = invokeCBMap[errorCnf->invokeId].sapCB->sapDesc;

	    TM_TRACE((ESRO_modCB, TM_ENTER, 
		     "esro_cb_getEvent: ResultCnf: sapDesc=0x%x "
		     "invokeId=%d usInvokeId=%d\n", 
		     evSapDesc, errorCnf->invokeId, 
		     lous_usInvokeId(errorCnf->invokeId)));

	    event->type = lous_event.type;
	    event->un.errorCnf = *errorCnf;

            (*sapCBMap[evSapDesc].errorCnf) (
			lous_usInvokeId(errorCnf->invokeId),
			errorCnf->userInvokeRef);

	    lous_removeInvokeCB(&invokeCBMap[errorCnf->invokeId]);
    	    lous_removeUSInvokeId(lous_usInvokeId(errorCnf->invokeId));

	    retVal = SUCCESS;
	    break;
    	}

    case LOPQ_FAILURE_IND :
        {
	    LOPQ_FailureInd *failureInd;
	    Int evSapDesc;

	    failureInd = &(lous_event.un.failureInd);

	    if (failureInd->invokeId >= LOPQ_INVOKE_CBS) {
		EH_problem("esro_cb_getEvent: Failure Indication: "
		 	   "Invalid invoke Id: %ld!\n");
	        retVal = FAIL;
		break;
	    }

            TM_TRACE((ESRO_modCB, TM_ENTER, 
                    "esro_cb_getEvent:   Failure Indication received, "
		    "psInvokeId=%d usInvokeId=%d\n", 
                    failureInd->invokeId, 
		    lous_usInvokeId(failureInd->invokeId)));

	    evSapDesc = invokeCBMap[failureInd->invokeId].sapCB->sapDesc;

	    TM_TRACE((ESRO_modCB, TM_ENTER, 
		     "esro_cb_getEvent: #####FailureInd: sapDesc=0x%x\n", 
		     evSapDesc));

	    event->type = lous_event.type;
	    event->un.failureInd = *failureInd;

            (*sapCBMap[evSapDesc].failureInd) (
				lous_usInvokeId(failureInd->invokeId),
			        failureInd->userInvokeRef,
				failureInd->failureValue);
	
	    lous_removeInvokeCB(&invokeCBMap[failureInd->invokeId]);
    	    lous_removeUSInvokeId(lous_usInvokeId(failureInd->invokeId));

	    retVal = SUCCESS;
	    break;
	}

	/*
	 * Status Fields Must be immediately delivered
	 */
    case LOPQ_SAP_BIND_STAT:
	/* 
	 * Should really queue it up on the sap event queue.
	 * But the way it is beeing used, it is OK not to do this.
	 */ 
        TM_TRACE((ESRO_modCB, TM_ENTER, 
		 "esro_cb_getEvent: SAP_BIND_STAT received\n"));

	*((LOPQ_PStatus *)event) = *((LOPQ_PStatus *)&lous_event);

	retVal = SUCCESS;

  	break;

    case LOPQ_INVOKE_REQ_STAT :
	{
    	LOPQ_PStatus   *pStatus;		/* Primitive Status */
    	ESRO_InvokeId  invokeId;		/* Invoke Id */
    	ESRO_InvokeId  userShellInvokeId;	/* User Shell Invoke Id */
    	ESRO_UserInvokeRef  userInvokeRef;	/* User Invoke Reference */
    	Int evSapDesc;

    	TM_TRACE((ESRO_modCB, TM_ENTER, 
		 "esro_cb_getEvent:   INVOKE_REQ_STAT received\n"));

        pStatus = (LOPQ_PStatus *)&lous_event;

    	evSapDesc = pStatus->un.invokeReqStat.locSapDesc;
	invokeId = pStatus->un.invokeReqStat.invokeId;
	userShellInvokeId = pStatus->un.invokeReqStat.userShellInvokeId;
	userInvokeRef = pStatus->un.invokeReqStat.userInvokeRef;

    	TM_TRACE((ESRO_modCB, TM_ENTER, 
		 "esro_cb_getEvent:   psInvokeId=%d  usInvokeId=%d\n", 
		 invokeId, userShellInvokeId));

	if ( pStatus->status ) {
	    TM_HEXDUMP(ESRO_modCB, ESRO_TRPRIM, "invokeReq status",  
		       (Byte *) pStatus, 20);
	    TM_TRACE((ESRO_modCB, TM_ENTER, 
		     "ESRO_invokeReq: status indicates that invoke failed"));
/***            lous_removeInvokeCB(invokeCB); ***/
            lous_removeUSInvokeId(userShellInvokeId);

            (*sapCBMap[evSapDesc].failureInd) (userShellInvokeId,
					       userInvokeRef,
				               ESRO_FailureLocResource);

	    retVal = FAIL;
	    break;
	} 

#if 0  /*?? Spec doesn't say ??*/
        if ( invokeId == (ESRO_InvokeId) 0 ) {
	     EH_problem("ESRO_invokeReq: Cannot assign invoke ID");
             uToPInvokeIdMap[userShellInvokeId].state = INACTIVE_STATE;
             uToPInvokeIdMap[userShellInvokeId].invokeId = -1;

             (*sapCBMap[evSapDesc].failureInd) (userShellInvokeId,
					        userInvokeRef,
				                ESRO_FailureLocResource);
	    retVal = FAIL;
	    break;
        }
#endif
        if (lous_addInvokeCB(evSapDesc, invokeId) == 0) {
	    EH_problem("ESRO_invokeReq: invokeCB entry already active");

             uToPInvokeIdMap[userShellInvokeId].state = INACTIVE_STATE;
             uToPInvokeIdMap[userShellInvokeId].invokeId = -1;

             (*sapCBMap[evSapDesc].failureInd) (userShellInvokeId,
					        userInvokeRef,
				                ESRO_FailureLocResource);
	    retVal = FAIL;
	    break;
        }

        uToPInvokeIdMap[userShellInvokeId].state    = ACTIVE_STATE;
        uToPInvokeIdMap[userShellInvokeId].invokeId = invokeId;

   	retVal = SUCCESS;
	break;
    }

    default:
	EH_problem("esro_cb_getEvent: ***** Invalid event *****");

        TM_TRACE((ESRO_modCB, TM_ENTER, 
		 "Inv event.type= %d  0x%x\n", lous_event.type,lous_event.type));

        TM_HEXDUMP(ESRO_modCB, ESRO_TRPRIM, 
		       "getEvent: Bad Event", (Byte *)  &lous_event, 20);

	event->type = -1;

   	retVal = FAIL;
	break;
    }

    return retVal;

} /* esro_cb_getEvent() */



/*<
 * Function:    lous_cb_getEvent
 *
 * Description: User shell get event.
 *
 * Arguments:   Primitive queue, event, waiting time.
 *
 * Returns:     0 on successful completion, otherwise a negative error value.
 * 
>*/

Int
lous_cb_getEvent(USQ_PrimQuInfo *primQu, LOPQ_Event *event)
{
    Int retVal;

    if (primQu == 0) {
	return (FAIL);
    }

    if ((retVal = USQ_getEvent(primQu, (Ptr)event, sizeof(*event))) != FAIL) {

    	TM_TRACE((ESRO_modCB, TM_ENTER, 
	         "lous_cb_getEvent:  event=%d  action=%d   event->type=%d\n",
		 primQu->eventQu, primQu->actionQu, event->type));

	return retVal;
    } else if (errno == EINTR) {	/* remove it */
/*    	TM_TRACE((ESRO_modCB, TM_ENTER, 
		 "lous_cb_getEvent: USQ_getEvent failed. Errno is EINTR! "
		 "It's OK!\n"));
*/
    	return retVal;			/* remove it */
    } else {
/*    	TM_TRACE((ESRO_modCB, TM_ENTER, 
		 "lous_cb_getEvent: USQ_getEvent failed. It's OK!\n"));
*/
	return retVal;
    }
}


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
ESRO_CB_getSchEvent(ESRO_SapDesc sapDesc)
{
    return ((SCH_Event *)&(sapCBMap[sapDesc].primQu->eventQu));
}


/*<
 * Function:    esro_storeEvent
 *
 * Description: Store event
 *
 * Arguments:   Primitive queue
 *
 * Returns:     
 * 
>*/

Int
esro_storeEvent(USQ_PrimQuInfo *primQu)
{
    lous_EventQuInfo *eventQuInfo;

    if (!(eventQuInfo = (lous_EventQuInfo *)SEQ_elemObtain(eventQuInfoPool))) {
	EH_problem("esro_storeEvent: SEQ_elemObtain failed");
	return 0;
    }

    eventQuInfo->quInfo = primQu;
    QU_INSERT (&eventQuInfoSeq, eventQuInfo);

    return (SUCCESS);	
}


/*<
 * Function:    esro_eventAvailable
 *
 * Description: Check if event available on the given queue.
 *
 * Arguments:   Primitive queue.
 *
 * Returns:  
 * 
>*/
Int
esro_eventAvailable(USQ_PrimQuInfo *primQu)
{
    lous_EventQuInfo *eventQuInfo;
    
    for (eventQuInfo = eventQuInfoSeq.first;  
	 eventQuInfo != (lous_EventQuInfo *)&eventQuInfoSeq;
	 eventQuInfo = eventQuInfo->next) {
	if (primQu->eventQu == eventQuInfo->quInfo->eventQu) {
	    return (SUCCESS);
	}
    }

    return (FAIL);	
}


/*<
 * Function:    lous_cpDataToDU
 *
 * Description: Get data and its length. Allocate Data Unit memory and copy
 *              the data.
 *
 * Arguments:   Data, lenght.
 *
 * Returns:     Pointer to Data Unit View.
 * 
>*/

STATIC  DU_View
lous_cpDataToDU(unsigned char *buffer, int length)
{
    DU_View du;
    extern DU_Pool *G_duMainPool;

    if (length == 0) {
	return((DU_View) 0);
    }

    if ( (du = DU_alloc(G_duMainPool, length)) == 0 ) {
	EH_problem("lous_cpDataToDu: DU_alloc failed\n");	
	return((DU_View) 0);
    }

    TM_TRACE((DU_modCB, DU_MALLOC, 
             "lous_cpDataToDU: DU_alloc: du=0x%x\n", du));

    OS_copy(DU_data(du), buffer, length);
    return(du);
}



/*<
 * Function:    lous_addUSInvoke
 *
 * Description: Add invoke
 *
 * Arguments:   None
 *
 * Returns:     Invoke Id.
 * 
>*/

Int
lous_addUSInvoke()
{
    UTOPInvokeIdMap *mapper;

    for (mapper = uToPInvokeIdMap;  mapper < uToPInvokeIdMap + LOPQ_INVOKE_CBS;
	 mapper++) {
        if (mapper->state == INACTIVE_STATE ) {
            mapper->state = ACTIVE_STATE;
	    return (mapper - uToPInvokeIdMap);
     	}
    }
    EH_problem("lous_addUSInvoke: No more invoke Id left in user shell\n");

    return -1;
}


/*<
 * Function:    lous_removeUSInvokeId
 *
 * Description: Remove invoke
 *
 * Arguments:   Invoke Id
 *
 * Returns:     None
 * 
>*/

Void
lous_removeUSInvokeId(ESRO_InvokeId userShellInvokeId)
{
    uToPInvokeIdMap[userShellInvokeId].state = INACTIVE_STATE;
    uToPInvokeIdMap[userShellInvokeId].invokeId = -1;
}


/*<
 * Function:    lous_usInvokeId
 *
 * Description: Get provider shell invoke id and return user shell invoke id
 *
 * Arguments:   Provider shell Invoke Id
 *
 * Returns:     None
 * 
>*/

Int
lous_usInvokeId(ESRO_InvokeId providerShellInvokeId)
{
    UTOPInvokeIdMap *idMap, *lastId = uToPInvokeIdMap + LOPQ_INVOKE_CBS;

    for (idMap = uToPInvokeIdMap; idMap < lastId; idMap++) {
	if (idMap->invokeId == providerShellInvokeId) {
	    return (idMap - uToPInvokeIdMap);
	}
    }
    return -1;
}
