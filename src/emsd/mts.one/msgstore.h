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
 * Available Other Licenses -- Usage Of This Software In Non Free Environments:
 * 
 * License for uses of this software with GPL-incompatible software
 * (e.g., proprietary, non Free) can be obtained from Neda Communications, Inc.
 * Visit http://www.neda.com/ for more information.
 * 
 */

/*
 * 
 * History:  Written 96.11.18
 */

#ifndef __MSGSTORE_H__
#define __MSGSTORE_H__

/* TM Trace Flags */
#define	MSG_TRACE_ERROR   	(1 << 0)
#define	MSG_TRACE_WARNING   	(1 << 1)
#define	MSG_TRACE_DETAIL   	(1 << 2)
#define	MSG_TRACE_ACTIVITY	(1 << 3)
#define	MSG_TRACE_INIT		(1 << 4)
#define	MSG_TRACE_DUMP		(1 << 8)
#define	MSG_TRACE_POLL		(1 << 12)
#define	MSG_TRACE_MSG		(1 << 15)

#undef  NULLOK
#define NULLOK(x)   (x ? x : "NULL")

struct MsgId {
    char                *pRfc822;
    EMSD_LocalMessageId   local;
};

struct MessageData {
  char *msg_header;
  char *msg_body;
};

struct DQ_DeliveryQueue {
    OS_Uint32             count;	/* Number of messages currently in queues */
    OS_Uint32             totalCount;	/* Number of messages added to queues */
    OS_Uint32             successCount;	/* Number of messages delivered */
    OS_Uint32             failureCount;	/* Number of delivery attempts which failed */
    time_t                lastAttempt;	/* Seconds past 00:00 GMT Jan 1, 1970 */
    time_t                nextAttempt;
    struct DQ_RecipQueue *pRecipFirst;	/* to efficiently move first to last... */
    struct DQ_RecipQueue *pRecipLast;	/* ...so delivery is round-robin */
};

struct DQ_ControlData {
    OS_Uint32              status;
#       define  MQ_UNDEFINED  0
#       define  MQ_READY      1
#       define  MQ_DELIVERING 2
#       define  MQ_FAILED     3
#       define  MQ_SUCCEDED   4
    char                   messageId[128];
    char                   dataFileName[OS_MAX_FILENAME_LEN];
    char                   originator[128];  /* sender of the message */
    OS_Uint8               priority;    /* higher is better */
    OS_Uint32              failureCount; /* Number of delivery attempts which failed */
    time_t                 submitted;	/* Seconds past 00:00 GMT Jan 1, 1970 */
    time_t                 completed;   /* Either delivered or failed at this time */
    time_t                 lastAttempt;
    time_t                 expiryTime;	/* Seconds past 00:00 GMT Jan 1, 1970 */
    struct DQ_ControlData  *next;	/* Next message */
    struct DQ_ControlData  *prev;	/* Previous message */
    struct DQ_RecipQueue   *pRecipQueue;
};

struct DQ_RecipQueue {
    OS_Uint32             recipient; 	/* can also be a BCD string */
    OS_Uint32             status;	/* EMPTY, READY, DELIVERING, WAITING */
#       define  RQ_UNDEFINED  0
#       define  RQ_EMPTY      1
#       define  RQ_READY      2
#       define  RQ_DELIVERING 3
#       define  RQ_WAITING    4
    OS_Uint32             count;	/* Number of messages currently in queue */
    OS_Uint32             totalCount;	/* Total number of messages added to queue */
    OS_Uint32             successTotal;	/* Number of messages delivered */
    OS_Uint32             failureTotal;	/* Number of deliveries which failed */
    OS_Uint32             SuccessCount;	/* Number of deliveries since failure */
    OS_Uint32             failureCount;	/* Number of deliveries which failed, reset on success */
    OS_Uint32             dropCount;	/* Number of messages were not delivered */
    time_t                lastAttempt;	/* Seconds past 00:00 GMT Jan 1, 1970 */
    time_t                nextAttempt;
    time_t                lastDelivery;
    struct DQ_ControlData *pMsg;	/* List of messages to deliver */

    OS_Uint32             submitCount;  /* Number of messages submittted by this recipient */
    time_t                lastSubmit;   /* Last time a submission was received */

    SUB_DeviceProfile     deviceProfile;

    struct DQ_RecipQueue  *prev;
    struct DQ_RecipQueue  *next;
};
  

ReturnCode              msg_init             (void);
ReturnCode              msg_addMessage       (char *pMsgId, char *pOrig, QU_Head recipHead, const char *pDataFile);
ReturnCode              msg_deleteMessage    (struct DQ_ControlData *pControl);
ReturnCode              msg_updateMessage    (struct DQ_ControlData *pControl, OS_Uint32 status);
ReturnCode              msg_deliverNow       (N_SapAddr *pNsap);
struct DQ_ControlData  *msg_getMessage       (void);
OS_Uint32               msg_rfc822ToEmsdAddr  (const char *pRfc822Addr);

#endif  /* __MSGSTORE_H__ */
