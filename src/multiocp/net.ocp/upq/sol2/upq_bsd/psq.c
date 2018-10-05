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
 * File name: psq.c (Provider Shell Queue)
 *
 * Description: Provider Shell Queue.
 *
 * Functions:
 *   PSQ_init(void)
 *   PSQ_putEvent(PSQ_PrimQuInfo *quInfo, Byte *data, Int size)
 *   PSQ_getQuInfo(String chnlName)
 *   psq_procPubQu(Ptr data, Int size)
 *   PSQ_procActionQu(IMQ_PrimMsg *data, Int size, quInfo)
 *   PSQ_primQuInit(Int nuOfQus)
 *   PSQ_primQuAdd(SP_Action *spsPrim)
 *   PSQ_primQuRemove(PSQ_PrimQuInfo *quInfo)
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: psq.c,v 1.16 1996/11/07 02:57:30 kamran Exp $";
#endif /*}*/

#include  "estd.h"
#include  "eh.h"
#include  "sf.h"
#include  "tm.h"
#include  "imq.h"
#include  "psq.h"
#include  "sp_shell.h"
#include  "sch.h"
#include  "target.h"
#include  "seq.h"

#define PUB_QU_NAME "/tmp/SP"  	      /* The Global Service Provider Queue */
char pubQuName[512] = PUB_QU_NAME;    /* The Global Service Provider Queue */

typedef struct PrimQuHead {
    QU_HEAD;			/* PSQ_PrimQuInfo */
} PrimQuHead;

static PrimQuHead availPrimQu;
LOCAL  PrimQuHead activePrimQu;

STATIC PSQ_QuRegPrvdrSeq quRegPrvdrSeq;
STATIC SEQ_PoolDesc   quRegPrvdrPool;

Int pubSockDesc;

static Int psq_procPubQu(Ptr data, Int size);
Int PSQ_procActionQu(IMQ_PrimMsg *data, Int size, PSQ_PrimQuInfo *quInfo);
Int psq_schedPubQu(Int *pubSockDescipt);
Int lops_cleanSap();

extern Void eh_unixProblem();
 

/*<
 * Function:    PSQ_init
 *
 * Description: Initialize Privder Shell Queue
 *
 * Arguments:   None.
 *
 * Returns:     None.
 *
>*/

