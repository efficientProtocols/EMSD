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


#ifndef __MTSUA_H__
#define	__MTSUA_H__

#include "estd.h"
#include "queue.h"
#include "emsdmail.h"
#include "emsd.h"

/* Maxinum number of segments supported by this MTA */
#define	MTSUA_MAX_SEGMENTS			32

/* Maximum size of a PDU if using Connectionless ESROS services */
#define	MTSUA_MAX_IPM_LEN_CONNECTIONLESS	1500

/* Minimum length of a PDU which requires use of Connection-oriented ESROS */
#define	MTSUA_MIN_IPM_LEN_CONNECTIONORIENTED	((OS_Uint32)MTSUA_MAX_SEGMENTS * MTSUA_MAX_IPM_LEN_CONNECTIONLESS)

/* Maximum length that a formatted deliver arg can be */
#define	MTSUA_MAX_FORMATTED_DELIVER_ARG_LEN	100  /* over-estimate */


typedef struct
{
    QU_ELEMENT;

    char 		recipient[1];
} MTSUA_Recipient;


typedef struct
{
    char 		dataFileName[OS_MAX_FILENAME_LEN];
    char		originator[MAILTMPLEN];
    char		messageId[MAILTMPLEN];
    OS_Uint16 		retries;
    OS_Uint32 	        nextRetryTime;
    QU_Head	 	recipList; /* queue of MTSUA_Recipient */
} MTSUA_ControlFileData;


/*
 * Types of protocols we handle, in the EMSD protocols.
 */
typedef enum
{
    MTSUA_Protocol_DeliverArg,
    MTSUA_Protocol_SubmitArg,
    MTSUA_Protocol_SubmitResult,
    MTSUA_Protocol_DeliveryControlArg,
    MTSUA_Protocol_DeliveryControlRes,
    MTSUA_Protocol_DeliveryVerifyArg,
    MTSUA_Protocol_DeliveryVerifyRes,
    MTSUA_Protocol_SubmissionVerifyArg,
    MTSUA_Protocol_SubmissionVerifyRes,

    MTSUA_Protocol_ErrorRequest,

    MTSUA_Protocol_Ipm,
    MTSUA_Protocol_ReportDelivery
} MTSUA_ProtocolType;


/*
 * ESROS SAP identifiers for the EMSD protocol.
 */
typedef enum
{
    MTSUA_Rsap_Mts_Invoker_3Way			= 2,
    MTSUA_Rsap_Mts_Invoker_2Way			= 6,
    MTSUA_Rsap_Mts_Performer_3Way		= 5,
    MTSUA_Rsap_Mts_Performer_2Way		= 9,

    MTSUA_Rsap_Ua_Invoker_3Way			= 4,
    MTSUA_Rsap_Ua_Invoker_2Way			= 8,
    MTSUA_Rsap_Ua_Performer_3Way		= 3,
    MTSUA_Rsap_Ua_Performer_2Way		= 7,


    /*
     * We really need eight more SAPs if we want to implement a Module
     * Management Agent in each of the MTS and the UA.  However, RSAP values 0
     * and 1 are reserved, and there are only four bits allocated to the RSAP.
     * So, there are only six SAPs remaining.
     *
     * For now, we'll just implement this for the UA.  Later, the proper
     * solution is to use a different UDP port for management, which provides
     * the full 16 (or 14 if we still reserve 0 and 1) RSAP values.
     */
    MTSUA_Rsap_Ua_MMAgentInvoker		= 10,
    MTSUA_Rsap_Ua_MMAgentPerformer		= 13,

    MTSUA_Rsap_Ua_MMManagerPerformer		= 11,
    MTSUA_Rsap_Ua_MMManagerInvoker		= 12,
} MTSUA_Saps;


/*
 * Status of message
 */
typedef enum
{
    MTSUA_Status_Success,
    MTSUA_Status_TransientError,
    MTSUA_Status_PermanentError
} MTSUA_Status;



void
mtsua_osFree(void * p);

void
mtsua_getPrintableRfc822Address(char *     pBuf,
				ADDRESS *  pAddr);

ReturnCode
mtsua_readControlFile(char *                  pControlFileName,
		      MTSUA_ControlFileData * pControlFileData);

ReturnCode
mtsua_deleteControlAndDataFiles(char *                   pControlFileName,
				MTSUA_ControlFileData *  pControlFileData);

ReturnCode
mtsua_writeControlFile(char *                   pTempControlFileName,
		       MTSUA_ControlFileData *  pControlFileData);

