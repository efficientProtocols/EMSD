/*
 *  
 *  Copyright (C) 1995-1999  Neda Communications, Inc. All rights reserved.
 * 
 *  This software is furnished under a license and use, duplication,
 *  disclosure and all other uses are restricted to the rights specified
 *  in the written license between the licensee and copyright holders.
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
 *   PSQ_procPubQu(Ptr data, Int size)
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
static char rcs[] = "$Id: psq.c,v 1.22 1999/09/30 00:04:35 mohsen Exp $";
#endif /*}*/

#include  "eh.h"
#include  "estd.h"
#include  "oe.h"
#include  "tm.h"
#include  "sf.h"
#include  "imq.h"
#include  "psq.h"
#include  "sp_shell.h"
#include "sch.h"
#include  "target.h"
#include  "seq.h"

extern int errno;

int psq_connectActionQu( PSQ_PrimQuInfo *quInfo);
int psq_connectEventQu ( PSQ_PrimQuInfo *quInfo);

#define PUB_QU_NAME "/tmp/SP"  	/* The Global Service Provider Queue */
char pubQuName[512] = PUB_QU_NAME;	/* The Global Service Provider Queue */


typedef struct PrimQuHead {
    PSQ_PrimQuInfo *first;
    PSQ_PrimQuInfo *last;
} PrimQuHead;

static PrimQuHead availPrimQu;
LOCAL  PrimQuHead activePrimQu;

STATIC PSQ_QuRegPrvdrSeq quRegPrvdrSeq;
STATIC SEQ_PoolDesc   quRegPrvdrPool;

Int pubSockDesc;

Int psq_schedPubQu();
Int lops_cleanSap();
 

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
    Int PSQ_origMask;

    static Bool virgin = TRUE;

#ifdef TM_ENABLED
    static char taskNamePub[100] = "Public Queue: ";
