/*
 *  Copyright (C) 1995,1999  Neda Communications, Inc. All rights reserved.
 * 
 *  This software is furnished under a license and use, duplication,
 *  disclosure and all other uses are restricted to the rights specified
 *  in the written license between the licensee and copyright holders.
 * 
 */

/*+
 * File name: udp_if.c  (UDP InterFace for connectionless service)
 *
 * Description: UDP InterFace for connectionless service
 *
 * Function:
 *
 *   inet_getSockaddr_in (...)
 *   udp_getSapInfo(T_SapSel *sapSel)
 *   UDP_dataReq(...)
 *   udp_receive(UDP_SapDesc locSapDesc)
 *   udp_sapInit(Int nuOfSaps)
 *   UDP_sapUnbind(T_SapSel *sapSel)
 *   UDP_sapBind(...)
 *   UDP_init(Int)
 *
-*/

/*
 * Author: Mohsen Banan.
 * History:
 *
 */

#ifdef RCS_VER	/*{*/
static char rcs[] = "$Id: udp_if.c,v 1.31 2002/10/25 19:37:53 mohsen Exp $";
#endif /*}*/

#include "estd.h"	/* Definition of standard types                      */
#include "oe.h"		/* Operating Environment specifications              */
#include "byteordr.h"	/* Byte order facilities: get/put                    */
#include "tm.h"		/* Trace Module                                      */
#include "eh.h"		/* Exception Handler                                 */
#include "du.h"		/* Data Unit (data structures and manipul. functions */
#include "addr.h"	/* SAP data structures                               */
#include "inetaddr.h"	/* extern definition of address translation functions*/
#include "sf.h"		/* System Facilities: queue manipuation functions    */
#include "sch.h"	/* Scheduler                                         */
#include "sap.h"	/* extern definition of SAP manipulation functions   */
#include "nm.h"		/* Network Management                                */
#include "udp_if.h"	/* UDP interface                                     */
#include "target.h"	/* target machine specific definitions               */
#include "local.h"	/* Module's local definitions                        */

#ifdef PDU_LOG
extern Int LOG_logit(Int code, char *dp, Int siz);
#define IN_PDU    	1
#define OUT_PDU   	2
#endif

#ifndef MSDOS
extern int errno;
#endif

#if defined(MSDOS) || defined(WINDOWS)
int sigstub(int i){return 0;}
#define sigblock sigstub
#define sigsetmask sigstub
#define sigmask sigstub
#ifndef SIGALRM
# define SIGALRM 0
#endif
#endif

/* Network management interface */

#define NC_NULSENT 1 /* NOTYET */
#define NC_NULRCVD 2 /* NOTYET */
#define ESRO_SAP   2 /* NOTYET */

Counter erop_udpSduSentCounter       = {0, NC_NULSENT, 0, (Counter *) NULL};
Counter udpSdu_rcvd                  = {0, NC_NULRCVD, 0, (Counter *) NULL};
Counter udp_pdu_bad                  = {0,         -1, 0, (Counter *) NULL};
Counter erop_udpPduDiscNOTYETCounter = {0,         -1, 0, (Counter *) NULL};

struct nm_info Ninfo[] = {
			  {&erop_udpSduSentCounter,       NM_READ | NM_WRITE},
			  {&udpSdu_rcvd,                  NM_READ | NM_WRITE},
			  {&udp_pdu_bad,                  NM_READ | NM_WRITE},
			  {&erop_udpPduDiscNOTYETCounter, NM_READ | NM_WRITE},
			  {(Counter *) NULL,              0}
};

/*
 *  Information required for support of communication
 *  with multiple NSUs through the UDPsap abstraction.
 */
typedef struct udp_SapQuHead {
    QU_HEAD;
} udp_SapQuHead;

typedef struct udp_SapInfo {
    QU_ELEMENT;

    T_SapSel		*sapSel;	/* UDPsap-ID for this user 	  */
    Int			sockFd;		/* Socket descriptor		  */
    int			(*dataInd)();	/* T_UNIT_DATA.indication handler */
} udp_SapInfo;