void
mtsua_freeControlFileData(MTSUA_ControlFileData * pControlFileData);


ReturnCode
mtsua_initCompileTrees(char * pErrorBuf);

QU_Head *
mtsua_getProtocolTree(MTSUA_ProtocolType  protocol,
		      OS_Uint8            protocolVersionMajor,
		      OS_Uint8            protocolVersionMinor);

ReturnCode
mtsua_allocMessage(EMSD_Message ** ppMessage);

void
mtsua_freeMessage(EMSD_Message * pMessage);

ReturnCode
mtsua_allocSubmitArg(EMSD_SubmitArgument ** ppSubmit);

void
mtsua_freeSubmitArg(EMSD_SubmitArgument * pSubmit);

ReturnCode
mtsua_allocSubmitRes(EMSD_SubmitResult ** ppSubmitResult);

void
mtsua_freeSubmitRes(EMSD_SubmitResult * pSubmitResult);

ReturnCode
mtsua_allocDeliverArg(EMSD_DeliverArgument ** ppDeliver);

void
mtsua_freeDeliverArg(EMSD_DeliverArgument * pDeliver);

ReturnCode
mtsua_allocDeliveryControlArg(EMSD_DeliveryControlArgument **ppDeliveryControl);

void
mtsua_freeDeliveryControlArg(EMSD_DeliveryControlArgument * pDeliveryControl);

ReturnCode
mtsua_allocDeliveryControlRes(EMSD_DeliveryControlResult ** ppDeliveryControl);

void
mtsua_freeDeliveryControlRes(EMSD_DeliveryControlResult * pDeliveryControl);

ReturnCode
mtsua_allocReportDelivery(EMSD_ReportDelivery ** ppReportDelivery);

void
mtsua_freeReportDelivery(EMSD_ReportDelivery * pReportDelivery);

ReturnCode
mtsua_allocSubmissionVerifyArg(EMSD_SubmissionVerifyArgument ** ppSubmitVerify);

void
mtsua_freeSubmissionVerifyArg(EMSD_SubmissionVerifyArgument * pSubmitVerify);

ReturnCode
mtsua_allocSubmissionVerifyRes(EMSD_SubmissionVerifyResult ** ppSubmitVerify);

void
mtsua_freeSubmissionVerifyRes(EMSD_SubmissionVerifyResult * pSubmitVerify);

ReturnCode
mtsua_allocDeliveryVerifyArg(EMSD_DeliveryVerifyArgument ** ppDeliveryVerify);

void
mtsua_freeDeliveryVerifyArg(EMSD_DeliveryVerifyArgument * pDeliveryVerify);

ReturnCode
mtsua_allocDeliveryVerifyRes(EMSD_DeliveryVerifyResult ** ppDeliveryVerify);

void
mtsua_freeDeliveryVerifyRes(EMSD_DeliveryVerifyResult * pDeliveryVerify);

ReturnCode
mtsua_allocErrorRequest(EMSD_ErrorRequest ** ppErrorRequest);

void
mtsua_freeErrorRequest(EMSD_ErrorRequest * pErrorRequest);

ReturnCode
mtsua_convertEmsdIpmToRfc822(EMSD_Message * pIpm,
			    ENVELOPE * pRfc822Env,
			    char * pExtraHeaders,
			    char * pLocalHostName);

ReturnCode
mtsua_convertEmsdDeliverToRfc822(EMSD_DeliverArgument * pDeliver,
				ENVELOPE * pRfc822Env,
				char * pExtraHeaders,
				char * pLocalHostName);

char *
mtsua_getRfc822Date(OS_Uint32 julianDate,
		    char * pBuf);

void
mtsua_setLookupEmsdAddress(ReturnCode (* pfLookup)(char * p,
						  OS_Uint16 len,
						  OS_Uint32 * pEmsdAddress,
						  ADDRESS * pAddr));

void
mtsua_setLookupRfc822Address(ReturnCode (* pfLookup)(char * pBuf,
						     OS_Uint32 bufLen));

void
mtsua_createRfc822AddrFromEmsd(EMSD_ORAddress * pORAddr,
			      char * pLocalDomainAndHost,
			      char * pBuf,
			      OS_Uint32 bufLen);

ReturnCode
mtsua_getEmsdAddrFromRfc822(char * pRfc822Addr,
			   OS_Uint32 * pEmsdAddress,
			   ADDRESS * pAddr);

#endif /* __MTSUA_H__ */
