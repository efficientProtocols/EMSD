/*
 *  This file is part of the Open C Platform (OCP) Library. OCP is a
 *  portable library for development of Data Communication Applications.
 * 
 *  Copyright (C) 1996 Neda Communications, Inc.
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

/*
 * Author: Derrell Lipman
 * History:
 *
 */

/*
**
** Static Block Memory functionality
**
**   This module allows allocation from a static block of memory.
**   Allocations are arranged in transactions, and are relocatable
**   within the static block, to maintain data integrity while
**   allowing memory garbage collection.
**
**/


#include "estd.h"
#include "eh.h"
#include "tm.h"
#include "queue.h"
#include "sbm.h"

typedef struct Header
{
    OS_Uint32		memSize;
    OS_Uint32		bytesRemaining;
    OS_Uint32		transAllocated;
} Header;

typedef struct Transaction
{
    OS_Uint32		transLen;
    OS_Uint32		offset;
    OS_Uint32 		transactionType;
} Transaction;

typedef struct TransactionInProcess
{
    Transaction *	pTrans;
    SBM_Transaction	hAddingToTrans;
    OS_Uint32		bytesRemaining;
    unsigned char *	pNextAlloc;
    unsigned char *	pStart;
} TransactionInProcess;


struct
{
    Header *		pHeader;
    unsigned char *	pStart;
    unsigned char *	pNextAvailable;
    Transaction *	pTransOffsets;
    OS_Boolean		bTransactionInProcess;
    TM_ModuleCB *	hTM;
} sbm_globals;


static OS_Boolean	bInitialized = FALSE;



/*
 * Function:	SBM_init()
 *
 * Description:	Initialize the Static Block Memory module.
 *
 * Parameters:
 *
 * Returns:
 *
 */
ReturnCode
SBM_init(void)
{
    if (bInitialized)
    {
	return Success;
    }

    if (TM_OPEN(sbm_globals.hTM, "SBM") == NULL)
    {
	return Fail;
    }

    bInitialized = TRUE;

    return Success;
}



/*
 * Function:	SBM_open()
 *
 * Description:	Prepare to use a static block of memory from which to allocate.
 *
 * Parameters:
 *
 *   pMem --
 *     Pointer to the memory from which allocations should be made.
 *
 *   memSize --
 *     The number of bytes of memory available for allocation, beginning at
 *     the location pointed to by pMem.
 *
 *     If memSize is set to zero, then it is assumed that this memory block
 *     has been saved, previously, and is now being given to us again for
 *     allocation.  In this case, the memory is assumed to be in exactly the
 *     same state as it was the previous time (although the pointer pMem may
 *     point to a different location than the first time).  Memory
 *     transactions that were made in the previous instance are available
 *     again in this instance.
 *
 * Returns:
 *
 */
ReturnCode
SBM_open(unsigned char * pMem,
	 OS_Uint32 memSize)
{
    OS_Uint32		origMemSize = memSize;

    if (! bInitialized)
    {
	return SBM_RC_NotInitialized;
    }

    TM_TRACE((sbm_globals.hTM, TM_ENTER,
	      "SBM_open(pMem=0x%lx, memSize=0x%lx)",
	      pMem, memSize));

    memSize = memSize - (memSize % sizeof(OS_Uint32));

    if (memSize != origMemSize)
    {
	TM_TRACE((sbm_globals.hTM, TM_ENTER,
		  "\tmemSize modified to 0x%lx", memSize));
    }

    /* Point to the start of memory */
    sbm_globals.pStart = pMem;

    /* Point to the header */
    sbm_globals.pHeader = (Header *) pMem;

    /* If it's not an existing memory block, specify memory settings */
    if (memSize != 0)
    {
	/*
	 * Determine the number of bytes remaining for transactions.
	 * The number of bytes available is the total bytes of
	 * non-volatile memory minus the size of the header.
	 */
	sbm_globals.pHeader->bytesRemaining = memSize - sizeof(Header);
    
	/* Save the memory size */
	sbm_globals.pHeader->memSize = memSize;

	/* Save the initial number of transaction offsets */
	sbm_globals.pHeader->transAllocated = 0;
    }

    /* Point to the beginning of the transaction offsets */
    sbm_globals.pTransOffsets = 
	(Transaction *) (sbm_globals.pStart +
			 sbm_globals.pHeader->memSize) -
	sbm_globals.pHeader->transAllocated;

    /* Point to our next available transaction location */
    sbm_globals.pNextAvailable =
	(unsigned char *) sbm_globals.pTransOffsets -
	sbm_globals.pHeader->bytesRemaining;

    /* There's no transaction in process */
    sbm_globals.bTransactionInProcess = FALSE;

    TM_TRACE((sbm_globals.hTM, TM_ENTER, "\tSuccess"));

    return Success;
}


/*
 * Function:	SBM_close()
 *
 * Description:	Close access to the Static Block Memory previously opened 
 *		via SBM_open().
 *
 * Parameters:
 *
 * Returns:
 *
 */
ReturnCode
SBM_close(void)
{
    if (! bInitialized)
    {
	return SBM_RC_NotInitialized;
    }

    sbm_globals.pHeader = NULL;
    sbm_globals.pStart = NULL;
    sbm_globals.pTransOffsets = NULL;
    sbm_globals.pNextAvailable = NULL;

    return Success;
}



