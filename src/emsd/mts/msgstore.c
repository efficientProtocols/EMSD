/*
 * 
 * Copyright (C) 1997-2002  Neda Communications, Inc.
 * 
 * This file is part of Neda-EMSD. An implementation of RFC-2524.
 * 
 * Neda-EMSD is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (GPL) as 
 * published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 * 
 * Neda-EMSD is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Neda-EMSD; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.  
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

static char *rcs = "$Id: msgstore.c,v 1.1.1.1 2002/10/24 19:50:03 mohsen Exp $";

#include "estd.h"
#include "config.h"
#include "tm.h"
#include "eh.h"
#include "addr.h"
#include "emsd.h"
#include "mts.h"
#include "mm.h"
#include "subpro.h"
#include "msgstore.h"

#ifdef TM_ENABLED
static void *	hTM;
#endif

static struct DQ_DeliveryQueue  deliveryQueue      = { 0, 0, 0, 0, 0, NULL, NULL };
static OS_Boolean               dumpDeliveryQueue  = FALSE;

static struct DQ_RecipQueue  *DQ_getRecipQueue (OS_Uint32  emsdAddress);
static ReturnCode             DQ_addMessage (char *pMsgId, char *originator, 
					     struct DQ_RecipQueue *pRecipDQ, const char *pDataFile);
static ReturnCode             msg_cleanQueue (void);
static OS_Boolean             emsdNsapEqual (const N_SapAddr *n1, const N_SapAddr *n2);

static void msg_sigUSR2 (int unused);


/*
 * msg_init (char *dirName)
 *
 */
ReturnCode
msg_init ()
{
    static OS_Boolean   initialized     = FALSE;

    if (initialized == TRUE) {
	return Success;
    }
    initialized = TRUE;

    sigset (SIGUSR2, msg_sigUSR2);

    /* set up the module's trace structure */
    if (TM_OPEN(hTM, "MSG_") == NULL) {
	EH_problem ("Could not init the MSG_ module's trace structure\n");
	return Fail;
    }
    TM_TRACE((hTM, MSG_TRACE_ACTIVITY, "msg_init()"));

    /*** read the files in to populate the MTA with undelivered messages */
    /*** not yet ***/
    
    return Success;
}

/*
 * msg_addMessage
 *
 * Adds a message to the  delivery queue.
 */
