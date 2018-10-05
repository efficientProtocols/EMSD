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
** Non-Volatile Memory functionality
**
**   This portation uses the file system to maintain non-volatility of
**   the memory.
**
**/


#include "estd.h"
#include "eh.h"
#include "tm.h"
#include "queue.h"
#include "sbm.h"
#include "nvm.h"


struct
{
    unsigned char *	pMem;
    size_t		memSize;
    FILE *		hFile;
    TM_ModuleCB *	hTM;
} nvm_globals;


static OS_Boolean	bInitialized;


/*
 * NVM_init()
 *
 * Initialize the Non-Volatile Memory module.
 */
ReturnCode
NVM_init(void)
{
    ReturnCode	    rc;

    if (bInitialized)
    {
	return Success;
    }

    /* Prepare for tracing */
    if (TM_OPEN(nvm_globals.hTM, "NVM") == NULL)
    {
	return Fail;
    }

    /* Initialize the Static Block Memory module */
    if ((rc =  SBM_init()) != Success)
    {
	return rc;
    }
    
    bInitialized = TRUE;

    return Success;
}



/*
 * NVM_open()
 *
 * For this disk-oriented portation, open the disk file to be used to hold the
 * non-volatile memory.
 *
 * Parameters:
 *
 *   pFileName -
 *     Name of the file in which the non-volatile memory is to be stored.
 *
 *   memSize --
 *     The number of bytes of memory available for allocation.
 *
 *     If memSize is set to zero, then it is assumed that this memory
 *     block has been saved, previously, and is now being given to us
 *     again for allocation.  In this case, the memory is assumed to
 *     be in exactly the same state as it was the previous time.
 *     Memory transactions that were made in the previous instance are
 *     available again in this instance.
 */
ReturnCode
NVM_open(char * pFileName,
	 OS_Uint32 memSize)
{
    ReturnCode		rc;
    char *		pMode;
    size_t		i;
    size_t		sysDepMemSize;

    if (! bInitialized)
    {
	return NVM_RC_NotInitialized;
    }

    /*
     * If we're to initialize the memory, open the file for writing.
     * Otherwise, open it for reading and writing.  Also, we'll
     * allocate dynamic memory to hold the non-volatile data
     * temporarily.  If we were given the length, we'll allocate that
     * much.  Otherwise, we'll see how long the existing file is, and
     * allocate that much.
     */
    if (memSize != 0)
    {
#if defined(OS_TYPE_UNIX)
	pMode = "w";
#else
	pMode = "wb";
#endif
    }
    else
    {
#if defined(OS_TYPE_UNIX)
	pMode = "r+";
#else
	pMode = "rb+";
#endif
	if ((rc = OS_fileSize(pFileName, &memSize)) != Success)
	{
	    TM_TRACE((nvm_globals.hTM, TM_ENTER,
		      "Could not determine size of specified file %s",
		      pFileName));

	    return rc;
	}
    }

    /* Make sure the requested memory size can be allocated on this machine */
    sysDepMemSize = (size_t) memSize;
    if (memSize != (OS_Uint32) sysDepMemSize)
    {
	return ResourceError;
    }

    /* Save the memory size */
    nvm_globals.memSize = sysDepMemSize;

    /* Allocate space to hold the file data */
    if ((nvm_globals.pMem = OS_alloc(sysDepMemSize)) == NULL)
    {
	TM_TRACE((nvm_globals.hTM, TM_ENTER,
		  "Allocation failed; requested size = 0x%lu", memSize));

	return ResourceError;
    }

    /* Open the file */
    if ((nvm_globals.hFile = fopen(pFileName, pMode)) == NULL)
    {
	TM_TRACE((nvm_globals.hTM, TM_ENTER,
		  "Could not open file %s with mode %s",
		  pFileName, pMode));

	OS_free(nvm_globals.pMem);
	return (*pMode == 'w' ? NVM_RC_CreateFailed : NVM_RC_OpenFailed);
    }

    /* No buffering on this file. */
    setbuf(nvm_globals.hFile, NULL);

    /* If we opened an existing file, read the file data */
    if (*pMode == 'r')
    {
	if ((i = fread(nvm_globals.pMem, 1, 
		  sysDepMemSize, nvm_globals.hFile)) != sysDepMemSize)
	{
	    TM_TRACE((nvm_globals.hTM, TM_ENTER,
		      "Read error, reading file %s; "
		      "expected %ld bytes, got %ld",
		      pFileName, (long) sysDepMemSize, (long) i));

	    return NVM_RC_ReadError;
	}
    }

    /* Pass this memory to the Static Block Memory module */
    if ((rc = SBM_open(nvm_globals.pMem,
		       *pMode == 'r' ? 0 : memSize)) != Success)
    {
	TM_TRACE((nvm_globals.hTM, TM_ENTER,
		  "SBM_open() failed, reason 0x%x", rc));

	return rc;
    }

    /* Dump the in-memory copy out to the file. */
    NVM_sync();

    return Success;
}



