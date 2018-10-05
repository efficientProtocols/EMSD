/*
 *  This file is part of the Open C Platform (OCP) Library. OCP is a
 *  portable library for development of Data Communication Applications.
 * 
 *  Copyright (C) 1995 Neda Communications, Inc.
 * 	Prepared by Mohsen Banan (mohsen@neda.com)
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
 * File name: psq.c
 *
 * Description: Provider Shell Queue
 * 
 * Functions:
 *   PSQ_init()
 *   PSQ_procPubQu(Ptr data, Int size)
 *   psq_schedPubQu(Int **pubSockDescript)
 *   psq_schedActionQu(PSQ_PrimQuInfo *quInfo)
 *   PSQ_putEvent(PSQ_PrimQuInfo *quInfo, Byte *data, Int size)
 *   PSQ_getQuInfo(String chnlName)
 *   psq_procActionQu(IMQ_PrimMsg *data, Int size)
 *   psq_primQuInit(Int nuOfQus)
 *   psq_primQuAdd(SP_Action *spsPrim)
 *   psq_primQuRemove(quInfo)
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: psq.c,v 1.12 1996/11/07 02:57:32 kamran Exp $";
#endif /*}*/

#include  "eh.h"
#include  "estd.h"
#include  "seq.h"
#include  "tm.h"
#include  "imq.h"
#include  "psq.h"
#include  "sp_shell.h"
#include  "sch.h"

#include  "target.h"
#include  "sf.h"

#define PUB_QU_NAME "/tmp/SP"
char pubQuName[512] = PUB_QU_NAME;

LOCAL Int psq_primQuInit(Int nuOfQus);
PUBLIC SuccFail psq_primQuAdd(SP_Action *spsPrim);
Int psq_procActionQu(IMQ_PrimMsg *data, Int size);
Int psq_schedPubQu(Int **pubSockDescript);
Int psq_schedActionQu(PSQ_PrimQuInfo *quInfo);
Int PSQ_procPubQu(Ptr data, Int size);
 
static IMQ_PrimDesc pubQuDesc;

typedef struct PrimQuHead {
    PSQ_PrimQuInfo *first;
    PSQ_PrimQuInfo *last;
} PrimQuHead;

static PrimQuHead availPrimQu;
LOCAL  PrimQuHead activePrimQu;

STATIC PSQ_QuRegPrvdrSeq quRegPrvdrSeq;
STATIC SEQ_PoolDesc   quRegPrvdrPool;
 

/*<
 * Function:	PSQ_init
 *
 * Description:	Initialize Provider Shell Queue
 *
 * Arguments:	None
 *
 * Returns:	None
 *
>*/
PUBLIC Void 
PSQ_init()
{
    IMQ_Key  pubQuKey;

    static Bool virgin = TRUE;

    if ( !virgin ) {
	return;
    }
    virgin = FALSE;

    IMQ_init(IMQ_ELEMENTS);

    quRegPrvdrPool = SEQ_poolCreate(sizeof(*quRegPrvdrSeq.first), 0);
    QU_INIT(&quRegPrvdrSeq);
    
    pubQuKey  = IMQ_keyCreate(pubQuName, 0);
    pubQuDesc = IMQ_primGet(pubQuKey);

    psq_primQuInit(10);         /* Initialize Primitive Qu Tables */

} /* PSQ_init() */


/*<
 * Function:	PSQ_procPubQu
 *
 * Description: Data arriving to pubQuDesc 
 *
 * Arguments:	data, size
 *
 * Returns: 	0 on succesfull completion, -1 otherwise.
 *
>*/
PUBLIC Int
PSQ_procPubQu(Ptr data, Int size)
{
    SP_Action *spsPrim;
#ifdef FUTURE
    SP_Action addCnf;
#endif

    spsPrim = (SP_Action *)data;
    
    switch (spsPrim->type) {
    case SP_QUADDREQ:
	psq_primQuAdd(spsPrim);
	break;
    default:
	EH_problem("PSQ_procPubQu: Unknown primitive in public queue\n");
	return -1;
    }
    return 0;

} /* PSQ_procPubQu() */



/*<
 * Function:    psq_schedPubQu
 *
 * Description: Schedule the public queue tasks.
 *
 * Arguments:   Public queue socket descriptor.
 *
 * Returns:     0 if successful, a negative error value otherwise.
 *
>*/