#endif

    if (!virgin) {
	return;
    }
    virgin = FALSE;

    /* DJL Changed sigMask = sigmask(SIGALRM); to: */
    sigMask = sigmask(SIGALRM) | sigmask(SIGUSR1) | sigmask(SIGUSR2);

    /* Inititialize the Inter Module Queue */
    IMQ_init(IMQ_QUEUES);

    quRegPrvdrPool = SEQ_poolCreate(sizeof(*quRegPrvdrSeq.first), 0);
    QU_INIT(&quRegPrvdrSeq);
    
    /* Create Socket from which to read */
    PSQ_origMask    = sigblock(sigMask);
    pubSockDesc = socket(AF_UNIX, SOCK_DGRAM, 0);
    sigsetmask(PSQ_origMask);
    if (pubSockDesc < 0) {
	perror("PSQ_init: opening datagram socket failed");
	exit(1);
    }

    if (unlink(pubQuName) == SUCCESS) {	/* In case it was left from last time */
    	TM_TRACE((IMQ_modCB, TM_ENTER, "PSQ_init: Removing Left Over File"));
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

    PSQ_origMask = sigblock(sigMask);
    if (bind(pubSockDesc, (struct sockaddr *) &name, nameLength) < 0) {
	EH_unixProblem("PSQ_init: Unable to bind");
	perror("PSQ_init: binding name to datagram socket");
	EH_fatal("PSQ_init: bind failed");
    }
    sigsetmask(PSQ_origMask);
/*******/
{
    Int statusFlags = 0;
    if ((statusFlags = fcntl(pubSockDesc, F_GETFL, 0)) < 0) {
 	perror ("fcntl: ");
	EH_problem("IMQ_primGet NEW: fcntl failed");
        IMQ_primDelete(pubSockDesc, NULL);
#if 0
        shutdown((int)pubSockDesc, 2);
#endif
	return;  /* return error_value */
    }

    statusFlags |= FNDELAY;
    if (fcntl (pubSockDesc, F_SETFL, statusFlags) < 0) {
	perror ("fcntl: ");
	EH_problem("IMQ_primGet NEW: fcntl failed");
        IMQ_primDelete(pubSockDesc, NULL);
#if 0
        shutdown((int)pubSockDesc, 2);
#endif
	return;  /* return error_value */
    }
}
/*******/

    /* Initialize Primitive Qu Tables */
    PSQ_primQuInit(50);

    IMQ_blockQuAdd(pubSockDesc);

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
    static Byte bigMsg[SP_getSize(SP_QUADDREQ)+100];
    IMQ_PrimMsg *msgPtr = (IMQ_PrimMsg *) bigMsg;
    Int size;

#ifdef TM_ENABLED
    static char taskNamePub[100] = "Public Queue: ";
#endif

    /* Process Public Qu */
    if ((size = IMQ_primRcv(*pubSockDescript, msgPtr, 
			    SP_getSize(SP_QUADDREQ))) 
        > 0) {
	    /* SYSV check for ENOMSG */
	    PSQ_procPubQu((Ptr) msgPtr, size);
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
    static char taskNameAct[100] = "Action Queue: ";
#endif

    /* action primitive Queue */
    if ((size = IMQ_primRcv(quInfo->actionQu, msgPtr, sizeof(bigMsg))) != -2) {
		/* SYSV check for ENOMSG */
	switch (size) {
	case 0:
	    break;
	case -1:
#ifdef TM_ENABLED
	strcpy(taskNameAct, "Action Queue: ");
    	SCH_submit((Void (*)(void)) psq_schedActionQu, (Ptr) quInfo, 
		   (SCH_Event) quInfo->actionQu,
		   (String) strcat(taskNameAct, TM_here()));
#else
    	SCH_submit((Void (*)(void)) psq_schedActionQu, (Ptr) quInfo, 
		   (SCH_Event) quInfo->actionQu);
#endif
	    return 0;

	default:
	    if (PSQ_procActionQu(msgPtr, size, quInfo) == -2) {
		return 0;
	    }
	}


#ifdef TM_ENABLED
	strcpy(taskNameAct, "Action Queue: ");
    	SCH_submit((Void (*)(void)) psq_schedActionQu, (Ptr) quInfo, 
		   (SCH_Event) SCH_PSEUDO_EVENT,
		   (String) strcat(taskNameAct, TM_here()));
#else
    	SCH_submit((Void (*)(void)) psq_schedActionQu, (Ptr) quInfo, 
		   (SCH_Event) SCH_PSEUDO_EVENT);
#endif
    } else {

	PSQ_QuRegPrvdr *quRegPrvdr;
    	for (quRegPrvdr = (PSQ_QuRegPrvdr *)quRegPrvdrSeq.first; 
	     quRegPrvdr != (PSQ_QuRegPrvdr *) &quRegPrvdrSeq ; 
	     quRegPrvdr = quRegPrvdr->next) {

	    if (quRegPrvdr->cleanSap) {
	        (*(quRegPrvdr->cleanSap))(quInfo);
	    }
	}
		
/*	IMQ_blockQuRemove(quInfo->actionQu); */

	PSQ_primQuRemove(quInfo);

        QU_move(quInfo, &availPrimQu);			/* check this */
	EH_problem("psq_schedActionQu: IMQ_primRcv failed "
		   "(means user process is not running anymore)");
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
          < 0 || retVal != size) {
        TM_TRACE((IMQ_modCB, TM_ENTER, "PSQ_putEvent: IMQ_primSnd failed"));
	return (FAIL);          
    } else {
        return retVal;        
    }

} /* PSQ_putEvent() */


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

    for (quInfo = activePrimQu.first; 
         quInfo != (PSQ_PrimQuInfo *)&activePrimQu;
	 quInfo = quInfo->next) {

	if (strcmp(chnlName, quInfo->chnlName) == 0 ) {
	    retVal = quInfo;
	    break;
	}
    }

    return ( retVal );

} /* PSQ_getQuInfo() */