/*
 * Function:	SBM_beginTrans()
 *
 * Description: 
 *
 * Begin a memory transaction.  Memory transactions are related groups of
 * allocations.  Entire transactions may be freed at one time.  A single
 * allocation is identified by its Transaction Handle, provided when a
 * transaction is begun -- via SBM_beginTrans(); and by its Memory Handle,
 * provided at the time of the allocation -- via SBM_alloc() or
 * SBM_allocStringCopy().
 *
 * Parameters:
 *
 *   transactionType --
 *     A single octet identifying the type of the transaction.  Transaction
 *     types are completely under the control of the caller.  Transactions may
 *     be "searched for" by transaction type, by calling SBM_nextTrans().
 *
 *   phTransInProcess --
 *     Pointer to a location in which a handle for this transaction will be
 *     placed.  This handle is valid for allocating memory during this
 *     transaction, but ceases to be valid upon ending the transaction by
 *     calling SBM_endTrans(); or upon aborting the transaction by calling
 *     SBM_abortTrans().  When a transaction is ended, a transaction handle is
 *     returned that may be used for accessing memory allocated during the
 *     transaction.
 *
 *   phTransaction --
 *     Pointer to a location in which a handle to the transaction is placed.
 *     This handle may be used, after the transaction is ended by calling
 *     SBM_endTrans(), for referencing memory allocated within this
 *     transaction, and for freeing the memory allocated to entire
 *     transaction.
 */
ReturnCode
SBM_beginTrans(OS_Uint8 transactionType,
	       SBM_TransInProcess * phTransInProcess,
	       SBM_Transaction * phTransaction)
{
    int				i;
    int				extraSpace;
    TransactionInProcess *	pTransInProcess;
    Transaction * 		pTrans;

    if (! bInitialized)
    {
	return SBM_RC_NotInitialized;
    }

    TM_TRACE((sbm_globals.hTM, TM_ENTER,
	      "SBM_beginTrans(transactionType=%u)",
	      transactionType));

    /* Make sure there's no other transaction in process */
    if (sbm_globals.bTransactionInProcess)
    {
	TM_TRACE((sbm_globals.hTM, TM_ENTER,
		  "\tFAIL: Transaction already in process"));
	
	return SBM_RC_TransactionInProcess;
    }

    /*
     * Make sure there's a transaction offset available for this new
     * transaction.  (For now, this is a linear search.  At some
     * point, we should optimize this some.)
     */
    for (i = 0,
	     pTrans = (sbm_globals.pTransOffsets +
		       sbm_globals.pHeader->transAllocated - 1);
	 i < sbm_globals.pHeader->transAllocated;
	 i++, pTrans--)
    {
	/* Is this one available? */
	if (pTrans->transLen == 0xffffffff)
	{
	    /* Yup. */
	    break;
	}
    }

    /* Did we find one? */
    if (i == sbm_globals.pHeader->transAllocated)
    {
	/* No.  Make room for more transactions */
	if (sbm_globals.pHeader->bytesRemaining < sizeof(Transaction))
	{
	    TM_TRACE((sbm_globals.hTM, TM_ENTER,
		      "\tFAIL: no more space for transactions"));

	    /* Not enough space available to add transaction headers */
	    return ResourceError;
	}

	extraSpace = sizeof(Transaction);
	pTrans = sbm_globals.pTransOffsets - 1;
    }
    else
    {
	extraSpace = 0;
    }

    /*
     * Make sure there's room for the transaction (if necessary) and
     * at least one byte of alloc'ed memory.
     */
    if (sbm_globals.pHeader->bytesRemaining < extraSpace + 1)
    {
	TM_TRACE((sbm_globals.hTM, TM_ENTER,
		  "\tFAIL: no more space for allocations"));

	return ResourceError;
    }

    /* Allocate a transaction structure */
    if ((pTransInProcess = OS_alloc(sizeof(TransactionInProcess))) == NULL)
    {
	TM_TRACE((sbm_globals.hTM, TM_ENTER,
		  "\tOS_alloc(%d) FAILED", sizeof(TransactionInProcess)));

	return ResourceError;
    }

    /* Save the transaction data starting point */
    pTransInProcess->pStart = sbm_globals.pNextAvailable;

    /* Save the next allocation point */
    pTransInProcess->pNextAlloc = pTransInProcess->pStart;

    /* Save the remaining amount of available memory */
    pTransInProcess->bytesRemaining =
	sbm_globals.pHeader->bytesRemaining - extraSpace;

    /* Save the transaction location */
    pTransInProcess->pTrans = pTrans;

    /* Assume we're not adding to a transaction */
    pTransInProcess->hAddingToTrans = SBM_NULL;

    /* Save the transaction type */
    pTrans->transactionType = transactionType;

    /* Save the current length of this transaction (zero) */
    pTrans->transLen = 0;

    /* Save the allocation starting point for this transaction */
    pTrans->offset = pTransInProcess->pStart - sbm_globals.pStart;

    /* There's now a transaction in process */
    sbm_globals.bTransactionInProcess = TRUE;

    /* Give 'em what they came for */
    *phTransInProcess = pTransInProcess;

    /*
     * The transaction handle is the index from the end of the
     * transaction area (the end of memory).  This makes the
     * transaction area relocatable, in case more memory is added
     * later.
     */
    *phTransaction =
	(Transaction *) (sbm_globals.pStart + sbm_globals.pHeader->memSize) -
	pTrans;

    TM_TRACE((sbm_globals.hTM, TM_ENTER,
	      "\tSuccess: TransInProcess = 0x%lx; Transaction = 0x%lx",
	      pTransInProcess, *phTransaction));

    return Success;
}