Int
psq_schedPubQu(Int **pubSockDescript)
{
    static Byte bigMsg[MAXACTIONQU];
    IMQ_PrimMsg *msgPtr = (IMQ_PrimMsg *) bigMsg;
    Int size;
    static char taskNamePub[50] = "Public Queue: ";

    /* Process Public Qu */
    if ((size = IMQ_primRcv((IMQ_PrimDesc)*pubSockDescript, msgPtr, 
        		    SP_getSize(SP_QUADDREQ))) 
         != -1) {
	    /* SYSV check for ENOMSG */
	    PSQ_procPubQu((Ptr) msgPtr, size);
    } else {
	    EH_problem("psq_schedPubQu: IMQ_primRcv failed");
    }

    strcpy(taskNamePub, "Public Queue: ");

    return 1;

} /* psq_schedPubQu() */



/*<
 * Function:    psq_schelActionQu
 *
 * Description: Schedule the action queue tasks.
 *
 * Arguments:   Ponter to action queue structure.
 *
 * Returns:     0 if successful, a negative error value otherwise.
 *
>*/

Int
psq_schedActionQu(PSQ_PrimQuInfo *quInfo)
{
    static Byte bigMsg[MAXACTIONQU];
    IMQ_PrimMsg *msgPtr = (IMQ_PrimMsg *) bigMsg;
    Int size;
    static char taskNameAct[50] = "Action Queue: ";

    /* action primitive Queue */
    if ((size = IMQ_primRcv(quInfo->actionQu, msgPtr, sizeof(bigMsg))) != -1) {
		/* SYSV check for ENOMSG */
	PSQ_procActionQu(msgPtr, size, quInfo);

	strcpy(taskNameAct, "Action Queue: ");

    } else {

	PSQ_QuRegPrvdr *quRegPrvdr;
    	for (quRegPrvdr = (PSQ_QuRegPrvdr *)quRegPrvdrSeq.first; 
	     quRegPrvdr != (PSQ_QuRegPrvdr *) &quRegPrvdrSeq ; 
	     quRegPrvdr = quRegPrvdr->next) {

	    (*(quRegPrvdr->cleanSap))(quInfo);
	}
		
	PSQ_primQuRemove(quInfo);
	EH_problem("psq_schedActionQu: IMQ_primRcv failed "
		   "(means user is not running anymore)");
    } 

    return 1;

} /* psq_schedActionQu() */



/*<
 * Function:	PSQ_putEvent
 *
 * Description:	Put event
 *
 * Arguments:	Queue pointer, data, size
 *
 * Returns:	0 on successfule completion
 *
>*/
PUBLIC SuccFail
PSQ_putEvent(PSQ_PrimQuInfo *quInfo, Byte *data, Int size)
{
    return  IMQ_primSnd(quInfo->eventQu, (IMQ_PrimMsg *) data, size);
}


/*<
 * Function:	PSQ_getQuInfo
 *
 * Description:	Get queue info
 *
 * Arguments:	channel name
 *
 * Returns:	Pointer to primitive queue if successful, 0 otherwise
 *
>*/
PUBLIC PSQ_PrimQuInfo *
PSQ_getQuInfo(String chnlName)
{
    PSQ_PrimQuInfo *quInfo;

    for (quInfo = activePrimQu.first; quInfo != (PSQ_PrimQuInfo *)&activePrimQu;
	    quInfo = quInfo->next) {
	if (strcmp(chnlName, quInfo->chnlName) == 0 ) {
	    return quInfo;
	}
    }

    return (PSQ_PrimQuInfo *)0;
}



/*<
 * Function:	psq_procActionQu
 *
 * Description:	Process action queue
 *
 * Arguments:	data, size of data
 *
 * Returns:	
 *
>*/
Int
PSQ_procActionQu(IMQ_PrimMsg *data, Int size, PSQ_PrimQuInfo *quInfo)
{
    /* All Action Primitives have an event as the first field fo them */
    Int type;
    PSQ_QuRegPrvdr *quRegPrvdr;

    type = ((SP_Action *)data)->type;
	
    for (quRegPrvdr  = (PSQ_QuRegPrvdr *) quRegPrvdrSeq.first; 
     	 quRegPrvdr != (PSQ_QuRegPrvdr *) &quRegPrvdrSeq ; 
	 quRegPrvdr  = quRegPrvdr->next) {

    	if (type  >  quRegPrvdr->lowerEnd  && type  <  quRegPrvdr->upperEnd )
	{
	    (*quRegPrvdr->procAction)(data, size);
	    break;
	}
    }

    if (quRegPrvdr == (PSQ_QuRegPrvdr *) &quRegPrvdrSeq) {
	EH_problem("PSQ_procActionQu: Invalid action primitive");
        return -2;
    }	

/*	OBSOLETE: Kept just FYI
    if ( type > TS_A_BASE  && type < BAD_ACTION) {
	TPS_userIn(data, size);
    } else if ( type > EGS_A_BASE  && type < TS_A_BASE) {
	EGPS_userIn(data, size);
    } else if ( type > NS_A_BASE  && type < EGS_A_BASE) {
	printf("Network Primitive\n");
    } else {
	EH_oops();
    }
*/
    return 0;
}
    