/*<
 * Function:    PSQ_procPubQu
 *
 * Description: Process action primitive on the queue (QueueAddRequest).
 *
 * Arguments:   Pointer to action struct, size of data (not used).
 *
 * Returns:     0 on successful completion, otherwise a negative error value.
 *
>*/

/* STATIC  NOTYET */
PUBLIC  Int
PSQ_procPubQu(Ptr data, Int size)
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

} /* PSQ_procPubQu() */


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
    Int retVal = 0;
    PSQ_QuRegPrvdr *quRegPrvdr;

    type = ((SP_Action *)data)->type;
	
    for (quRegPrvdr  = (PSQ_QuRegPrvdr *) quRegPrvdrSeq.first; 
     	 quRegPrvdr != (PSQ_QuRegPrvdr *) &quRegPrvdrSeq ; 
	 quRegPrvdr  = quRegPrvdr->next) {

    	if (type  >  quRegPrvdr->lowerEnd  && type  <  quRegPrvdr->upperEnd )
	{
	    if ( quRegPrvdr->procAction) {
	        retVal = (*quRegPrvdr->procAction)(data, size);
	    } else {
		EH_problem("PSQ_procActionQu: Range found for action "
			   "primitive, but no function is associated\n");
    		TM_TRACE((IMQ_modCB, TM_ENTER, 
			 "                     action code=%d\n", type));
    		TM_TRACE((IMQ_modCB, TM_ENTER, 
			 "                     Registered range=%d-%d\n", 
    			 quRegPrvdr->lowerEnd, quRegPrvdr->upperEnd));

		retVal = -1;		
	    }

	    break;
	}
    }

    if (quRegPrvdr == (PSQ_QuRegPrvdr *) &quRegPrvdrSeq) {
	EH_problem("PSQ_procActionQu: Invalid action primitive");
    	TM_TRACE((IMQ_modCB, TM_ENTER, 
		 "                     action code=%d\n", type));
        return -1;
    }	

/*  OBSOLETE: Kept just FYI
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

    return retVal;

} /* PSQ_procActionQu() */
    

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
	if ((quInfo = (PSQ_PrimQuInfo *) SF_memGet(sizeof(*quInfo))) 
	    == (PSQ_PrimQuInfo *)0) {
	    EH_problem("PSQ_primQuInit: Out of memory");
            return -2;
	}
	QU_INIT(quInfo);
	QU_INSERT(quInfo, &availPrimQu);
    }
    return 0;

} /* PSQ_primQuInit() */


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
    SP_QuAddReq	   *quAddReq;
    PSQ_PrimQuInfo *quInfo;
    IMQ_Key actionQuKey;

#ifdef TM_ENABLED
    static char taskNameAct[100] = "Action Queue: ";
#endif

    quAddReq = &(spsPrim->un.quAddReq);


    if ((quInfo = availPrimQu.first) == (PSQ_PrimQuInfo *) &availPrimQu) {
	EH_problem("PSQ_primQuAdd: Out of Prim Qus");
	return ( FAIL );  /* change! */
    }
    
    strcpy(quInfo->chnlName, quAddReq->chnlName);

    actionQuKey      = IMQ_keyCreate(quAddReq->chnlName, 0);
    if ((quInfo->actionQu = IMQ_acceptConn(actionQuKey)) == 0) { /* errno*/
	return (FAIL); /* change to err_val */
    }

#ifdef TM_ENABLED
    strcpy(taskNameAct, "Connect action queue: ");
    SCH_submit((Void (*)(void)) psq_connectActionQu, (Ptr) quInfo, 
		(SCH_Event) quInfo->actionQu,
		(String) strcat(taskNameAct, TM_here()));
#else
    SCH_submit((Void (*)(void)) psq_connectActionQu, (Ptr) quInfo, 
		(SCH_Event) quInfo->actionQu);
#endif

    return (SUCCESS);

} /* PSQ_primQuAdd() */