/*
 * NVM_close()
 *
 * For this disk-oriented portation, close the file associated with the
 * non-volatile memory.
 */
ReturnCode
NVM_close(void)
{
    ReturnCode		rc;

    if (! bInitialized)
    {
	return NVM_RC_NotInitialized;
    }

    /* Close the Static Block Memory that we've been using */
    if ((rc = SBM_close()) != Success)
    {
	TM_TRACE((nvm_globals.hTM, TM_ENTER,
		  "SBM_close() failed, reason 0x%x", rc));

	/* Fall through and continue cleaning up; don't return yet. */
    }

    /* Close the file */
    fclose(nvm_globals.hFile);

    /* Free the memory */
    OS_free(nvm_globals.pMem);

    /* Catch attempts at accessing it later */
    nvm_globals.hFile = NULL;
    nvm_globals.pMem = NULL;

    return Success;
}



/*
 * NVM_beginTrans()
 *
 * Begin a memory transaction.  Memory transactions are related groups of
 * allocations.  Entire transactions may be freed at one time.  A single
 * allocation is identified by its Transaction Handle, provided when a
 * transaction is begun -- via NVM_beginTrans(); and by its Memory Handle,
 * provided at the time of the allocation -- via NVM_alloc() or
 * NVM_allocStringCopy().
 *
 * Parameters:
 *
 *   transactionType --
 *     A single octet identifying the type of the transaction.  Transaction
 *     types are completely under the control of the caller.  Transactions may
 *     be "searched for" by transaction type, by calling NVM_nextTrans().
 *
 *   phTransInProcess --
 *     Pointer to a location in which a handle for this transaction will be
 *     placed.  This handle is valid for allocating memory during this
 *     transaction, but ceases to be valid upon ending the transaction by
 *     calling NVM_endTrans(); or upon aborting the transaction by calling
 *     NVM_abortTrans().  When a transaction is ended, a transaction handle is
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
NVM_beginTrans(OS_Uint8 transactionType,
	       NVM_TransInProcess * phTransInProcess,
	       NVM_Transaction * phTransaction)
{
    if (! bInitialized)
    {
	return NVM_RC_NotInitialized;
    }

    return SBM_beginTrans(transactionType,
			  (SBM_TransInProcess *) phTransInProcess,
			  (SBM_Transaction *) phTransaction);
}


/*
 * NVM_alloc()
 *
 * Allocate a block of memory, to be part of a transaction already in process.
 *
 * Parameters:
 *
 *   hTransInProcess --
 *     Handle to an in-process transaction, previously provided by
 *     NVM_transStart().
 *
 *   numBytes --
 *     The number of bytes requested for this allocation.
 *
 *   phMem --
 *     Pointer to a location in which a handle to this memory
 *     allocation will be placed.  The block of memory allocated by
 *     this function may be referenced, after the transaction has been
 *     ended by calling NVM_endTrans(), using the Transaction Handle
 *     provided by NVM_beginTrans() and this Memory Handle.
 */
ReturnCode
NVM_alloc(NVM_TransInProcess hTransInProcess,
	  OS_Uint32 numBytes,
	  NVM_Memory * phMem)
{
    if (! bInitialized)
    {
	return NVM_RC_NotInitialized;
    }

    return SBM_alloc((SBM_TransInProcess) hTransInProcess,
		     numBytes,
		     (SBM_Memory *) phMem);
}


/*
 * NVM_allocStringCopy()
 *
 * Allocate a block of memory, to be part of a transaction already in process.
 * The specified string is copied into the newly allocated memory.
 *
 * Parameters:
 *
 *   hTransInProcess --
 *     Handle to an in-process transaction, previously provided by
 *     NVM_transStart().
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
 *     ended by calling NVM_endTrans(), using the Transaction Handle
 *     provided by NVM_beginTrans() and this Memory Handle.
 */
ReturnCode
NVM_allocStringCopy(NVM_TransInProcess hTransInProcess,
		    char * pString,
		    NVM_Memory * phMem)
{
    if (! bInitialized)
    {
	return NVM_RC_NotInitialized;
    }

    return SBM_allocStringCopy((SBM_TransInProcess) hTransInProcess,
			       pString,
			       (SBM_Memory *) phMem);
}


/*
 * NVM_AddToTrans()
 *
 * Add new allocations to an existing transaction.  Following the new
 * allocations, the transaction must be ended or aborted by calling
 * NVM_endTrans() or NVM_abortTrans().
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
 *     calling NVM_endTrans(); or upon aborting the transaction by calling
 *     NVM_abortTrans().  When a transaction is ended, a transaction handle is
 *     returned that may be used for accessing memory allocated during the
 *     transaction.
 *
 */