ReturnCode
msg_addMessage (char *pMsgId, char *pOriginator, QU_Head recipHead, const char *pDataFile)
{
    ReturnCode             rc                = Success;
    OS_Uint32              deliveryCount     = 0;
    OS_Uint32              emsdAddr           = 0;
    char                  *pAddr             = NULL;
    struct DQ_RecipQueue  *pRecipDQ          = NULL;
    MTSUA_Recipient       *pRecip            = NULL;
    MTSUA_Recipient       *pRecipFirst       = NULL;
    char                   dirPath[1024];

    static OS_Uint32       msgSeq            = 0;

    TM_TRACE((hTM, MSG_TRACE_DETAIL, "msg_addMessage(%s) entered", pMsgId));

    /* find the recipient queues */
    /* For each recipient... */
    for (pRecip = QU_FIRST(&recipHead); !QU_EQUAL(pRecip, &recipHead); pRecip = QU_NEXT(pRecip)) {

        if (pRecipFirst == NULL) {
	  pRecipFirst = pRecip;
	} else {
	  if (pRecipFirst == pRecip) {
	    TM_TRACE((hTM, MSG_TRACE_WARNING, "Loop detected, breaking out"));
	    break;
	  }
        }
/***	TM_TRACE((hTM, MSG_TRACE_DETAIL, "### %08x  %08x", pRecip, &recipHead)); ***/

	pAddr = pRecip->recipient;

	if ((emsdAddr = msg_rfc822ToEmsdAddr (pAddr)) == 0) {
	    TM_TRACE((hTM, MSG_TRACE_DETAIL, "Skipping RFC 822 address '%s'", pAddr));
	    continue;
	}
        TM_TRACE((hTM, MSG_TRACE_DETAIL, "Recipient = %lu", emsdAddr));

	deliveryQueue.count++;
	/* get the queue for this recipient */
	pRecipDQ = DQ_getRecipQueue (emsdAddr);
	if (pRecipDQ == NULL) {
	    TM_TRACE((hTM, MSG_TRACE_WARNING,
		      "Could not find nor create Recipient Delivery Queue for %lu", emsdAddr));
	    deliveryQueue.failureCount++;
	    continue;
	}
	
	/* copy the data file to the recipient's data directory */
	sprintf (dirPath, "%s/recipData/%lu/%08lx-%s",
		 globals.config.deliver.pSpoolDir, emsdAddr, msgSeq++, pMsgId);

	rc = link (pDataFile, dirPath);
	if (rc) {
	  perror (dirPath);
	}

	/* we have a queue to add this entry to */
	if (DQ_addMessage (pMsgId, pOriginator, pRecipDQ, dirPath)) {
	    TM_TRACE((hTM, MSG_TRACE_DETAIL,
		      "DQ_addMessage(%s, %lu, %s) failed", pMsgId, pRecipDQ->recipient, dirPath));
	} else {
	    deliveryCount++;

	    if (deliveryCount > 32) {
	      TM_TRACE((hTM, MSG_TRACE_DETAIL, "Too many recipients"));
	      break;
	    }
	}
    }
    /* Ckeck to see that at least one delivery was queued */
    if (deliveryCount == 0) {
	TM_TRACE((hTM, MSG_TRACE_DETAIL, "No messages were queued for delivery for %s", pMsgId));
    } else {
	TM_TRACE((hTM, MSG_TRACE_DETAIL,
		  "%d instances were queued for delivery for %s", deliveryCount, pMsgId));
	deliveryQueue.totalCount += deliveryCount;
    }
    return (deliveryCount > 0 ? Success : Fail);
}


/*
 * DQ_getRecipQueue
 *
 * Returns the recipient DQ structure, creating one if need be.
 * Recipients are not removed from the queue when the queue is empty.
 */
static struct DQ_RecipQueue *
DQ_getRecipQueue (OS_Uint32  emsdAddress)
{
    struct DQ_RecipQueue  *pRecip           = NULL;
    SUB_Profile           *pSubPro          = NULL;
    char                   emsdAddrStr[16];
    char                   dirPath[512];

    TM_TRACE((hTM, MSG_TRACE_DETAIL, "DQ_getRecipQueue(%lu) entered", emsdAddress));

    /* cycle through the recipient queues */
    for (pRecip = deliveryQueue.pRecipFirst; pRecip; pRecip = pRecip->next) {
	if (pRecip->recipient == emsdAddress)
	    break;
    }
    if (pRecip == NULL) {
        TM_TRACE((hTM, MSG_TRACE_ACTIVITY, "Creating Recipient queue for emsdAddress %lu", emsdAddress));

	/* make sure the subscriber exists */
	sprintf (emsdAddrStr, "%lu", emsdAddress);
	pSubPro = SUB_getEntry (emsdAddrStr, NULL);
	if (pSubPro == NULL) {
	  TM_TRACE((hTM, MSG_TRACE_ERROR, "No subscriber '%s', cannot get delivery queue", emsdAddrStr));
	  return (NULL);
	}

	/* create a recipient entry */
	pRecip                = calloc (1, sizeof (struct DQ_RecipQueue));
	pRecip->recipient     = emsdAddress;
	pRecip->status        = RQ_EMPTY;
	pRecip->nextAttempt   = time(NULL);  /* try ASAP */

	/* make a copy of the device profile */
	memcpy (&pRecip->deviceProfile, &pSubPro->deviceProfile, sizeof pRecip->deviceProfile);

	/* add the entry to the recipient queue */
	pRecip->next          = deliveryQueue.pRecipFirst;
	deliveryQueue.pRecipFirst = pRecip;

	if (pRecip->next)
	    pRecip->next->prev = pRecip;

	if (deliveryQueue.pRecipLast == NULL)
	    deliveryQueue.pRecipLast = pRecip;

	/* make sure there are control and data directories for this recipient */
	sprintf (dirPath, "%s/recipData/%lu", globals.config.deliver.pSpoolDir, emsdAddress);
	if (mkdir (dirPath, 0777)) {
	    if (errno != EEXIST) {
	        TM_TRACE((hTM, MSG_TRACE_ERROR, "Cannot create data directory: '%s'", dirPath));
	    }
	}
	sprintf (dirPath, "%s/recipControl/%lu", globals.config.deliver.pSpoolDir, emsdAddress);
	if (mkdir (dirPath, 0777)) {
	    if (errno != EEXIST) {
	        TM_TRACE((hTM, MSG_TRACE_ERROR, "Cannot create control directory: '%s'", dirPath));
	    }
	}

    }
    return (pRecip);
}