/*<
 * Function:    psq_connectActionQu
 *
 * Description: Connect action queue.
 *
 * Arguments:	Queue info
 *
 * Returns: 
 *
>*/

int
psq_connectActionQu( PSQ_PrimQuInfo *quInfo)
{
    int retVal;
    IMQ_Key eventQuKey;

#ifdef TM_ENABLED
    static char taskNameAct[100];
#endif

    QU_move(quInfo, &activePrimQu);

    if ((retVal = IMQ_connect(quInfo->actionQu)) == 0) {
    	TM_TRACE((IMQ_modCB, TM_ENTER, 
		 "psq_conncectActionQu: IMQ_connect failed"));
	return (FAIL); 		/* socket is already closed */
    }

    quInfo->actionQu = retVal;

    eventQuKey       = IMQ_keyCreate(quInfo->chnlName, 1);
    if ((quInfo->eventQu  = IMQ_acceptConn(eventQuKey)) == 0) { /* errVal*/
	return (FAIL); /* change to err_val */
    }

#ifdef TM_ENABLED
    strcpy(taskNameAct, "Establish connection: ");
    SCH_submit((Void (*)(void)) psq_connectEventQu, (Ptr) quInfo, 
		(SCH_Event) quInfo->eventQu,
		(String) strcat(taskNameAct, TM_here()));
#else
    SCH_submit((Void (*)(void)) psq_connectEventQu, (Ptr) quInfo, 
		(SCH_Event) quInfo->eventQu);
#endif
    return (SUCCESS);

} /* psq_connectActionQu() */



/*<
 * Function:    psq_connectEventQu
 *
 * Description: Connect event queue
 *
 * Arguments:	Queue info
 *
 * Returns: 
 *
>*/

int
psq_connectEventQu( PSQ_PrimQuInfo *quInfo)
{
    int retVal;
    SP_Action      addCnf;

#ifdef TM_ENABLED
    static char taskNameAct[100];
#endif

    if ((retVal = IMQ_connect(quInfo->eventQu)) == 0) {
    	TM_TRACE((IMQ_modCB, TM_ENTER, 
		 "psq_conncectEventQu: IMQ_connect failed"));
	return (FAIL); 		/* socket is already closed */ 
    }
    quInfo->eventQu = retVal;

    addCnf.type 	      = SP_QUADDCNF;
    addCnf.un.quAddCnf.status = SUCCESS;

    if (IMQ_primSnd(quInfo->eventQu, (IMQ_PrimMsg *)&addCnf, 
		    SP_getSize(SP_QUADDCNF)) == -2) { /* change! */
    	TM_TRACE((IMQ_modCB, TM_ENTER, 
		 "psq_conncectEventQu: IMQ_primSnd failed"));
	return (FAIL);
    }

#ifdef TM_ENABLED
	strcpy(taskNameAct, "Action Queue: ");
    	SCH_submit((Void (*)(void)) psq_schedActionQu, (Ptr) quInfo, 
		   (SCH_Event) quInfo->actionQu,
		   (String) strcat(taskNameAct, TM_here()));
#else
    	SCH_submit((Void (*)(void)) psq_schedActionQu, (Ptr) quInfo, 
		   (SCH_Event) quInfo->actionQu);
#endif

    return ( SUCCESS );   

} /* psq_connectEventQu() */


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
    IMQ_primDelete(quInfo->actionQu, quInfo->chnlName);
    IMQ_primDelete(quInfo->eventQu,  quInfo->chnlName);

    QU_move(quInfo, &availPrimQu);

    TM_TRACE((IMQ_modCB, TM_ENTER, 
	     "PSQ_primQuRemove: Action queue %d (%s) and "
	     "Event queue %d (%s) removed"));

    return ( SUCCESS );

} /* PSQ_primQuRemove() */


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

    QU_INSERT(&quRegPrvdrSeq, quRegPrvdr);
    
    return (SUCCESS);

} /* PSQ_register() */