PUBLIC Void 
PSQ_init(void)
{
    struct sockaddr_un name;
    Int nameLength;
    static int sigMask;
    Int origMask;

    static Bool virgin = TRUE;

#ifdef TM_ENABLED
    static char taskNamePub[OS_MAX_FILENAME_LEN + 15] = "Public Queue: ";
#endif

    if (!virgin) {
	return;
    }
    virgin = FALSE;

    sigMask = sigmask(SIGALRM) | sigmask(SIGUSR1) | sigmask(SIGUSR2);

    /* Inititialize the Inter Module Queue */
    IMQ_init();

    quRegPrvdrPool = SEQ_poolCreate(sizeof(PSQ_PrimQuInfo), 0);
    QU_INIT(&quRegPrvdrSeq);
    
    /* Create Socket from which to read */
    origMask    = sigblock(sigMask);
    pubSockDesc = socket(AF_UNIX, SOCK_DGRAM, 0);
    sigsetmask(origMask);
    if (pubSockDesc < 0) {
	perror("PSQ_init: opening datagram socket failed");
	exit(1);
    }

    if (unlink(pubQuName) == SUCCESS) {	/* In case it was left from last time */
	EH_unixProblem("PSQ_init: Removing Left Over File");
    }
    
    /* Create Public Qu Name. */
    OS_memSet((char *)&name, '\0', sizeof(name));
    name.sun_family = AF_UNIX;
    strcpy(name.sun_path, pubQuName);
#ifdef SOL2
    nameLength = sizeof(name.sun_family) + strlen(name.sun_path);
#else
    nameLength = sizeof(name);
#endif

    origMask = sigblock(sigMask);
    if (bind(pubSockDesc, (struct sockaddr *) &name, nameLength) < 0) {
	EH_unixProblem("PSQ_init: Unable to bind");
	EH_problem("PSQ_init: bind failed");
	perror("PSQ_init: binding name to datagram socket");
	exit(1);
    }
    sigsetmask(origMask);

    /* Initialize Primitive Qu Tables */
    PSQ_primQuInit(50);

#ifdef TM_ENABLED
    strcpy(taskNamePub, "Public Queue: ");
    SCH_submit((Void (*)(void))psq_schedPubQu, (Ptr) &pubSockDesc, 
		(SCH_Event) pubSockDesc,
		(String) strcat(taskNamePub, TM_here()));
#else
    SCH_submit((Void (*)(void))psq_schedPubQu, (Ptr) &pubSockDesc, 
		(SCH_Event) pubSockDesc);
#endif

} /* PSQ_init() */



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
psq_schedPubQu(Int *pubSockDescript)
{
    static Byte bigMsg[MAXACTIONQU];
    IMQ_PrimMsg *msgPtr = (IMQ_PrimMsg *) bigMsg;
    Int size;

#ifdef TM_ENABLED
    static char taskNamePub[OS_MAX_FILENAME_LEN + 15] = "Public Queue: ";
#endif

    /* Process Public Qu */
    if ((size = IMQ_primRcv(*pubSockDescript, msgPtr, 
			    SP_getSize(SP_QUADDREQ))) 
         != -1) {
	    /* SYSV check for ENOMSG */
	    psq_procPubQu((Ptr) msgPtr, size);
    } else {
	    EH_problem("psq_schedPubQu: IMQ_primRcv failed");
    }


#ifdef TM_ENABLED
    strcpy(taskNamePub, "Public Queue: ");
    SCH_submit((Void (*)(void)) psq_schedPubQu, (Ptr) pubSockDescript, 
		(SCH_Event) *pubSockDescript, 
		(String) strcat(taskNamePub, TM_here()));
#else
    SCH_submit((Void (*)(void)) psq_schedPubQu, (Ptr) pubSockDescript, 
		(SCH_Event) *pubSockDescript);
#endif

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

#ifdef TM_ENABLED
    static char taskNameAct[OS_MAX_FILENAME_LEN + 15] = "Action Queue: ";
#endif

    /* action primitive Queue */
    if ((size = IMQ_primRcv(quInfo->actionQu, msgPtr, sizeof(bigMsg))) != -1) {
		/* SYSV check for ENOMSG */
	PSQ_procActionQu(msgPtr, size, quInfo);

#ifdef TM_ENABLED
	strcpy(taskNameAct, "Action Queue: ");
    	SCH_submit((Void (*)(void)) psq_schedActionQu, (Ptr) quInfo, 
		   (SCH_Event) quInfo->actionQu,
		   (String) strcat(taskNameAct, TM_here()));
#else
    	SCH_submit((Void (*)(void)) psq_schedActionQu, (Ptr) quInfo, 
		   (SCH_Event) quInfo->actionQu);
#endif
    } else {

	PSQ_QuRegPrvdr *quRegPrvdr;
    	for (quRegPrvdr = (PSQ_QuRegPrvdr *)quRegPrvdrSeq.first; 
	     quRegPrvdr != (PSQ_QuRegPrvdr *) &quRegPrvdrSeq ; 
	     quRegPrvdr = quRegPrvdr->next) {

	    (*(quRegPrvdr->cleanSap))(quInfo);
	}
		
	PSQ_primQuRemove(quInfo);
#ifdef TM_ENABLED
	fprintf(stderr, 
                "psq_schedActionQu: IMQ_primRcv failed (means user is dead)");
#endif
    } 

    return 1;

} /* psq_schedActionQu() */