/*<
 * Function:	psq_primQuInit
 *
 * Description:	Initialize primitive queue
 *
 * Arguments:	Number of queues
 *
 * Returns:	0 on successful completion
 *
>*/
LOCAL Int
psq_primQuInit(Int nuOfQus)
{
    Int i;
    PSQ_PrimQuInfo *quInfo;

    QU_INIT(&availPrimQu);
    QU_INIT(&activePrimQu);

    for (i = 0; i < nuOfQus; i++) {
	if ((quInfo = (PSQ_PrimQuInfo *) SF_memGet(sizeof(*quInfo))) 
	    == (PSQ_PrimQuInfo *)0) {
	    EH_problem("psq_primQuInit: Out of memory\n");
    	    return FAIL;
	}
	QU_INIT(quInfo);
	QU_INSERT(quInfo, &availPrimQu);
    }
    return 0;
}


/*<
 * Function:	psq_primQuAdd
 *
 * Description: Add primitive queue
 *
 * Arguments:	Action primtive for add
 *
 * Returns:
 *
>*/
PUBLIC SuccFail
psq_primQuAdd(SP_Action *spsPrim)
{
    SP_Action addCnf;
    SP_QuAddReq	*quAddReq;
    PSQ_PrimQuInfo *quInfo;
    IMQ_Key actionQuKey;
    IMQ_Key eventQuKey;
    static char taskNameAct[50] = "Action Queue: ";

    quAddReq = &(spsPrim->un.quAddReq);

    if ((quInfo = availPrimQu.first) == (PSQ_PrimQuInfo *) &availPrimQu) {
	EH_problem("psq_primQuAdd: Out of Prim Queues\n");
	return ( FAIL );
    }
    
    strcpy(quInfo->chnlName, quAddReq->chnlName);

    actionQuKey = IMQ_keyCreate(quAddReq->chnlName, 0);
    quInfo->actionQu = IMQ_primGet(actionQuKey);

    eventQuKey = IMQ_keyCreate(quAddReq->chnlName, 1);
    quInfo->eventQu = IMQ_primGet(eventQuKey);

    QU_move(quInfo, &activePrimQu);

    strcpy(taskNameAct, "Action Queue: ");

    addCnf.type  	      = SP_QUADDCNF;
    addCnf.un.quAddCnf.status = SUCCESS;
    IMQ_primSnd(quInfo->eventQu, (IMQ_PrimMsg *)&addCnf, 
		SP_getSize(SP_QUADDCNF));

    return ( SUCCESS );
}


/*<
 * Function:	PSQ_primQuRemove
 *
 * Description:	Remove primitive queue
 *
 * Arguments:	Queue pointer
 *
 * Returns:	0 on successful completion, -1 otherwise
 *
>*/
PUBLIC SuccFail
PSQ_primQuRemove(PSQ_PrimQuInfo *quInfo)
{
#ifndef MSDOS
    close(quInfo->actionQu);
    close(quInfo->eventQu);
    shutdown(quInfo->actionQu, 2);
    shutdown(quInfo->eventQu,  2);
#endif
#if 0
    unlink(IMQ_keyCreate(quInfo->chnlName, 0));
    unlink(IMQ_keyCreate(quInfo->chnlName, 1));
#endif

    QU_move(quInfo, &availPrimQu);
    return ( SUCCESS );
}


/*<
 * Function:    PSQ_register
 *
 * Description: Register event boundaries
 *
 * Arguments:	Lower end, upper end, action function, cleanup function.
 *
 * Returns: 
 *
>*/

Int
PSQ_register(Int lowerEnd, Int upperEnd, 
	     Int (*lops_procAction)(IMQ_PrimMsg *, Int), 
    	     Int (*lops_cleanSap)(PSQ_PrimQuInfo *))
{
    PSQ_QuRegPrvdr *quRegPrvdr;

    if ( ! (quRegPrvdr = (PSQ_QuRegPrvdr *) SEQ_elemObtain(quRegPrvdrPool)) ) {
	EH_problem("PSQ_register: SEQ_elemObtain failed");
	return (0);
    }

    quRegPrvdr->lowerEnd   = lowerEnd;
    quRegPrvdr->upperEnd   = upperEnd;
    quRegPrvdr->procAction = lops_procAction;
    quRegPrvdr->cleanSap   = lops_cleanSap;

    QU_INSERT ((QU_Elem *)&quRegPrvdrSeq, quRegPrvdr);
    
    return (SUCCESS);
}