ReturnCode
NVM_addToTrans(NVM_Transaction hOldTrans,
	       NVM_TransInProcess * phTransInProcess)
{
    if (! bInitialized)
    {
	return NVM_RC_NotInitialized;
    }

    return SBM_addToTrans((SBM_Transaction) hOldTrans,
			  (SBM_TransInProcess *) phTransInProcess);
}


/*
 * NVM_endTrans()
 *
 * End a memory transaction already in process.
 *
 * Parameters:
 *
 *   hTransInProcess --
 *     Handle to the in-process transaction, previously provided by
 *     NVM_beginTrans().
 */
ReturnCode
NVM_endTrans(NVM_TransInProcess hTransInProcess)
{
    ReturnCode		rc;

    if (! bInitialized)
    {
	return NVM_RC_NotInitialized;
    }

    /* Do Static Block Memory processing to end the transaction */
    if ((rc = SBM_endTrans((SBM_TransInProcess) hTransInProcess)) != Success)
    {
	TM_TRACE((nvm_globals.hTM, TM_ENTER,
		  "SBM_endTrans() failed, reason 0x%x", rc));

	return rc;
    }

    /* Dump the in-memory copy out to the file. */
    NVM_sync();

    return Success;
}


/*
 * NVM_abortTrans()
 *
 * Abort a transaction which is in process.
 *
 * Parameters --
 *
 *   hTransInProcess --
 *     Handle to the in-process transaction, previously provided by
 *     NVM_beginTrans().
 */
ReturnCode
NVM_abortTrans(NVM_TransInProcess hTransInProcess)
{
    if (! bInitialized)
    {
	return NVM_RC_NotInitialized;
    }

    return SBM_abortTrans((SBM_TransInProcess) hTransInProcess);
}


/*
 * NVM_freeTrans()
 *
 * Free all memory allocated in associated with the specified transaction.
 *
 * Parameters --
 *
 *   hTransaction --
 *     Handle to the transaction for which all associated allocated
 *     memory is to be freed.  This is the handle which was provided
 *     by NVM_beginTrans().
 */
ReturnCode
NVM_freeTrans(NVM_Transaction hTransaction)
{
    ReturnCode		rc;

    if (! bInitialized)
    {
	return NVM_RC_NotInitialized;
    }

    /* Do Static Block Memory processing to free the transaction */
    if ((rc = SBM_freeTrans((SBM_Transaction) hTransaction)) != Success)
    {
	TM_TRACE((nvm_globals.hTM, TM_ENTER,
		  "SBM_freeTrans() failed, reason 0x%x", rc));

	return rc;
    }

    /* Dump the in-memory copy out to the file. */
    NVM_sync();

    return Success;
}


/*
 * NVM_getMemPointer()
 *
 * Get a temporary pointer to memory which has been allocated, as part of a
 * transaction, via NVM_beginTrans(), NVM_alloc() or NVM_allocStringCopy(),
 * and NVM_endTrans().
 *
 * Parameters --
 *
 *   hTransaction --
 *     Handle to the transaction in which the desired memory was allocated.
 *     This is the handle which was provided by NVM_beginTrans().
 *
 *   hMem --
 *     Handle to the allocation in which the desired memory was allocated.
 *     This is the handle which was provided by SMB_alloc() or
 *     NVM_allocStringCopy().
 */
void *
NVM_getMemPointer(NVM_Transaction hTransaction,
		  NVM_Memory hMem)
{
    if (! bInitialized)
    {
	return NULL;
    }

    return SBM_getMemPointer((SBM_Transaction) hTransaction,
			     (SBM_Memory) hMem);
}


/*
 * NVM_nextTrans()
 *
 * Get a handle to the first or next transaction which was created with the
 * specified transaction type.
 *
 * Parameters:
 *
 *   transactionType --
 *     Transaction type of the transaction which is desired.  If this value
 *     matches the transaction type of a transaction, as previously specified
 *     to NVM_transStart(), then the handle to that transaction will be
 *     provided.
 *
 *   hPreceedingTransaction --
 *     In order to iterate through each of the transactions, this handle may
 *     be set to NVM_TRANS_START to obtain the handle to the first transaction
 *     with a matching transaction type; or it may be set to the handle
 *     provided by a previous call to NVM_nextTrans(), to obtain a handle
 *     to the next transaction with a matching transaction type.
 *
 *   phTransaction --
 *     Pointer to a location in which the handle to a transaction with a
 *     matching transaction type will be placed.
 */
