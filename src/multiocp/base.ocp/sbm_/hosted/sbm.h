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

#ifndef __SBM_H__
#define	__SBM_H__

#include "estd.h"

typedef void *		SBM_TransInProcess;
typedef OS_Uint32	SBM_Transaction;
typedef OS_Uint32	SBM_Memory;


typedef struct SBM_Pointer
{
    SBM_Transaction 	hTrans;
    SBM_Memory 		hMem;
} SBM_Pointer;

typedef struct SBM_QuElement
{
    SBM_Pointer		next;
    SBM_Pointer		prev;
} SBM_QuElement;

typedef SBM_QuElement	SBM_QuHead;

/*
 * Use one of these as the first element within a structure which is
 * to be a queue head or queue element.
 */
#define	SBM_QUHEAD	SBM_Pointer next; SBM_Pointer prev
#define	SBM_QUELEMENT	SBM_Pointer next; SBM_Pointer prev


#define	SBM_NULL	(0xffffffff)

#define	SBM_FIRST	(0xffffffff)

enum SBM_ReturnCode
{
    SBM_RC_NotInitialized			= (1 | ModId_Sbm),
    SBM_RC_TransactionInProcess			= (2 | ModId_Sbm),
    SBM_RC_NoTransactionInProcess		= (3 | ModId_Sbm),
};


/*
 * SBM_init()
 *
 * Initialize the Static Block Memory module.
 */
ReturnCode
SBM_init(void);


/*
 * SBM_open()
 *
 * Prepare to use a static block of memory from which to allocate.
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
 */
ReturnCode
SBM_open(unsigned char * pMem,
	 OS_Uint32 memSize);


/*
 * SBM_close()
 *
 * Close access to the Static Block Memory previously opened via SBM_open().
 */
ReturnCode
SBM_close(void);


/*
 * SBM_beginTrans()
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
	       SBM_Transaction * phTransaction);


/*
 * SBM_alloc()
 *
 * Allocate a block of memory, to be part of a transaction already in process.
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
 */
ReturnCode
SBM_alloc(SBM_TransInProcess hTransInProcess,
	  OS_Uint32 numBytes,
	  SBM_Memory * phMem);


/*
 * SBM_allocStringCopy()
 *
 * Allocate a block of memory, to be part of a transaction already in process.
 * The specified string is copied into the newly allocated memory.
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
 */
ReturnCode
SBM_allocStringCopy(SBM_TransInProcess hTransInProcess,
		    char * pString,
		    SBM_Memory * phMem);

/*
 * SBM_AddToTrans()
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
 */
ReturnCode
SBM_addToTrans(SBM_Transaction hOldTrans,
	       SBM_TransInProcess * phTransInProcess);


/*
 * SBM_endTrans()
 *
 * End a memory transaction already in process.
 *
 * Parameters:
 *
 *   hTransInProcess --
 *     Handle to the in-process transaction, previously provided by
 *     SBM_beginTrans().
 *
 *   phTransaction --
 *     Pointer to a location in which a handle to the transaction is placed.
 *     This handle may be used for referencing memory allocated within this
 *     transaction, and for freeing the memory allocated to entire
 *     transaction.
 */
ReturnCode
SBM_endTrans(SBM_TransInProcess hTransInProcess);


/*
 * SBM_abortTrans()
 *
 * Abort a transaction which is in process.
 *
 * Parameters --
 *
 *   hTransInProcess --
 *     Handle to the in-process transaction, previously provided by
 *     SBM_beginTrans().
 */
ReturnCode
SBM_abortTrans(SBM_TransInProcess hTransInProcess);


/*
 * SBM_freeTrans()
 *
 * Free all memory allocated in associated with the specified transaction.
 *
 * Parameters --
 *
 *   hTransaction --
 *     Handle to the transaction for which all associated allocated
 *     memory is to be freed.  This is the handle which was provided
 *     by SBM_beginTrans().
 */
ReturnCode
SBM_freeTrans(SBM_Transaction hTransaction);


/*
 * SBM_getMemPointer()
 *
 * Get a temporary pointer to memory which has been allocated, as part of a
 * transaction, via SBM_beginTrans(), SBM_alloc() or SBM_allocStringCopy(),
 * and SBM_endTrans().
 *
 * Parameters --
 *
 *   hTransaction --
 *     Handle to the transaction in which the desired memory was allocated.
 *     This is the handle which was provided by SBM_beginTrans().
 *
 *   hMem --
 *     Handle to the allocation in which the desired memory was allocated.
 *     This is the handle which was provided by SMB_alloc() or
 *     SBM_allocStringCopy().
 */
void *
SBM_getMemPointer(SBM_Transaction hTransaction,
		  SBM_Memory hMem);


/*
 * SBM_nextTrans()
 *
 * Get a handle to the first or next transaction which was created with the
 * specified transaction type.
 *
 * Parameters:
 *
 *   transactionType --
 *     Transaction type of the transaction which is desired.  If this value
 *     matches the transaction type of a transaction, as previously specified
 *     to SBM_transStart(), then the handle to that transaction will be
 *     provided.
 *
 *   hPreceedingTransaction --
 *     In order to iterate through each of the transactions, this handle may
 *     be set to SBM_TRANS_START to obtain the handle to the first transaction
 *     with a matching transaction type; or it may be set to the handle
 *     provided by a previous call to SBM_nextTrans(), to obtain a handle
 *     to the next transaction with a matching transaction type.
 *
 *   phTransaction --
 *     Pointer to a location in which the handle to a transaction with a
 *     matching transaction type will be placed.
 */
ReturnCode
SBM_nextTrans(OS_Uint8 transactionType,
	      SBM_Transaction hPreceedingTransaction,
	      SBM_Transaction * phTransaction);


/*
 * SBM_prevTrans()
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
 */
ReturnCode
SBM_prevTrans(OS_Uint8 transactionType,
	      SBM_Transaction hFollowingTransaction,
	      SBM_Transaction * phTransaction);


/*
 * SBM_quInit()
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
SBM_quInit(SBM_Transaction hTrans,
	   SBM_Memory hMem);


/*
 * SBM_quInsert()
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
SBM_quInsert(SBM_Transaction hInsertMeTrans,
	     SBM_Memory hInsertMeMem,
	     SBM_Transaction hBeforeMeTrans,
	     SBM_Memory hBeforeMeMem);


/*
 * SBM_quRemove()
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
SBM_quRemove(SBM_Transaction hTrans,
	     SBM_Memory hMem);


/*
 * SBM_quNext()
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
SBM_quNext(SBM_Transaction hThisTrans,
	   SBM_Memory hThisMem,
	   SBM_Transaction * phNextTrans,
	   SBM_Memory * phNextMem);


/*
 * SBM_quNext()
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
SBM_quPrev(SBM_Transaction hThisTrans,
	   SBM_Memory hThisMem,
	   SBM_Transaction * phPrevTrans,
	   SBM_Memory * phPrevMem);


/*
 * SBM_dumpTrans()
 *
 * For debugging purposes, dump the transaction lists.
 */
void
SBM_dumpTrans(void);


#endif /* __SBM_H__ */
