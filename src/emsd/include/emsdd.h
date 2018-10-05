/*
 *
 *Copyright (C) 1997-2002  Neda Communications, Inc.
 *
 *This file is part of Neda-EMSD. An implementation of RFC-2524.
 *
 *Neda-EMSD is free software; you can redistribute it and/or modify it
 *under the terms of the GNU General Public License (GPL) as 
 *published by the Free Software Foundation; either version 2,
 *or (at your option) any later version.
 *
 *Neda-EMSD is distributed in the hope that it will be useful, but WITHOUT
 *ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with Neda-EMSD; see the file COPYING.  If not, write to
 *the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *Boston, MA 02111-1307, USA.  
 *
*/
/*
 * 
 * Available Other Licenses -- Usage Of This Software In Non Free Environments:
 * 
 * License for uses of this software with GPL-incompatible software
 * (e.g., proprietary, non Free) can be obtained from Neda Communications, Inc.
 * Visit http://www.neda.com/ for more information.
 * 
 */

/*
 * 
 * History:
 *
 */

#ifndef __EMSDD_H__
#define	__EMSDD_H__

#include "queue.h"
#include "emsdmail.h"
#include "emsd.h"
#include "esro.h"
#include "tm.h"

/* Time to sleep between scanning directory */
#define	EMSDD_SLEEP_TIME				10

/* Time to wait for ALL segments of a segmented message to arrive */
#define	EMSDD_SEGMENTATION_TIMER			10

/* Maximum size of a PDU if using Connectionless ESROS services */
#define	EMSDD_MAX_IPM_LEN_CONNECTIONLESS		1500

/* Maximum size of a PDU if using Connectionless ESROS services */
#define	EMSDD_MAX_SEGMENTS			32

/* Minimum length of a PDU which requires use of Connection-oriented ESROS */
#define	EMSDD_MIN_IPM_LEN_CONNECTIONORIENTED	(EMSDD_MAX_SEGMENTS * EMSDD_MAX_IPM_LEN_CONNECTIONLESS)

/* Maximum length that a formatted deliver arg can be */
#define	EMSDD_MAX_FORMATTED_DELIVER_ARG_LEN	100  /* over-estimate */

/* Maximum number of message send retries that can be run-time configured */
#define	EMSDD_MAX_RETRIES			50


/*
 * Trace Features
 */
#define	EMSDD_TRACE_MESSAGE_FLOW	TM_BIT0
#define	EMSDD_TRACE_INIT		TM_BIT1
#define	EMSDD_TRACE_ADDRESS	TM_BIT2
#define	EMSDD_TRACE_PDU		TM_BIT8

typedef enum
{
    EMSDD_Status_Success		= 0,
    EMSDD_Status_TransientError	= 1,
    EMSDD_Status_PermanentError	= 2
} EMSDD_Status;


typedef struct
{
    QU_HEAD;
} EMSDD_RecipList;

typedef struct
{
    QU_ELEMENT;

    EMSDD_Status	    status;
    OS_Uint16	    protocolVersionMajor;
    OS_Uint16	    protocolVersionMinor;
    char *	    pErrorString;
    char	    recipient[1];
} EMSDD_Recipient;

typedef struct
{
    ESRO_SapSel	    remoteRsapSelector;
    T_SapSel	    remoteTsapSelector;
    N_SapAddr	    remoteNsapAddress;
} EMSDD_NetworkAddress;

typedef enum
{
    EMSDD_Rsap_Emsdd_Invoker		= 2,
    EMSDD_Rsap_Emsdd_Performer		= 5,

    EMSDD_Rsap_Etwpd_Invoker		= 4,
    EMSDD_Rsap_Etwpd_Performer	= 3
} EMSDD_Saps;

typedef struct
{
    OS_Uint16 		retries;
    OS_Uint32 		nextRetryTime;
    char 		dataFileName[OS_MAX_FILENAME_LEN];
    char		originator[MAILTMPLEN];
    EMSDD_RecipList 	recipList;
} EMSDD_ControlFileData;


typedef struct
{
    OS_Uint32 		    sequenceId;
    OS_Uint32 		    lastSegmentNumber;
    OS_Uint32 		    nextExpectedSegmentNumber;
    EMSD_Operations	    operationType;
    union
    {
	EMSD_SubmitArgument *	pSubmitArg;
	EMSD_DeliverArgument * 	pDeliverArg;
    } 			    operation;
    void *		    hTimer;
    void * 		    hBuf;
} EMSDD_PendingSegments;

extern TM_ModuleCB *	emsdd_hModCB;


ReturnCode
emsdd_convertRfc822ToEmsdContent(ENVELOPE * pRfc822Env,
			       char * pRfc822Body,
			       char *pHeaderStr,
			       void ** ppContent,
			       EMSD_ContentType * pContentType);

ReturnCode
emsdd_convertRfc822ToEmsdSubmit(ENVELOPE * pRfc822Env,
			      char * pRfc822Body,
			      EMSD_SubmitArgument * pSubmit);

ReturnCode
emsdd_convertRfc822ToEmsdDeliver(ENVELOPE * pRfc822Env,
			       char * pRfc822Body,
			       EMSD_DeliverArgument * pDeliver,
			       EMSD_ContentType contentType);