/*
 * DQ_addMessage
 *
 * Adds a message to the delivery queue.
 */
ReturnCode
DQ_addMessage (char *pMsgId, char *pOriginator, struct DQ_RecipQueue *pRecipDQ, const char *pDataFile)
{
    time_t                   now     = 0;
    struct DQ_ControlData   *pMsg    = NULL;

    TM_TRACE((hTM, MSG_TRACE_DETAIL, "DQ_addMessage '%s', '%s', '%s'",
	      pMsgId, NULLOK(pOriginator), NULLOK(pDataFile)));

    now = time (NULL);

    /* link in the entry */
    if (pRecipDQ->pMsg == NULL) {
	pRecipDQ->pMsg = calloc (1, sizeof (struct DQ_ControlData));
	pMsg = pRecipDQ->pMsg;
    } else {
        /*** might get fancier for priority recognition ***/
	pMsg = pRecipDQ->pMsg;
	while (pMsg->next)
	    pMsg = pMsg->next;
	pMsg->next = calloc (1, sizeof (struct DQ_ControlData));
	pMsg = pMsg->next;
    }

    /* initialize pMsg, assuming currectly all zeros */
    strncpy (pMsg->messageId, pMsgId, sizeof pMsg->messageId - 1);
    pMsg->status = MQ_READY;
    pMsg->submitted = now;
    pMsg->expiryTime = now + (1 * 60 * 60);    /* expire in one hour */
    pMsg->pRecipQueue = pRecipDQ;
    strncpy (pMsg->dataFileName, NULLOK(pDataFile), sizeof pMsg->dataFileName - 1);
    strncpy (pMsg->originator, NULLOK(pOriginator), sizeof pMsg->originator - 1);

    pRecipDQ->count++;

    /* set the state of the queue */
    switch (pRecipDQ->status) {
    case RQ_EMPTY:
	pRecipDQ->status = RQ_READY;
	TM_TRACE((hTM, MSG_TRACE_DETAIL,
		  "Added a message to a queue in state EMPTY [%lu]", pRecipDQ->recipient));
	break;
    case RQ_DELIVERING:
	TM_TRACE((hTM, MSG_TRACE_DETAIL,
		  "Added a message to a queue in state DELIVERING [%lu]", pRecipDQ->recipient));
	break;
    case RQ_WAITING:
	TM_TRACE((hTM, MSG_TRACE_DETAIL,
		  "Added a message to a queue in state WAITING [%lu], pRecipDQ->recipient", pRecipDQ->recipient));
	break;
    case RQ_READY:
	TM_TRACE((hTM, MSG_TRACE_DETAIL,
		  "There are now %ld messages in the delivery queue [%lu]", pRecipDQ->count, pRecipDQ->recipient));
	break;
    }

    /* set up the nextAttempt values, so the message will be sent */
    if ((pMsg->pRecipQueue->nextAttempt == 0)
     || (pMsg->pRecipQueue->nextAttempt > now + 30)) {
      pMsg->pRecipQueue->nextAttempt = now;
    }
    if ((deliveryQueue.nextAttempt == 0)
     || (deliveryQueue.nextAttempt > pMsg->pRecipQueue->nextAttempt)) {
      deliveryQueue.nextAttempt = pMsg->pRecipQueue->nextAttempt;
    }

    /*** create control and data files for recovery ***/
    
    return Success;
}