extern DU_Pool *G_duMainPool;

static udp_SapQuHead availSapQu;
LOCAL  udp_SapQuHead activeSapQu;

TM_ModuleCB *UDP_modCB;

/* 
 * Set the bit corresponding to each signal that should be 
 * blocked for delivery.
 */
static int sigMask;


/*<
 * Function:    inet_getSockaddr_in
 *
 * Description: Set the Transport SAP selector and Network SAP address in 
 *              socket structure.
 *
 * Arguments:   Socket structure (outgoing argument)
 *              Transport SAP selector
 *              Network SAP address
 *
 * Returns:     None.
 *
>*/

STATIC Void
inet_getSockaddr_in (struct sockaddr_in *sockAddr,	/* OUT */
		     T_SapSel *tsapSel,
		     N_SapAddr *nsapAddr)
{
    Int portNu;
#ifdef FUTURE
    LgUns ipAddr;
#endif

    OS_memSet((char *) sockAddr, '\0', sizeof(*sockAddr));
    
    sockAddr->sin_family = AF_INET;
    INET_nsapAddrToin_addr(nsapAddr, &(sockAddr->sin_addr));

    INET_tsapSelToPortNu(tsapSel, &portNu);
    sockAddr->sin_port = htons(portNu);
}



/*<
 * Function:    udp_getSapInfo
 *
 * Description: Gets a Transport SAP selector and returns the UDP SAP info. 
 *
 * Arguments:   Transport SAP selector
 *
 * Returns:     UDP SAP info
 *
>*/

LOCAL struct udp_SapInfo *
udp_getSapInfo(T_SapSel *sapSel)
{
    udp_SapInfo *sap;

    for (sap = QU_FIRST(&activeSapQu);
	 ! QU_EQUAL(sap, &activeSapQu);
	 sap = QU_NEXT(sap)) {

	if ( SAP_selCmp((SAP_Sel *)sap->sapSel, (SAP_Sel *)sapSel) == 0 ) {
	    return (sap);
	} 
    }
    return ((udp_SapInfo *) 0);
}

/*
 * Function:    UDP_dataReq
 *
 * Description: UDP_DATA.request Primitive. Sends UDP data unit.
 *
 * Arguments:   Local UDP SAP descriptor
 *              Transport SAP selector of the remote peer
 *              Network SAP address of the remote peer
 *
 * Returns:     0 if successful, -1 if unsuccessful.
 *
 */

SuccFail
UDP_dataReq(UDP_SapDesc locSapDesc,
	    T_SapSel *remTsapSel,
	    N_SapAddr *remNsapAddr,
	    DU_View udpSdu)
{
    struct sockaddr_in destAddr;
    udp_SapInfo *udp_sapInfo;

    Int sendtoRetVal;

#ifdef TM_ENABLED
    {
	Int portNu;
	struct in_addr remIpAddr;

	INET_nsapAddrToin_addr(remNsapAddr, &remIpAddr);
	INET_tsapSelToPortNu(remTsapSel, &portNu);
#if !defined(MSDOS) && !defined(WINDOWS) /* NOTYET */
	TM_TRACE((UDP_modCB, TM_ENTER, 
		 "UDP_dataReq:          RemPortNu=%d, RemIpAddr=%s\n",
		 portNu, inet_ntoa(remIpAddr)));
#endif
    }
#endif

    if (!udpSdu) {
	EH_problem("UDP_dataReq: Null udp passed to UDP_dataReq\n");
	return FAIL;
    }
    udp_sapInfo = (udp_SapInfo *) locSapDesc;
    
    inet_getSockaddr_in(&destAddr, remTsapSel, remNsapAddr);

    sigMask = sigblock(sigmask(SIGALRM));
    sendtoRetVal = sendto(udp_sapInfo->sockFd,
	       DU_data(udpSdu), 
	       DU_size(udpSdu),
	       0, 
	       (struct sockaddr *)&destAddr,
	       sizeof(destAddr));
    sigsetmask(sigMask);

#ifdef PDU_LOG
    LOG_logit(OUT_PDU, DU_data(udpSdu), DU_size(udpSdu));
#endif

    if (sendtoRetVal != DU_size(udpSdu)) {
	return (FAIL);
    }

    NM_incCounter(NM_UDP, &erop_udpSduSentCounter, 1);

    return ( SUCCESS );

} /* UDP_dataReq() */