/*<
 * Function:    PSQ_putEvent
 *
 * Description: Put an event in the queue.
 *
 * Arguments:   Primitive queue, data, data size.
 *
 * Returns:     Number of bytes written or -1 in case of error.
 *
>*/

PUBLIC Int
PSQ_putEvent(PSQ_PrimQuInfo *quInfo, Byte *data, Int size)
{
    Int retVal;

    if ( (retVal = IMQ_primSnd(quInfo->eventQu, (IMQ_PrimMsg *) data, size))
          < 0 || retVal != size)           
                  return (FAIL);          
    else                                 
                  return retVal;        
}


/*<
 * Function:    PSQ_getQuInfo
 *
 * Description: Get pointer to queue for a given channel number.
 *
 * Arguments:   Channel name.
 *
 * Returns:     Pointer to queu if successful, NULL if unsuccessful
 *
>*/

PUBLIC PSQ_PrimQuInfo *
PSQ_getQuInfo(String chnlName)
{
    PSQ_PrimQuInfo *quInfo;
    PSQ_PrimQuInfo *retVal;

    retVal = (PSQ_PrimQuInfo *)0;

    for (quInfo = QU_FIRST(&activePrimQu); 
         ! QU_EQUAL(quInfo, &activePrimQu);
	 quInfo = QU_NEXT(quInfo)) {

	if (strcmp(chnlName, quInfo->chnlName) == 0 ) {
	    retVal = quInfo;
	    break;
	}
    }

    return ( retVal );
}



/*<
 * Function:    psq_procPubQu
 *
 * Description: Process action primitive on the queue (QueueAddRequest).
 *
 * Arguments:   Pointer to action struct, size of data (not used).
 *
 * Returns:     0 on successful completion, otherwise a negative error value.
 *
>*/

static  Int
psq_procPubQu(Ptr data, Int size)
{
    SP_Action *spsPrim;

    spsPrim = (SP_Action *)data;
    
    switch (spsPrim->type) {
    case SP_QUADDREQ:
	PSQ_primQuAdd(spsPrim);
	break;
    default:
	EH_problem("PSQ_procPubQu: Invalid primitive type");
        return -2;
    }
    return 0;
}


/*<
 * Function:    PSQ_procActionQu
 *
 * Description: Process action on the Queue (LOPQ, Transport, or Network layer
 *              primitive)
 *
 * Arguments:   Primitive message, size.
 *
 * Returns:     0 on succesful completion, otherwise a negative error value.
 *
>*/