ReturnCode
NVM_nextTrans(OS_Uint8 transactionType,
	      NVM_Transaction hPreceedingTransaction,
	      NVM_Transaction * phTransaction)
{
    if (! bInitialized)
    {
	return NVM_RC_NotInitialized;
    }

    return SBM_nextTrans(transactionType,
			 (SBM_Transaction) hPreceedingTransaction,
			 (SBM_Transaction *) phTransaction);
}


/*
 * NVM_prevTrans()
 *
 * Get a handle to the previous transaction which was created with the
 * specified transaction type.
 *
 * Parameters:
 *
 *   transactionType --
 *     Transaction type of the transaction which is desired.  If this value
 *     matches the transaction type of a transaction, as previously specified
 *     to NVM_transStart(), then the handle to that transaction will be
 *     provided.
 *
 *   hFollowingTransaction --
 *     The handle provided by a previous call to NVM_nextTrans() or
 *     NVM_prevTrans(), to obtain a handle to the previous transaction
 *     with a matching transaction type.
 *
 *   phTransaction --
 *     Pointer to a location in which the handle to a transaction with a
 *     matching transaction type will be placed.
 */
ReturnCode
NVM_prevTrans(OS_Uint8 transactionType,
	      NVM_Transaction hFollowingTransaction,
	      NVM_Transaction * phTransaction)
{
    if (! bInitialized)
    {
	return NVM_RC_NotInitialized;
    }

    return SBM_prevTrans(transactionType,
			 (SBM_Transaction) hFollowingTransaction,
			 (SBM_Transaction *) phTransaction);
}


/*
 * NVM_quInit()
 *
 * Initialize a queue element, for later insertion onto a queue.
 *
 * Parameters:
 *
 *   hTrans --
 *     Handle to the transaction from which the queue element was allocated.
 *
 *   hMem --
 *     Allocation handle to the memory for this queue element.
 */
void
NVM_quInit(NVM_Transaction hTrans,
	   NVM_Memory hMem)
{
    SBM_quInit((SBM_Transaction) hTrans, (SBM_Memory) hMem);
    
    /* Dump the in-memory copy out to the file. */
    NVM_sync();
}


/*
 * NVM_quInsert()
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
 */
void
NVM_quInsert(NVM_Transaction hInsertMeTrans,
	     NVM_Memory hInsertMeMem,
	     NVM_Transaction hBeforeMeTrans,
	     NVM_Memory hBeforeMeMem)
{
    SBM_quInsert((SBM_Transaction) hInsertMeTrans,
		 (SBM_Memory) hInsertMeMem,
		 (SBM_Transaction) hBeforeMeTrans,
		 (SBM_Memory) hBeforeMeMem);

    /* Dump the in-memory copy out to the file. */
    NVM_sync();
}



/*
 * NVM_quRemove()
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
 */
void
NVM_quRemove(NVM_Transaction hTrans,
	     NVM_Memory hMem)
{
    SBM_quRemove((SBM_Transaction) hTrans, (SBM_Memory) hMem);
    
    /* Dump the in-memory copy out to the file. */
    NVM_sync();
}


/*
 * NVM_quNext()
 *
 * Obtain the handles to a successor queue element.
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
 */
void
NVM_quNext(NVM_Transaction hThisTrans,
	   NVM_Memory hThisMem,
	   NVM_Transaction * phNextTrans,
	   NVM_Memory * phNextMem)
{
    SBM_quNext((SBM_Transaction) hThisTrans,
	       (SBM_Memory) hThisMem,
	       (SBM_Transaction *) phNextTrans,
	       (SBM_Memory *) phNextMem);
}


/*
 * NVM_quNext()
 *
 * Obtain the handles to a predecessor queue element.
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
 */
void
NVM_quPrev(NVM_Transaction hThisTrans,
	   NVM_Memory hThisMem,
	   NVM_Transaction * phPrevTrans,
	   NVM_Memory * phPrevMem)
{
    SBM_quPrev((SBM_Transaction) hThisTrans,
	       (SBM_Memory) hThisMem,
	       (SBM_Transaction *) phPrevTrans,
	       (SBM_Memory *) phPrevMem);
}


/*
 * NVM_sync()
 *
 * Syncronize the on-disk version of the non-volatile block with what's in
 * our in-memory copy.
 */
void
NVM_sync(void)
{
    rewind(nvm_globals.hFile);
    if (fwrite(nvm_globals.pMem, 1,
	       nvm_globals.memSize,
	       nvm_globals.hFile) != nvm_globals.memSize)
    {
	/* In this portation, we're going to call this a fatal error! */
	EH_fatal("Could not write non-volatile memory buffer to disk");
    }
}


/*
 * NVM_dumpTrans()
 *
 * For debugging purposes, dump the transaction lists.
 */
void
NVM_dumpTrans(void)
{
    NVM_dumpTrans();
}