/*
 * Function:	SBM_alloc()
 *
 * Description:	Allocate a block of memory, to be part of a transaction 
 *		already in process.
 *
 * Parameters:
 *
 *   hTransInProcess --
 *     Handle to an in-process transaction, previously provided by
 *     SBM_transStart().
 *
 *   numBytes --
 *     The number of bytes requested for this allocation.
 *
 *   phMem --
 *     Pointer to a location in which a handle to this memory
 *     allocation will be placed.  The block of memory allocated by
 *     this function may be referenced, after the transaction has been
 *     ended by calling SBM_endTrans(), using the Transaction Handle
 *     provided by SBM_beginTrans() and this Memory Handle.
 *
 * Returns:
 *
 */
ReturnCode
SBM_alloc(SBM_TransInProcess hTransInProcess,
	  OS_Uint32 numBytes,
	  SBM_Memory * phMem)
{
    TransactionInProcess *	pTransInProcess = hTransInProcess;
    int				remainder;
#ifdef TM_ENABLED
    OS_Uint32			origNumBytes = numBytes;
#endif

    if (! bInitialized)
    {
	return SBM_RC_NotInitialized;
    }

    /* Align on a boundary acceptable to all modern machines */
    if ((remainder = numBytes % sizeof(OS_Uint32)) != 0)
    {
	numBytes += sizeof(OS_Uint32) - remainder;
    }

    /* Make sure requested number of bytes can be allocated on this machine */
    if (numBytes != (size_t) numBytes)
    {
	TM_TRACE((sbm_globals.hTM, TM_ENTER, 
		  "SBM_alloc() Requested nubmer of bytes "
		  "too big for architecture"));
	return ResourceError;
    }

    TM_TRACE((sbm_globals.hTM, TM_ENTER,
	      "SBM_alloc(hTransInProcess=0x%lx, requested=%lu, numBytes=%lu)",
	      pTransInProcess, origNumBytes, numBytes));

    /* Make sure there's a transaction in process */
    if (! sbm_globals.bTransactionInProcess)
    {
	TM_TRACE((sbm_globals.hTM, TM_ENTER,
		  "\tFAIL: no transaction in process"));
	
	return SBM_RC_NoTransactionInProcess;
    }

    /* Make sure there's room for this allocation */
    if (numBytes > pTransInProcess->bytesRemaining)
    {
	TM_TRACE((sbm_globals.hTM, TM_ENTER,
		  "\tFAIL: no more space for allocations"));

	return ResourceError;
    }

    /* Update the number of bytes remaining */
    pTransInProcess->bytesRemaining -= numBytes;

    /* Update the transaction length */
    pTransInProcess->pTrans->transLen += numBytes;

    /* Give 'em the offset to this mem from the beginning of the transaction */
    *phMem = pTransInProcess->pNextAlloc - pTransInProcess->pStart;

    /* Update the next allocation pointer */
    pTransInProcess->pNextAlloc += numBytes;

    TM_TRACE((sbm_globals.hTM, TM_ENTER, "Success: hMem=0x%d", *phMem));

    return Success;
}


/*
 * Function:	SBM_allocStringCopy()
 *
 * Description: Allocate a block of memory, to be part of a transaction 
 *		already in process. 
 *		The specified string is copied into the newly allocated memory.
 *
 * Parameters:
 *
 *   hTransInProcess --
 *     Handle to an in-process transaction, previously provided by
 *     SBM_transStart().
 *
 *   pString --
 *     A string which is to be copied into the allocated memory.  The number
 *     of bytes of allocated memory is the length of the string (including its
 *     null-terminator byte).
 *
 *   phMem --
 *     Pointer to a location in which a handle to this memory
 *     allocation will be placed.  The block of memory allocated by
 *     this function may be referenced, after the transaction has been
 *     ended by calling SBM_endTrans(), using the Transaction Handle
 *     provided by SBM_beginTrans() and this Memory Handle.
 *
 * Returns:
 *
 */
ReturnCode
SBM_allocStringCopy(SBM_TransInProcess hTransInProcess,
		    char * pString,
		    SBM_Memory * phMem)
{
    TransactionInProcess *	pTransInProcess = hTransInProcess;
    int				numBytes;
    int				remainder;

    if (! bInitialized)
    {
	return SBM_RC_NotInitialized;
    }

    TM_TRACE((sbm_globals.hTM, TM_ENTER,
	      "SBM_allocStringCopy(hTransInProcess=0x%lx, pString=(%s))",
	      pTransInProcess, pString == NULL ? "<null>" : pString));

    /* Make sure there's a transaction in process */
    if (! sbm_globals.bTransactionInProcess)
    {
	TM_TRACE((sbm_globals.hTM, TM_ENTER,
		  "\tFAIL: no transaction in process"));

	return SBM_RC_NoTransactionInProcess;
    }

    /* If we were passed a null pointer for the string, give 'em our NULL */
    if (pString == NULL)
    {
	/* Give 'em what they came for */
	*phMem = SBM_NULL;

	TM_TRACE((sbm_globals.hTM, TM_ENTER, "Success: hMem=NULL"));

	return Success;
    }

    numBytes = strlen(pString) + 1;
    
    /* Align on a boundary acceptable to all modern machines */
    if ((remainder = numBytes % sizeof(OS_Uint32)) != 0)
    {
	numBytes += sizeof(OS_Uint32) - remainder;
    }

    /* Make sure there's room for this allocation */
    if (numBytes > pTransInProcess->bytesRemaining)
    {
	TM_TRACE((sbm_globals.hTM, TM_ENTER,
		  "\tFAIL: no more space for allocations"));

	return ResourceError;
    }

    /* Update the number of bytes remaining */
    pTransInProcess->bytesRemaining -= numBytes;

    /* Update the transaction length */
    pTransInProcess->pTrans->transLen += numBytes;

    /* Copy the string data into the allocated area */
    strcpy((char *) pTransInProcess->pNextAlloc, pString);

    /* Give 'em the offset to this mem from the beginning of the transaction */
    *phMem = pTransInProcess->pNextAlloc - pTransInProcess->pStart;

    /* Update the next allocation pointer */
    pTransInProcess->pNextAlloc += numBytes;

    TM_TRACE((sbm_globals.hTM, TM_ENTER, "Success: hMem=0x%x", *phMem));

    return Success;
}