Int
PSQ_procActionQu(IMQ_PrimMsg *data, Int size, PSQ_PrimQuInfo *quInfo)
{
    /* All Action Primitives have an event as the first field fo them */
    Int type;
    PSQ_QuRegPrvdr *quRegPrvdr;

    type = ((SP_Action *)data)->type;
	
    for (quRegPrvdr  = QU_FIRST(&quRegPrvdrSeq); 
     	 ! QU_EQUAL(quRegPrvdr, &quRegPrvdrSeq); 
	 quRegPrvdr  = QU_NEXT(quRegPrvdr)) {

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

/*   
    if ( type > LOPQ_A_BASE  && type < BAD_ACTION) {
	LOPS_userIn((LOPQ_Primitive *)data, size);
    } else if ( type > TS_A_BASE  && type < LOPQ_A_BASE) {
	 NOTYET TPS_userIn(data, size); 
    } else if ( type > NS_A_BASE  && type < TS_A_BASE) {
	printf("Network Primitive\n");
    } else {
	EH_problem("PSQ_procActionQu: Invalid action primitive");
        return -2;
    }
*/

    return 0;
}
    

/*<
 * Function:    PSQ_primQuInit
 *
 * Description: Initialize primitive queue.
 *
 * Arguments:   Number of queues.
 *
 * Returns:     0 on successful completion, otherwise a negative error value.
 *
>*/

LOCAL Int
PSQ_primQuInit(Int nuOfQus)
{
    Int i;
    PSQ_PrimQuInfo *quInfo;

    QU_INIT(&availPrimQu);
    QU_INIT(&activePrimQu);
    for (i = 0; i < nuOfQus; i++) {
	if ((quInfo = (PSQ_PrimQuInfo *) SF_memGet(sizeof(*quInfo))) == (PSQ_PrimQuInfo *)0) {
	    EH_problem("PSQ_primQuInit: Out of memory");
            return -2;
	}
	QU_INIT(quInfo);
	QU_INSERT(quInfo, &availPrimQu);
    }
    return 0;
}



/*<
 * Function:    PSQ_primQuAdd
 *
 * Description: Add action to primitive queue.
 *
 * Arguments:   Action primitive.
 *
 * Returns:     0 on successful completion, otherwise a negative error value.
 *
>*/

PUBLIC Int   
PSQ_primQuAdd(SP_Action *spsPrim)
{
    SP_Action      addCnf;
    SP_QuAddReq	   *quAddReq;
    PSQ_PrimQuInfo *quInfo;
    IMQ_Key actionQuKey;
    IMQ_Key eventQuKey;

#ifdef TM_ENABLED
    static char taskNameAct[OS_MAX_FILENAME_LEN + 15] = "Action Queue: ";
#endif

    quAddReq = &(spsPrim->un.quAddReq);

    if (QU_EQUAL(quInfo = QU_FIRST(&availPrimQu), &availPrimQu)) {
	EH_problem("PSQ_primQuAdd: Out of Prim Qus");
	return ( FAIL );  /* change! */
    }
    
    strcpy(quInfo->chnlName, quAddReq->chnlName);

    actionQuKey      = IMQ_keyCreate(quAddReq->chnlName, 0);
    quInfo->actionQu = IMQ_primGet(actionQuKey);
    eventQuKey       = IMQ_keyCreate(quAddReq->chnlName, 1);
    quInfo->eventQu  = IMQ_primGet(eventQuKey);

    QU_REMOVE(quInfo);
    QU_INSERT(quInfo, &activePrimQu);

#ifdef TM_ENABLED
    strcpy(taskNameAct, "Action Queue: ");
    SCH_submit((Void (*)(void)) psq_schedActionQu, (Ptr) quInfo, 
		(SCH_Event) quInfo->actionQu,
		(String) strcat(taskNameAct, TM_here()));
#else
    SCH_submit((Void (*)(void)) psq_schedActionQu, (Ptr) quInfo, 
		(SCH_Event) quInfo->actionQu);
#endif

    addCnf.type 	      = SP_QUADDCNF;
    addCnf.un.quAddCnf.status = SUCCESS;

    IMQ_primSnd(quInfo->eventQu, (IMQ_PrimMsg *)&addCnf, 
		SP_getSize(SP_QUADDCNF));    /* change! */

    return ( SUCCESS );   /* change! */
}


/*<
 * Function:    PSQ_primQuRemove
 *
 * Description: Remove primitive queue.
 *
 * Arguments:   Primitive queue.
 *
 * Returns:     0 on successful completion, -1 otherwise.
 *
>*/

PUBLIC SuccFail
PSQ_primQuRemove(PSQ_PrimQuInfo *quInfo)
{
    close(quInfo->actionQu);
    close(quInfo->eventQu);
    shutdown(quInfo->actionQu, 2);
    shutdown(quInfo->eventQu,  2);
#if 0
    unlink(IMQ_keyCreate(quInfo->chnlName, 0));
    unlink(IMQ_keyCreate(quInfo->chnlName, 1));
#endif

    QU_REMOVE(quInfo);
    QU_INSERT(quInfo, &availPrimQu);
    return ( SUCCESS );
}

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

    QU_INSERT (&quRegPrvdrSeq, quRegPrvdr);
    
    return (SUCCESS);
}