/*
 * get a message from one of the delivery queues, for delivery
 */
struct DQ_ControlData *
msg_getMessage (void)
{
    struct DQ_ControlData  *pMsg       = NULL;
    struct DQ_RecipQueue   *pRecip     = NULL;
    time_t                  now        = 0;

    msg_cleanQueue();

    now = time(NULL);

    /* check for work at the top level */
    if (deliveryQueue.nextAttempt > now) {
	TM_TRACE((hTM, MSG_TRACE_POLL, "Delivery's Next Attempt time not reached"));
	return (NULL);
    }

    /* cycle through the recipient queues */
    for (pRecip = deliveryQueue.pRecipFirst; pRecip; pRecip = pRecip->next) {

	if (pRecip->status != RQ_READY)
	    continue;

	if (pRecip->nextAttempt > now)
	    continue;

	if (pRecip->nextAttempt > now)
	    continue;

	/* cycle through the message queues */
	for (pMsg = pRecip->pMsg; pMsg; pMsg = pMsg->next) {
	    if ((pMsg->status == MQ_READY) || (pMsg->status == MQ_FAILED))
		break;
	}
	if (pMsg) {
	    /* we found a good queue, mark it as in delivery */
	    pMsg->status = MQ_DELIVERING;
	    pMsg->lastAttempt = now;
	    pMsg->pRecipQueue->lastAttempt = now;
	    pMsg->pRecipQueue->status = RQ_DELIVERING;

	    /*** move recip to the end of the queue, to be fair and quick ***/

	    break;
	}
    }
    deliveryQueue.lastAttempt = now;
    deliveryQueue.nextAttempt = now + (pMsg ? 0 : 2);

    return (pMsg);
}


/*
 * Delete a message from the delivery queue
 */
ReturnCode
msg_deleteMessage (struct DQ_ControlData *pMsg)
{
    int sysRc   = 0;

    if (pMsg->completed == 0) {
	pMsg->status = MQ_FAILED;
	pMsg->completed = time(NULL);

	pMsg->pRecipQueue->failureTotal++;
	pMsg->pRecipQueue->count--;

	/*** delete the data and control files ***/
	sysRc = unlink (pMsg->dataFileName);
	if (sysRc) {
	  TM_TRACE((hTM, MSG_TRACE_ACTIVITY, 
		    "Error deleting data file: '%s' [%d]", pMsg->dataFileName, sysRc));
	}
    }
    return Success;
}
    
/*
 * Set the status of a message
 */
ReturnCode
msg_updateMessageStatus (struct DQ_ControlData *pMsg, OS_Uint32 status)
{
    int     sysRc     = 0;

    TM_TRACE((hTM, MSG_TRACE_ACTIVITY, "Setting state of message %lu/%s from 0x%X to 0x%X",
	      pMsg->pRecipQueue->recipient, pMsg->messageId, pMsg->status, status));

    if (pMsg->completed) {
        TM_TRACE((hTM, MSG_TRACE_WARNING, "Attempt to set state of completed message rejected"));
        return Fail;
    }

    pMsg->status = status;

    /* update counters */
    switch (status) {
    case MQ_UNDEFINED:
	break;
    case MQ_READY:
      pMsg->pRecipQueue->status = RQ_READY;
      break;
    case MQ_DELIVERING:
      pMsg->pRecipQueue->status = RQ_DELIVERING;
      break;
    case MQ_SUCCEDED:
      pMsg->completed = time(NULL);
      pMsg->pRecipQueue->successTotal++;
      pMsg->pRecipQueue->failureCount = 0;
      pMsg->pRecipQueue->count--;
      pMsg->pRecipQueue->lastAttempt = pMsg->completed;
      pMsg->pRecipQueue->nextAttempt = pMsg->completed;
      pMsg->pRecipQueue->status = RQ_READY;

      /*** delete the data and control files ***/
      sysRc = unlink (pMsg->dataFileName);
      if (sysRc) {
	TM_TRACE((hTM, MSG_TRACE_ACTIVITY, 
		  "Error deleting data file: '%s' [%d]", pMsg->dataFileName, sysRc));
      }
      break;
    case MQ_FAILED:
      pMsg->pRecipQueue->status = RQ_READY;
      break;
    default:
      TM_TRACE((hTM, MSG_TRACE_WARNING, "Invalid status for message queue [%d]", status));
      break;
    }

    return Success;
}