/*
 * Function:	SBM_AddToTrans()
 *
 * Add new allocations to an existing transaction.  Following the new
 * allocations, the transaction must be ended or aborted by calling
 * SBM_endTrans() or SBM_abortTrans().
 *
 * Parameters:
 *
 *   hOldTrans --
 *     Handle to the transaction to which allocations are to be added.
 *
 *   phTransInProcess --
 *     Pointer to a location in which a handle for this transaction will be
 *     placed.  This handle is valid for allocating memory during this
 *     transaction, but ceases to be valid upon ending the transaction by
 *     calling SBM_endTrans(); or upon aborting the transaction by calling
 *     SBM_abortTrans().  When a transaction is ended, a transaction handle is
 *     returned that may be used for accessing memory allocated during the
 *     transaction.
 *
 * Returns:
 *
 */
ReturnCode
SBM_addToTrans(SBM_Transaction hOldTrans,
	       SBM_TransInProcess * phTransInProcess)
{
    ReturnCode			rc;
    TransactionInProcess *	pTransInProcess;
    Transaction * 		pOldTrans;
    Transaction * 		pNewTrans;
    Transaction			tempTrans;
    SBM_Transaction		hNewTrans;
    SBM_Memory			hMem;

    if (! bInitialized)
    {
	return SBM_RC_NotInitialized;
    }

    TM_TRACE((sbm_globals.hTM, TM_ENTER,
	      "SBM_addToTrans(hTrans=%lu)",
	      hOldTrans));

    /* Make sure there's no other transaction in process */
    if (sbm_globals.bTransactionInProcess)
    {
	TM_TRACE((sbm_globals.hTM, TM_ENTER,
		  "\tFAIL: Transaction already in process"));
	
	return SBM_RC_TransactionInProcess;
    }

    /* Determine the transaction address */
    pOldTrans =
	(Transaction *) (sbm_globals.pStart + sbm_globals.pHeader->memSize) -
	hOldTrans;

    /* Create a new transaction */
    if ((rc = SBM_beginTrans((OS_Uint8) pOldTrans->transactionType,
			     (SBM_TransInProcess) &pTransInProcess,
			     &hNewTrans)) != Success)
    {
	TM_TRACE((sbm_globals.hTM, TM_ENTER,
		  "\tFAIL: SBM_beingTrans() return 0x%x", rc));
	return rc;
    }

    /* Allocate memory the size of the old transaction */
    if ((rc = SBM_alloc(pTransInProcess,
			pOldTrans->transLen, &hMem)) != Success)
    {
	TM_TRACE((sbm_globals.hTM, TM_ENTER,
		  "\tFAIL: Could not allocate old trans len %lu",
		  pOldTrans->transLen));

	(void) SBM_abortTrans(pTransInProcess);
	return rc;
    }

    /* Copy the old transaction to the newly allocated memory */
    OS_copy(SBM_getMemPointer(hNewTrans, SBM_FIRST),
	    SBM_getMemPointer(hOldTrans, SBM_FIRST),
	    (size_t) pOldTrans->transLen);

    /* Determine the new transaction address */
    pNewTrans =
	(Transaction *) (sbm_globals.pStart + sbm_globals.pHeader->memSize) -
	hNewTrans;

    /* Swap the transaction data */
    tempTrans = *pNewTrans;
    *pNewTrans = *pOldTrans;
    *pOldTrans = tempTrans;

    /* Point to the old and new transaction elements */
    pTransInProcess->pTrans = pOldTrans;
    pTransInProcess->hAddingToTrans = hNewTrans;
    
    /* Give 'em what they came for */
    *phTransInProcess = pTransInProcess;

    TM_TRACE((sbm_globals.hTM, TM_ENTER,
	      "\tSuccess: TransInProcess = 0x%lx", pTransInProcess));

    return Success;
}


/*
 * Function:	SBM_endTrans()
 *
 * Description:	End a memory transaction already in process.
 *
 * Parameters:
 *
 *   hTransInProcess --
 *     Handle to the in-process transaction, previously provided by
 *     SBM_beginTrans().
 *
 * Returns:
 *
 */