/*<
 * Function:    udp_receive
 *
 * Description: Receive UDP.
 *
 * Arguments:   UDP SAP descriptor.
 *
 * Returns:     0 if successful, -1 if unsuccessful.
 *
>*/

SuccFail
udp_receive(UDP_SapDesc locSapDesc)
{
    DU_View du;
    int remSockAddrLen;
    struct sockaddr_in remSockAddr;
    udp_SapInfo *udp_sapInfo;
    T_SapSel remTsapSel;
    N_SapAddr remNsapAddr;
    Int sduLen;
    Int bufLen;
    Int fatalCnt = 0;

#ifdef TM_ENABLED
    static char taskN[100] = "Receive UDP: ";
#endif

    /* local UDP_sap must be active, UDP_sap address must be valid */

    udp_sapInfo = (udp_SapInfo *) locSapDesc;
    if ( (du = DU_alloc(G_duMainPool, MAXBFSZ)) == (DU_View) 0 ) {
	EH_problem("udp_receive: DU_alloc failed");
#ifdef TM_ENABLED
        strcpy(taskN, "Receive UDP: ");
    	return SCH_submit ((Void *)udp_receive, 
		           (UDP_SapDesc) locSapDesc, 
		           udp_sapInfo->sockFd, 
		           (String) strcat(taskN, TM_here()));
#else
    	return SCH_submit ((Void *)udp_receive, 
		           (UDP_SapDesc) locSapDesc, 
		           udp_sapInfo->sockFd);
#endif
	/* NOTYET */
	sleep(5);
	if (++fatalCnt > 7) {
	    EH_fatal("udp_receive: Too many DU_alloc failures\n");
	} else {
	    return (FAIL);
	}
    }

    TM_TRACE((DU_modCB, DU_MALLOC, 
	     "udp_receive: Pool DU_alloc: du=0x%lx\n", du));

#if 1
    remSockAddrLen = sizeof(remSockAddr);
    if ((sduLen = recvfrom(udp_sapInfo->sockFd,
			   DU_data(du),
			   DU_size(du),
			   0,
			   (struct sockaddr *) &remSockAddr,
			   &remSockAddrLen)) < 0) {
#else
    if ((sduLen = read(udp_sapInfo->sockFd, DU_data(du), DU_size(du))) < 0) {
#endif

#ifdef PDU_LOG
    LOG_logit(IN_PDU, DU_data(du), sduLen);
#endif

	DU_free(du);

        TM_TRACE((DU_modCB, DU_MALLOC, 
		 "udp_receive: recv failed: DU_free: du=0x%lx\n", du));

	return (FAIL);
    }

#ifdef PDU_LOG
    LOG_logit(IN_PDU, DU_data(du), sduLen);
#endif

    /* Adjust the size */
    bufLen = DU_size(du);
    if (bufLen < sduLen) {
	EH_problem("udp_receive: Buffer smaller than SDU size");
	DU_free(du);
        TM_TRACE((DU_modCB, DU_MALLOC, "udp_receive: DU_free: du=0x%lx\n", du));
	return ( FAIL );
    }
    DU_adjust(du, bufLen - sduLen);

    TM_TRACE((UDP_modCB, TM_ENTER, 
	     "udp_receive:          bufLen=%d, sduLen=%d, duSize=%d\n",
	     bufLen, sduLen, DU_size(du)));

    INET_in_addrToNsapAddr(&(remSockAddr.sin_addr), &remNsapAddr);
    INET_portNuToTsapSel(ntohs(remSockAddr.sin_port), /* number received as port no is always in network format. */
			&remTsapSel); 

    (*udp_sapInfo->dataInd) (&remTsapSel, 
			     &remNsapAddr, 
			     (T_SapSel *) 0 /* NOTYET */,
			     (N_SapAddr *) 0 /* NOTYET */,
			     du);

#ifdef TM_ENABLED
    strcpy(taskN, "Receive UDP: ");
    return SCH_submit ((Void *)udp_receive, 
		       (UDP_SapDesc) locSapDesc, 
		       udp_sapInfo->sockFd, 
		       (String) strcat(taskN, TM_here()));
#else
    return SCH_submit ((Void *)udp_receive, 
		       (UDP_SapDesc) locSapDesc, 
		       udp_sapInfo->sockFd);
#endif
} /* udp_receive() */


/*<
 * Function:    udp_sapInit
 *
 * Description: Initialize SAP
 *
 * Arguments:   Number of SAP's.
 *
 * Returns:     0 if successful, -1 if unsuccessful.
 *
>*/

SuccFail
udp_sapInit(Int nuOfSaps)
{
    Int i;
    udp_SapInfo *sap;

    QU_INIT(&availSapQu);
    QU_INIT(&activeSapQu);
    for (i = 0; i < nuOfSaps; i++) {
	if ((sap = (udp_SapInfo *) SF_memGet(sizeof(*sap))) == (udp_SapInfo *)0) {
	    EH_problem("udp_sapInit: Out of memory, Sap allocation");
	}
	QU_INIT(sap);
	QU_INSERT(sap, &availSapQu);
    }
    return (SUCCESS);
}


/*<
 * Function:    UDP_sapUnBind
 *
 * Description: Unbind UDP SAP.
 *
 * Arguments:   SAP selector.
 *
 * Returns:     0 if successful, negative error value if unsuccessful.
 *
>*/

PUBLIC SuccFail
UDP_sapUnbind(T_SapSel *sapSel)
{
    udp_SapInfo *sap;

    if ((sap = udp_getSapInfo(sapSel)) == (udp_SapInfo *) 0) {
	EH_problem("UDP_sapUnBind: Sap not active");
	return ( FAIL );
/*	return -2; *** err.h ***/
    }

    QU_MOVE(sap, &availSapQu);
    return ( SUCCESS );
}


/*<
 * Function:    UDP_sapBind
 *
 * Description: Bind SAP.
 *
 * Arguments:   Transport SAP selector, pointer to function that handles the
 *              the primitives.
 *
 * Returns:     0 if successful, -1 if unsuccessful.
 *
>*/
PUBLIC UDP_SapDesc
UDP_sapBind(T_SapSel *sapSel,
	    int (*dataInd) (T_SapSel *remTsapSel,
			    N_SapAddr *remNsapAddr,
			    T_SapSel *locTsapSel,
			    N_SapAddr *locNsapAddr,
			    DU_View data))
{
    udp_SapInfo *sap;
    struct sockaddr_in sockAddr;
    int sock;
    Int origMask;
#ifdef FUTURE
    struct stat buf;
    int fromLen;
    fd_set ready;
#endif
    Int retVal;
    Int portNu;
#ifdef TM_ENABLED
    static char taskN[100] = "First Receive UDP: ";
#endif

    if (QU_EQUAL((sap = QU_FIRST(&availSapQu)), &availSapQu)) {
	EH_problem("UDP_sapBind: Out of SAP");
	return ( (UDP_SapDesc) 0  );
/*	return -2; *** err.h ***/
    }

    /* If the SAP-SEL is already in use, the function fails.  */
    if (udp_getSapInfo(sapSel) != (udp_SapInfo *) 0) {
	EH_problem("UDP_sapBind: SAP in use");
	return ( (UDP_SapDesc) 0  );
/*	return -3; *** err.h ***/
    }

    /* So the SAP can be created */
    sap->sapSel  = sapSel;
    sap->dataInd = dataInd;

    /* Open a UDP Internet Socket */
    origMask = sigblock(sigMask);
    sock     = socket(AF_INET, SOCK_DGRAM, 0);
    sigsetmask(origMask);
    if (sock < 0) {
	perror("opening strean socket");
	EH_problem("UDP_sapBind: socket function failed");
	return ( (UDP_SapDesc) 0  );
/*	return -4; *** err.h ***/
    }

    /* Bind this address to it */
    
    OS_memSet((char *) &sockAddr, '\0', sizeof(sockAddr));
    
    sockAddr.sin_family      = AF_INET;
    sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    INET_tsapSelToPortNu(sapSel, &portNu);
    sockAddr.sin_port = htons(portNu);

    origMask = sigblock(sigMask);
    retVal = bind(sock, (struct sockaddr *)&sockAddr, sizeof(sockAddr));
    sigsetmask(origMask);

    if (retVal == 0) {
	Int statusFlags;

#ifndef WINDOWS
	/* We Don't want to block for IPC operations */
	if ((statusFlags = fcntl(sock, F_GETFL, 0/*NA*/)) < 0) {
	    perror("fcntl");
	    EH_problem("UDP_sapBind: fcntl failed");
	}
	statusFlags |= FNDELAY;
	if (fcntl(sock, F_SETFL, statusFlags) < 0) {
	    perror("fcntl");
	    EH_problem("UDP_sapBind: fcntl failed");
	    return ( (UDP_SapDesc) 0  );
/*	    return -5; *** err.h ***/
	}
#endif
    } else {
	/* So Bind Failed */
	perror("UDP_sapBind: binding stream socket");
	EH_problem("UDP_sapBind: bind function failed");
	return ( (UDP_SapDesc) NULL  );
/*      return -6; *** err.h ***/
    }
    sap->sockFd = sock;

    QU_MOVE(sap, &activeSapQu);

    TM_TRACE((UDP_modCB, TM_ENTER, "UDP_sapBind: SAP=0x%lx\n", sap));

    if (dataInd != 0) {
#ifdef TM_ENABLED
        strcpy(taskN, "First Receive UDP: ");
	SCH_submit ((Void *)udp_receive, (UDP_SapDesc) sap, sap->sockFd, 
		    (String) strcat(taskN, TM_here()));
#else
	SCH_submit ((Void *)udp_receive, (UDP_SapDesc) sap, sap->sockFd);
#endif
    }

    return ( (UDP_SapDesc) sap );

} /* UDP_sapBind */


/*<
 * Function:    UDP_init
 *
 * Description: Initialize the layer.
 *              Should be performer once during system initialization before
 *              any other use of the  layer services.
 *
 * Arguments:   None.
 *
 * Returns:     0 if successful, -1 if unsuccessful.
 *
>*/


SuccFail
UDP_init(int nuOfSaps)
{
    static Bool virgin = TRUE;
#ifdef WINDOWS
    WORD version;
    WSADATA sockdata;
    char *p = (char *) &version;
    *p++ = 1; *p = 1;	/* version 1.1 */
#endif
    if ( !virgin ) {
	return 0;
    }
    virgin = FALSE;

#ifdef WINDOWS
    if (WSAStartup(version, &sockdata))
	{ 
	    char errbuf[256];
	    sprintf(errbuf, "UDP_init: startup error %d", WSAGetLastError());
	    EH_problem(errbuf);
	    return(FAIL);
	}
#endif
#ifdef TM_ENABLED
    if (TM_OPEN(UDP_modCB, "UDP_") == NULL) {
    	EH_problem("UDP_init: TM_open failed");
	return ( FAIL );
    }
#endif

/*
    sigMask = sigmask(SIGALRM);
*/
    udp_sapInit(nuOfSaps); 

    return ( SUCCESS );
}