/*** remove expired and deleted entries ***/
static ReturnCode
msg_cleanQueue (void)
{
    struct DQ_ControlData  *pMsg         = NULL;
    struct DQ_ControlData  *pFreeMe      = NULL;
    struct DQ_RecipQueue   *pRecip       = NULL;
    SUB_DeviceProfile      *pDev         = NULL;
    OS_Boolean              deleteFlag   = FALSE;
    time_t                  now          = 0;
    static time_t           lastClean    = 0;

    /*
     *  Note that the queue counts did not consider completed entries, they have
     *  already been adjusted.
     */

    /* do not clean too often */
    now = time (NULL);
    if ((now - lastClean < 60) && (dumpDeliveryQueue == FALSE)) {
	return Success;
    }
    lastClean = now;

    if (dumpDeliveryQueue == TRUE) {
      TM_TRACE((hTM, MSG_TRACE_DUMP, "=============== Delivery Queue Dump ==============="));
      TM_TRACE((hTM, MSG_TRACE_DUMP, "Dumping message queue: Next attempt in %ds",
		(deliveryQueue.nextAttempt - now < 9999) ? deliveryQueue.nextAttempt - now : 9999));
    }
    /* cycle through the recipient queues */
    for (pRecip = deliveryQueue.pRecipFirst; pRecip; pRecip = pRecip->next) {
        pDev = &pRecip->deviceProfile;

	if (dumpDeliveryQueue == TRUE) {
	    TM_TRACE((hTM, MSG_TRACE_DUMP, "Recipient Queue: %8d %03d.%03d.%03d.%03d status=%s inQ=%3d last=%4d next=%4d",
		      pRecip->recipient, pDev->nsap.addr[0], pDev->nsap.addr[1],
		      pDev->nsap.addr[2], pDev->nsap.addr[3],
		      pRecip->status   == RQ_READY      ? "Ready" :
		        pRecip->status == RQ_DELIVERING ? "Delivering" :
		        pRecip->status == RQ_EMPTY      ? "Empty" :
		        pRecip->status == RQ_WAITING    ? "Waiting" : "Unknown",
		      pRecip->count,
		      (now - pRecip->lastAttempt < 9999) ? now - pRecip->lastAttempt : 9999,
		      (pRecip->nextAttempt - now < 9999) ? pRecip->nextAttempt - now : 9999));
	}
	/* cycle through the message queues */
	pFreeMe = NULL;
	for (pMsg = pRecip->pMsg; pMsg; pMsg = pMsg->next) {

	    deleteFlag = FALSE;

	    if (pFreeMe)		/* delete element from previous iteration */
		free (pFreeMe);
	    pFreeMe = NULL;

	    if (dumpDeliveryQueue == TRUE) {
		TM_TRACE((hTM, MSG_TRACE_DUMP, ">   Message Queue: status=%s '%s' %s %s",
			  pMsg->status   == MQ_READY      ? "Ready" :
			    pMsg->status == MQ_DELIVERING ? "Delivering" :
			    pMsg->status == MQ_FAILED     ? "Failed" :
			    pMsg->status == MQ_SUCCEDED  ? "Succeded" : "Unknown",
			  pMsg->messageId,
			  pMsg->completed ? "Completed" : "",
			  now > pMsg->expiryTime ? "Expired" : ""));
	    }

	    if (pMsg->completed && (now - pMsg->completed > 15 * 60)) {
		deleteFlag = TRUE;
	    }
	    else if (now > pMsg->expiryTime) {
		TM_TRACE((hTM, MSG_TRACE_ACTIVITY, "Expiring message '%s'", pMsg->messageId));
		mts_deliveryNonDel (pMsg, 0);
		deleteFlag = TRUE;
	    }
	    if (deleteFlag == TRUE) {
		TM_TRACE((hTM, MSG_TRACE_ACTIVITY, "Cleaning up message '%s'", pMsg->messageId));

		if (pMsg->next) {
		    pMsg->next->prev = pMsg->prev;
		}
		if (pMsg->prev) {
		    pMsg->prev->next = pMsg->next;
		} else {
		  /* this is the first element, update the pRecipe queue */
		  if (pRecip->pMsg == pMsg) {
		    pRecip->pMsg = pMsg->next;
		  } else {
		    TM_TRACE((hTM, MSG_TRACE_ERROR, "Queue error '%s'", pMsg->messageId));
		  }
		}
		pFreeMe = pMsg;
	    }
	}
	if (pFreeMe) 
	    free (pFreeMe);
	pFreeMe = NULL;

    }
    dumpDeliveryQueue = FALSE;

    return Success;
}