ReturnCode
SBM_endTrans(SBM_TransInProcess hTransInProcess)
{
    ReturnCode			rc;
    OS_Uint32			bytesRemaining;
    unsigned char *		pNextAvailable;
    TransactionInProcess *	pTransInProcess = hTransInProcess;

    if (! bInitialized)
    {
	return SBM_RC_NotInitialized;
    }

    TM_TRACE((sbm_globals.hTM, TM_ENTER,
	      "SBM_endTrans(hTransInProcess=0x%lx",
	      pTransInProcess));

    /* Make sure there's a transaction in process */
    if (! sbm_globals.bTransactionInProcess)
    {
	TM_TRACE((sbm_globals.hTM, TM_ENTER,
		  "\tFAIL: no transaction in process"));
	
	return SBM_RC_NoTransactionInProcess;
    }

    /* There's no longer a transaction in process */
    sbm_globals.bTransactionInProcess = FALSE;

    /* Update the number of bytes remaining */
    bytesRemaining = sbm_globals.pHeader->bytesRemaining;
    sbm_globals.pHeader->bytesRemaining = pTransInProcess->bytesRemaining;

    /* Update the next available memory pointer */
    pNextAvailable = sbm_globals.pNextAvailable;
    sbm_globals.pNextAvailable = pTransInProcess->pNextAlloc;

    /* If we added transaction space... */
    if (pTransInProcess->pTrans < sbm_globals.pTransOffsets)
    {
	/* ... then update them. */
	sbm_globals.pTransOffsets--;
	sbm_globals.pHeader->transAllocated++;
    }

    if (pTransInProcess->hAddingToTrans != SBM_NULL)
    {
	/* Free the old transaction */
	if ((rc = SBM_freeTrans(pTransInProcess->hAddingToTrans)) != Success)
	{
	    TM_TRACE((sbm_globals.hTM, TM_ENTER,
		      "\tFAIL: SMB_freeTrans() returned 0x%x", rc));

	    sbm_globals.bTransactionInProcess = TRUE;
	    sbm_globals.pHeader->bytesRemaining = bytesRemaining;
	    sbm_globals.pNextAvailable = pNextAvailable;
	    if (pTransInProcess->pTrans < sbm_globals.pTransOffsets)
	    {
		sbm_globals.pTransOffsets++;
		sbm_globals.pHeader->transAllocated--;
	    }
	    (void) SBM_abortTrans(hTransInProcess);
	    return rc;
	}
    }

    /* Free the transaction-in-process structure */
    OS_free(pTransInProcess);

    TM_TRACE((sbm_globals.hTM, TM_ENTER, "\tSuccess"));

    return Success;
}


/*
 * Function:	SBM_abortTrans()
 *
 * Description:	Abort a transaction which is in process.
 *
 * Parameters --
 *
 *   hTransInProcess --
 *     Handle to the in-process transaction, previously provided by
 *     SBM_beginTrans().
 *
 * Returns:
 *
 */
ReturnCode
SBM_abortTrans(SBM_TransInProcess hTransInProcess)
{
    Transaction *		pOrigTrans;
    TransactionInProcess *	pTransInProcess = hTransInProcess;

    if (! bInitialized)
    {
	return SBM_RC_NotInitialized;
    }

    TM_TRACE((sbm_globals.hTM, TM_ENTER,
	      "SBM_abortTrans(hTransInProcess=0x%lx)",
	      hTransInProcess));

    /* Make sure there's a transaction in process */
    if (! sbm_globals.bTransactionInProcess)
    {
	TM_TRACE((sbm_globals.hTM, TM_ENTER,
		  "\tFAIL: no transaction in process"));

	return SBM_RC_NoTransactionInProcess;
    }

    if (pTransInProcess->hAddingToTrans != SBM_NULL)
    {
	/* Determine the new transaction address */
	pOrigTrans =
	    (Transaction *) (sbm_globals.pStart +
			     sbm_globals.pHeader->memSize) -
	    pTransInProcess->hAddingToTrans;
	
	/* Restore the original transaction data */
	*pTransInProcess->pTrans = *pOrigTrans;
    }
    else
    {
	/* Reset the transaction length to show that it's available */
	pTransInProcess->pTrans->transLen = 0xffffffff;
    }
    
    /* There's no longer a transaction in process */
    sbm_globals.bTransactionInProcess = FALSE;

    /* Free the transaction-in-process structure */
    OS_free(pTransInProcess);

    TM_TRACE((sbm_globals.hTM, TM_ENTER, "\tSuccess"));

    return Success;
}


/*
 * Function:	SBM_freeTrans()
 *
 * Description:	Free all memory allocated in associated with the specified 
 *		transaction.
 *
 * Parameters --
 *
 *   hTransaction --
 *     Handle to the transaction for which all associated allocated
 *     memory is to be freed.  This is the handle which was provided
 *     by SBM_beginTrans().
 *
 * Returns:
 *
 */