ReturnCode
emsdd_convertEmsdIpmToRfc822(EMSD_Message * pIpm,
			   ENVELOPE * pRfc822Env,
			   char * pExtraHeaders);

ReturnCode
emsdd_convertEmsdDeliverToRfc822(EMSD_DeliverArgument * pDeliver,
			       ENVELOPE * pRfc822Env,
			       char * pExtraHeaders);

char *
emsdd_getRfc822Date(OS_Uint32 julianDate,
		   char * pBuf);

void
emsdd_setLocalDomainAndHost(char * pLocalDomainAndHost);

void
emsdd_getLocalDomainAndHost(char ** ppLocalDomainAndHost);

ReturnCode
emsdd_allocMessage(EMSD_Message ** ppMessage);

void
emsdd_freeMessage(EMSD_Message * pMessage);

ReturnCode
emsdd_allocSubmitArg(EMSD_SubmitArgument ** ppSubmit);

void
emsdd_freeSubmitArg(EMSD_SubmitArgument * pSubmit);

ReturnCode
emsdd_allocSubmitRes(EMSD_SubmitResult ** ppSubmitResult);

void
emsdd_freeSubmitRes(EMSD_SubmitResult * pSubmitResult);

ReturnCode
emsdd_allocDeliverArg(EMSD_DeliverArgument ** ppDeliver);

void
emsdd_freeDeliverArg(EMSD_DeliverArgument * pDeliver);

ReturnCode
emsdd_allocDeliveryControlArg(EMSD_DeliveryControlArgument **
			     ppDeliveryControl);

void
emsdd_freeDeliveryControlArg(EMSD_DeliveryControlArgument *
			    pDeliveryControl);

ReturnCode
emsdd_allocReportDelivery(EMSD_ReportDelivery ** ppReportDelivery);

void
emsdd_freeReportDelivery(EMSD_ReportDelivery * pReportDelivery);

void
emsdd_setSleepTime(OS_Uint16 sleepTime);

ReturnCode
emsdd_registerClient(char * pNewMessageDirectory,
		    char * pQueueDirectory,
		    ReturnCode (* pfSend)(char * pMessageFileName,
					  EMSDD_ControlFileData * pcfData),
		    void (* pfNonDeliver)(char * pControlFileName,
					  char * pMessageFileName,
					  char * pOriginator,
					  EMSDD_RecipList *pRecipList));

void
emsdd_setLookupEmsdAddress(ReturnCode (* pfLookup)(char * p,
						 OS_Uint16 len,
						 OS_Uint32 * pEmsdAddress,
						 ADDRESS * pAddr));

void
emsdd_setLookupRfc822Address(ReturnCode (* pfLookup)(char * pBuf,
						    OS_Uint32 bufLen));


ReturnCode
emsdd_mainLoop(void);

ReturnCode
emsdd_createMessageId(char * pLocalDomainAndHost,
		     OS_Uint32 * pSubmissionTime,
		     OS_Uint32 * pDailyMessageNumber,
		     char * pRfc822MessageId);

void
emsdd_createRfc822MessageIdFromEmsd(EMSD_MessageId * pMsgId,
				  char * pLocalDomainAndHost,
				  char * pBuf);

void
emsdd_createRfc822AddrFromEmsd(EMSD_ORAddress * pORAddr,
			     char * pLocalDomainAndHost,
			     char * pBuf,
			     OS_Uint32 bufLen);

void
emsdd_getRfc822AddrFromEmsd(EMSD_ORAddress * pORAddr);

ReturnCode
emsdd_getEmsdAddrFromRfc822(char * pRfc822Addr,
			  OS_Uint32 * pEmsdAddress,
			  ADDRESS * pAddr);

void
emsdd_getPrintableRfc822Address(char * pBuf,
			       ADDRESS * pAddr);

char *
emsdd_copyCreateRfc822String(void * pSrc,
			    OS_Uint16 len);

char *
emsdd_copyString(char *pString);

ReturnCode
emsdd_addRetryTime(OS_Uint32 seconds);

ReturnCode
emsdd_readControlFile(char * pControlFileName,
		     EMSDD_ControlFileData * pControlFileData);

ReturnCode
emsdd_writeControlFile(char * pControlFileName,
		      EMSDD_ControlFileData * pControlFileData);

void
emsdd_freeControlFileData(EMSDD_ControlFileData * pControlFileData);

void
emsdd_freeRecipient(EMSDD_Recipient * pRecip);

ReturnCode
emsdd_listenerInit(void ** phListener);

void
emsdd_listenerTerminate(void * hListener);

ReturnCode
emsdd_segment(void * pArg,
	     void * hContents,
	     void * pRecipList,
	     void * pSegInfo,
	     OS_Uint16 maxLenConnectionless,
	     OS_Uint16 minLenConnectionOriented,
	     OS_Uint16 maxFormattedArgLen,
	     ReturnCode (* pfFormatAndInvoke)(void * pArg,
					      void * hContents,
					      void * pRecipList));

ReturnCode
emsdd_reassemble(EMSD_Operations operationType,
		void ** ppArg,
		void * hListener,
		OS_Uint32 reassemblyTimer,
		OS_Boolean * pbComplete);


#endif /* __EMSDD_H__ */