void
msg_sigUSR2 (int unused)
{
    dumpDeliveryQueue = TRUE;
}


OS_Uint32
msg_rfc822ToEmsdAddr (const char *pRfc822Addr)
{
  OS_Uint32    numFields        = 0;
  OS_Uint32    emsdAddr          = 0;
  OS_Uint32    i1, i2, i3;

  if (pRfc822Addr == NULL) {
    TM_TRACE((hTM, MSG_TRACE_WARNING, "msg_rfc822ToEmsdAddr(NULL), no match"));
    return (0);
  }

#if CHECK_DOMAIN
  {
    OS_Boolean   domainValidFlag  = FALSE;
    char        *pDomain          = NULL;

    /* Make sure that the domain matches the assigned emsd domain */
    if (pDomain = strrchr (pRfc822Addr, '@')) {
      if (!strcmp (pDomain, globals.config.pLocalHostName)) {
	domainValidFlag = TRUE;
      }
    }
    if (domainValidFlag == FALSE) {
      TM_TRACE((hTM, MSG_TRACE_WARNING, "msg_rfc822ToEmsdAddr(%s), domain mismatch", pRfc822Addr));
      return (0);
    }
  }
#endif /* CHECK_DOMAIN */
      
  numFields = sscanf (pRfc822Addr, "%lu.%lu.%lu", &i1, &i2, &i3);

  switch (numFields) {
  case 1:
    emsdAddr = i1;
    break;
  case 2:
    emsdAddr = (i1 * 1000) + i2;
    break;
  }

  TM_TRACE((hTM, MSG_TRACE_DETAIL, "msg_rfc822ToEmsdAddr: '%s' = %lu", NULLOK(pRfc822Addr), emsdAddr));

  return (emsdAddr);
}

ReturnCode
msg_deliverNow (N_SapAddr *pNsap)
{
  struct DQ_RecipQueue   *pRecip     = NULL;
  time_t                  now        = 0;

  now = time(NULL);

    /* cycle through the recipient queues */
  for (pRecip = deliveryQueue.pRecipFirst; pRecip; pRecip = pRecip->next) {
    if (pRecip->count > 0) {
      if (emsdNsapEqual (&pRecip->deviceProfile.nsap, pNsap) == TRUE) {
	TM_TRACE((hTM, MSG_TRACE_DETAIL, "Request delivery for %lu", pRecip->recipient));
	pRecip->nextAttempt = now;
      }
    }
  }
  return Success;
}

static OS_Boolean
emsdNsapEqual (const N_SapAddr *n1, const N_SapAddr *n2)
{
  if (n1 == NULL || n2 == NULL)
    return (FALSE);

  if ((n1->addr[0]  == n2->addr[0])
   && (n1->addr[1]  == n2->addr[1])
   && (n1->addr[2]  == n2->addr[2])
   && (n1->addr[3]  == n2->addr[3])) {
    return (TRUE);
  }
  return (FALSE);
}