ReturnCode
SBM_freeTrans(SBM_Transaction hTransaction)
{
    int 	    i;
    Transaction *   pTrans;
    OS_Uint32	    thisOffset;
    OS_Uint32	    moveBy;
    unsigned char * pSrc;
    unsigned char * pDest;

    if (! bInitialized)
    {
	return SBM_RC_NotInitialized;
    }

    TM_TRACE((sbm_globals.hTM, TM_ENTER,
	      "SBM_freeTrans(hTransaction=0x%lx)",
	      hTransaction));

    /* Make sure there's no transaction in process */
    if (sbm_globals.bTransactionInProcess)
    {
	TM_TRACE((sbm_globals.hTM, TM_ENTER,
		  "FAIL: transaction in process"));
	
	return SBM_RC_TransactionInProcess;
    }

    /* Determine the transaction address */
    pTrans =
	(Transaction *) (sbm_globals.pStart + sbm_globals.pHeader->memSize) -
	hTransaction;

    /* Move all memory beyond this transaction forward. */
    thisOffset = pTrans->offset;
    pDest = sbm_globals.pStart + thisOffset;
    pSrc = sbm_globals.pStart + thisOffset + pTrans->transLen;
    OS_move(pDest, pSrc, (OS_Uint16)((unsigned char *) sbm_globals.pTransOffsets - pSrc));

    /* Update the number of bytes remaining for allocation */
    sbm_globals.pHeader->bytesRemaining += pTrans->transLen;

    /* Update this transaction: it no longer exists. */
    pTrans->transLen = 0xffffffff;

    /* Determine the amount by which each offset is to be modified */
    moveBy = pSrc - pDest;

    /* Update our next available allocation pointer */
    sbm_globals.pNextAvailable -= moveBy;

    /* Update all offsets which just changed */
    for (i = 0,
	     pTrans = (sbm_globals.pTransOffsets +
		       sbm_globals.pHeader->transAllocated - 1);
	 i < sbm_globals.pHeader->transAllocated;
	 i++, pTrans--)
    {
	/* Does this one need to be changed? */
	if (pTrans->transLen != 0xffffffff &&
	    pTrans->offset > thisOffset)
	{
	    /* Yup. */
	    pTrans->offset -= moveBy;
	}
    }

    /* We can free up any unused transactions at the beginning of the list */
    for (i = 0, pTrans = sbm_globals.pTransOffsets;
	 i < sbm_globals.pHeader->transAllocated &&
	     pTrans->transLen == 0xffffffff;
	 i++, pTrans++)
    {
	sbm_globals.pTransOffsets++;
	sbm_globals.pHeader->transAllocated--;
	sbm_globals.pHeader->bytesRemaining += sizeof(Transaction);
    }

    TM_TRACE((sbm_globals.hTM, TM_ENTER, "\tSuccess"));

    return Success;
}


/*
 * Function:	SBM_getMemPointer()
 *
 * Description: 
 *
 * Get a temporary pointer to memory which has been allocated, as part of a
 * transaction, via SBM_beginTrans(), SBM_alloc() or SBM_allocStringCopy(),
 * and SBM_endTrans().
 *
 * Parameters --
 *
 *   hTransaction --
 *     Handle to the transaction in which the desired memory was
 *     allocated.  This is the handle which was provided by
 *     SBM_beginTrans().
 *
 *   hMem --
 *     Handle to the allocation in which the desired memory was
 *     allocated.  This is the handle which was provided by
 *     SMB_alloc() or SBM_allocStringCopy().  As a special case, if a
 *     pointer to the memory provided by the very first allocation is
 *     desired, this parameter be set to SBM_FIRST.
 *
 * Returns:
 *
 */
void *
SBM_getMemPointer(SBM_Transaction hTransaction,
		  SBM_Memory hMem)
{
    Transaction *   pTrans;

    TM_TRACE((sbm_globals.hTM, TM_ENTER, "SBM_getMemPointer(0x%lx, 0x%x)",
	      (unsigned long) hTransaction, (unsigned int) hMem));

    if (! bInitialized)
    {
	return NULL;
    }

    /* See if they're asking for the "first" allocation in this transaction. */
    if (hMem == SBM_FIRST)
    {
	hMem = 0;
    }

    /* Determine the transaction address */
    pTrans  = (Transaction *) (sbm_globals.pStart + sbm_globals.pHeader->memSize);
    pTrans -= hTransaction;

    TM_TRACE((sbm_globals.hTM, TM_ENTER,
	      "SBM_getMemPointer(0x%lx, 0x%x) returns 0x%lx",
	      (unsigned long) hTransaction,
	      (unsigned int) hMem,
	      (unsigned long) (sbm_globals.pStart +
			       pTrans->offset + (OS_Uint32) hMem)));

    /* Give 'em a pointer to their memory */
    return sbm_globals.pStart + pTrans->offset + (OS_Uint32) hMem;
}



/*
 * Function:	SBM_nextTrans()
 *
 * Description: 
 *
 * Get a handle to the first or next transaction which was created with the
 * specified transaction type.
 *
 * Parameters:
 *
 *   transactionType --
 *     Transaction type of the transaction which is desired.  If this
 *     value matches the transaction type of a transaction, as
 *     previously specified to SBM_transStart(), then the handle to
 *     that transaction will be provided.
 *
 *   hPreceedingTransaction --
 *     In order to iterate through each of the transactions, this
 *     handle may be set to SBM_FIRST to obtain the handle to the
 *     first transaction with a matching transaction type; or it may
 *     be set to the handle provided by a previous call to
 *     SBM_nextTrans(), to obtain a handle to the next
 *     transaction with a matching transaction type.
 *
 *   phTransaction --
 *     Pointer to a location in which the handle to a transaction with a
 *     matching transaction type will be placed.
 *
 * Returns:
 *
 */
ReturnCode
SBM_nextTrans(OS_Uint8 transactionType,
	      SBM_Transaction hPreceedingTransaction,
	      SBM_Transaction * phTransaction)
{
    int		    i;
    Transaction *   pTrans;
    OS_Uint16	    offset;

    if (! bInitialized)
    {
	return SBM_RC_NotInitialized;
    }

    TM_TRACE((sbm_globals.hTM, TM_ENTER,
	      "SBM_nextTrans(transType=%u, preceedingTrans=0x%lx)",
	      transactionType,
	      (unsigned long) hPreceedingTransaction));

    /* Determine where to begin searching */
    if (hPreceedingTransaction == SBM_FIRST)
    {
	offset = 1;
    }
    else
    {
	offset = (OS_Uint16) hPreceedingTransaction + 1;
    }

    /* Start looking for a transaction of the specified type */
    for (i = offset,
	     pTrans = (sbm_globals.pTransOffsets +
		       sbm_globals.pHeader->transAllocated - offset);
	 i <= sbm_globals.pHeader->transAllocated;
	 i++, pTrans--)
    {
	/* Is this an existing transaction, and is it of the correct type? */
	if (pTrans->transLen != 0xffffffff &&
	    pTrans->transactionType == transactionType)
	{
	    /*
	     * Yup.  Give 'em the transaction handle.  The transaction
	     * handle is the index from the end of the transaction
	     * area (the end of memory).
	     */
	    *phTransaction = i;

	    TM_TRACE((sbm_globals.hTM, TM_ENTER,
		      "Success: found transaction 0x%lx",
		      (unsigned long) *phTransaction));

	    return Success;
	}
    }

    TM_TRACE((sbm_globals.hTM, TM_ENTER,
	      "Fail: no matching transaction type found."));

    return Fail;
}


/*
 * Function:	SBM_prevTrans()
 *
 * Description: 
 *
 * Get a handle to the previous transaction which was created with the
 * specified transaction type.
 *
 * Parameters:
 *
 *   transactionType --
 *     Transaction type of the transaction which is desired.  If this
 *     value matches the transaction type of a transaction, as
 *     previously specified to SBM_transStart(), then the handle to
 *     that transaction will be provided.
 *
 *   hFollowingTransaction --
 *     The handle provided by a previous call to SBM_nextTrans() or
 *     SBM_prevTrans(), to obtain a handle to the previous transaction
 *     with a matching transaction type.
 *
 *   phTransaction --
 *     Pointer to a location in which the handle to a transaction with a
 *     matching transaction type will be placed.
 *
 * Returns:
 *
 */
ReturnCode
SBM_prevTrans(OS_Uint8 transactionType,
	      SBM_Transaction hFollowingTransaction,
	      SBM_Transaction * phTransaction)
{
    int		    i;
    Transaction *   pTrans;
    OS_Uint16	    offset;

    if (! bInitialized)
    {
	return SBM_RC_NotInitialized;
    }

    TM_TRACE((sbm_globals.hTM, TM_ENTER,
	      "SBM_prevTrans(transType=%u, followingTrans=0x%lx)",
	      transactionType,
	      (unsigned long) hFollowingTransaction));

    if (hFollowingTransaction == SBM_FIRST)
    {
	/* There's no previous transaction to the first one. */
	TM_TRACE((sbm_globals.hTM, TM_ENTER,
		  "Fail: no previous transaction to SBM_FIRST."));

	return Fail;
    }
    
    offset = (OS_Uint16) hFollowingTransaction - 1;

    /* Start looking for a transaction of the specified type */
    for (i = offset,
	     pTrans = (sbm_globals.pTransOffsets +
		       sbm_globals.pHeader->transAllocated - offset);
	 i > 0;
	 i--, pTrans++)
    {
	/* Is this an existing transaction, and is it of the correct type? */
	if (pTrans->transLen != 0xffffffff &&
	    pTrans->transactionType == transactionType)
	{
	    /*
	     * Yup.  Give 'em the transaction handle.  The transaction
	     * handle is the index from the end of the transaction
	     * area (the end of memory).
	     */
	    *phTransaction = i;

	    TM_TRACE((sbm_globals.hTM, TM_ENTER,
		      "Success: found transaction 0x%lx",
		      (unsigned long) *phTransaction));

	    return Success;
	}
    }

    TM_TRACE((sbm_globals.hTM, TM_ENTER,
	      "Fail: no matching transaction type found."));

    return Fail;
}


/*
 * Function:	SBM_quInit()
 *
 * Description:	Initialize a queue element, for later insertion onto a queue.
 *
 * Parameters:
 *
 *   hTrans --
 *     Handle to the transaction from which the queue element was allocated.
 *
 *   hMem --
 *     Allocation handle to the memory for this queue element.
 *
 * Returns:	None.
 *
 */
void
SBM_quInit(SBM_Transaction hTrans,
	   SBM_Memory hMem)
{
    SBM_QuElement *	pQElement;

    /* Get a pointer to the queue element structure */
    pQElement = SBM_getMemPointer(hTrans, hMem);

    /* Point the next and prev pointers at ourself */
    pQElement->next.hTrans = pQElement->prev.hTrans = hTrans;
    pQElement->next.hMem = pQElement->prev.hMem = hMem;
}


/*
 * Function:	SBM_quInsert()
 *
 * Description: 
 *
 * Insert, onto a queue, the element identified by hInsertMeTrans and
 * hInsertMeMem before the element identified by hBeforeMeTrans and
 * hBeforeMeMem.
 *
 * Parameters --
 *
 *   hInsertMeTrans
 *   hInsertMeMem --
 *     Transaction and Allocation handles for the queue element which is to
 *     be inserted.
 *
 *   hBeforeMeTrans
 *   hBeforeMeMem --
 *     Transaction and Allocation handles for the queue element which is
 *     already on the queue and which is to succeed the queue element being
 *     inserted.
 *
 * Returns:	None.
 *
 */
void
SBM_quInsert(SBM_Transaction hInsertMeTrans,
	     SBM_Memory hInsertMeMem,
	     SBM_Transaction hBeforeMeTrans,
	     SBM_Memory hBeforeMeMem)
{
    SBM_QuElement *	pInsertMe;
    SBM_QuElement *	pBeforeMe;
    SBM_QuElement *	pBeforePrev;
    SBM_QuElement *	pTempElement;
    SBM_Pointer		tempElement;

    pInsertMe = SBM_getMemPointer(hInsertMeTrans, hInsertMeMem);
    pBeforeMe = SBM_getMemPointer(hBeforeMeTrans, hBeforeMeMem);

    pBeforePrev = SBM_getMemPointer(pBeforeMe->prev.hTrans,
				    pBeforeMe->prev.hMem);
    pBeforePrev->next.hTrans = hInsertMeTrans;
    pBeforePrev->next.hMem = hInsertMeMem;
    
    tempElement.hTrans = pInsertMe->prev.hTrans;
    tempElement.hMem = pInsertMe->prev.hMem;
    pTempElement = SBM_getMemPointer(pInsertMe->prev.hTrans,
				     pInsertMe->prev.hMem);
    pTempElement->next.hTrans = hBeforeMeTrans;
    pTempElement->next.hMem = hBeforeMeMem;

    pInsertMe->prev.hTrans = pBeforeMe->prev.hTrans;
    pInsertMe->prev.hMem = pBeforeMe->prev.hMem;
    
    pBeforeMe->prev.hTrans = tempElement.hTrans;
    pBeforeMe->prev.hMem = tempElement.hMem;
}



/*
 * Function:	SBM_quRemove()
 *
 * Description: 
 *
 * Remove the element of a queue which is identified by the specified
 * transaction and allocation handles.
 *
 * Parameters --
 *
 *   hTrans
 *   hMem --
 *     Transaction and Allocation handles for the queue element which is to be
 *     removed from the queue.
 *
 * Returns:	None.
 *
 */
void
SBM_quRemove(SBM_Transaction hTrans,
	     SBM_Memory hMem)
{
    SBM_QuElement *	pRemoveMe;
    SBM_QuElement *	pRemoveNext;
    SBM_QuElement *	pRemovePrev;

    pRemoveMe = SBM_getMemPointer(hTrans, hMem);
    pRemoveNext = SBM_getMemPointer(pRemoveMe->next.hTrans,
				    pRemoveMe->next.hMem);
    pRemovePrev = SBM_getMemPointer(pRemoveMe->prev.hTrans,
				    pRemoveMe->prev.hMem);

    /* remove myself from the list */
    pRemoveNext->prev.hTrans = pRemoveMe->prev.hTrans;
    pRemoveNext->prev.hMem = pRemoveMe->prev.hMem;

    pRemovePrev->next.hTrans = pRemoveMe->next.hTrans;
    pRemovePrev->next.hMem = pRemoveMe->next.hMem;

    /* re-initialize myself */
    pRemoveMe->next.hTrans = pRemoveMe->prev.hTrans = hTrans;
    pRemoveMe->next.hMem = pRemoveMe->prev.hMem = hMem;
}


/*
 * Function:	SBM_quNext()
 *
 * Description:	Obtain the handles to a successor queue element.
 *
 * Parameters:
 *
 *   hThisTrans
 *   hThisMem --
 *     Transaction and Allocation handles for the queue element for which a
 *     successor is to be found.
 *
 *   phNextTrans
 *   phNextMem --
 *     Pointer to locations in which to place the Transaction and Allocation
 *     handles for the successor queue element.
 *
 * Returns:	None.
 *
 */
void
SBM_quNext(SBM_Transaction hThisTrans,
	   SBM_Memory hThisMem,
	   SBM_Transaction * phNextTrans,
	   SBM_Memory * phNextMem)
{
    SBM_QuElement *	pThis;

    /* Point to the This element */
    pThis = SBM_getMemPointer(hThisTrans, hThisMem);

    /* Give 'em the Next element */
    *phNextTrans = pThis->next.hTrans;
    *phNextMem = pThis->next.hMem;
}


/*
 * Function:	SBM_quNext()
 *
 * Description:	Obtain the handles to a predecessor queue element.
 *
 * Parameters:
 *
 *   hThisTrans
 *   hThisMem --
 *     Transaction and Allocation handles for the queue element for which a
 *     predecessor is to be found.
 *
 *   phPrevTrans
 *   phPrevMem --
 *     Pointer to locations in which to place the Transaction and Allocation
 *     handles for the predecessor queue element.
 *
 * Returns:	None.
 *
 */
void
SBM_quPrev(SBM_Transaction hThisTrans,
	   SBM_Memory hThisMem,
	   SBM_Transaction * phPrevTrans,
	   SBM_Memory * phPrevMem)
{
    SBM_QuElement *	pThis;

    /* Point to the This element */
    pThis = SBM_getMemPointer(hThisTrans, hThisMem);

    /* Give 'em the Previous element */
    *phPrevTrans = pThis->prev.hTrans;
    *phPrevMem = pThis->prev.hMem;
}


/*
 * Function:	SBM_dumpTrans()
 *
 * Description:	For debugging purposes, dump the transaction lists.
 *
 * Parameters:	None.
 *
 * Returns:	None.
 *
 */
void
SBM_dumpTrans(void)
{
#if ! defined(OS_TYPE_MSDOS) || ! defined(OS_VARIANT_Windows)
    int		    i;
    Transaction *   pTrans;

    printf("hTrans\tLength\t\tOffset\t\tType\n");
    printf("======\t======\t\t======\t\t====\n");
    for (i = 0,
	     pTrans = (sbm_globals.pTransOffsets +
		       sbm_globals.pHeader->transAllocated - 1);
	 i < sbm_globals.pHeader->transAllocated;
	 i++, pTrans--)
    {
	printf("%d\t0x%08lx\t%lu\t\t%lu\n",
	       i + 1,
	       pTrans->transLen, pTrans->offset, pTrans->transactionType);
    }
    printf("\n");
#endif
}
